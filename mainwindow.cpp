#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGlobal>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QDesktopServices>
#include <QProcess>
#include <QUrl>
#include <QTime>
#include <QFileInfo>
#include <QMessageBox>

#include "globs.h"
#include "settingsdialog.h"
#include "dateitem.h"
#include "showviewdelegate.h"
#include "jumpstyle.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    selected_show(invalid_show),
    selected_row(-1),
    playing_show_row(-1)
{
    ui->setupUi(this);

    /* Build status bar */
    status_text = new QLabel();
    status_progress = new QProgressBar();
    status_progress->setVisible(false);
    status_progress->setTextVisible(false);
    status_progress->setMaximumHeight(status_text->fontMetrics().height());
    ui->statusBar->addWidget(status_text, 1);
    ui->statusBar->addWidget(status_progress);

    /* Set up data store */
    connect(&data_store, SIGNAL(statusChanged(QString)), SLOT(status(QString)));
    connect(&data_store, SIGNAL(showsChanged()), SLOT(onInitted()));
    connect(&data_store, SIGNAL(downloadStarted()), SLOT(onProgressStart()));
    connect(&data_store, SIGNAL(downloadProgress(qint64,qint64)), SLOT(onProgress(qint64,qint64)));
    connect(&data_store, SIGNAL(downloadFinished(QVector<Show>,QVector<Show>)), SLOT(onProgressFinish(QVector<Show>,QVector<Show>)));

    /* Build models */
    model = new ShowListModel(this, data_store, false);
    proxyModel = new ShowListProxyModel(this);

    /* Build views */
    proxyModel->setSourceModel(model);
    proxyModel->setSortRole(Qt::UserRole);
    ui->lstShowView->setModel(proxyModel);
    ui->lstShowView->setItemDelegate(new ShowViewDelegate());
    #if QT_VERSION >= 0x050000
    ui->lstShowView->horizontalHeader()->setSectionResizeMode(ShowListModel::COLUMN_TITLE, QHeaderView::Stretch);
    ui->lstShowView->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
    #else
    ui->lstShowView->horizontalHeader()->setResizeMode(ShowListModel::COLUMN_TITLE, QHeaderView::Stretch);
    ui->lstShowView->verticalHeader()->resizeMode(QHeaderView::Fixed);
    #endif
    ui->lstShowView->horizontalHeader()->setStretchLastSection(false);
    ui->lstShowView->verticalHeader()->setDefaultSectionSize(ui->lstShowView->fontMetrics().height() + 5);
    ui->lstShowView->setColumnWidth(ShowListModel::COLUMN_EP, 30);
    ui->lstShowView->setColumnWidth(ShowListModel::COLUMN_DATE, 50);
    ui->lstShowView->sortByColumn(ShowListModel::COLUMN_DATE, Qt::DescendingOrder);
    ui->lstShowView->setSortingEnabled(true);

    if(settings.value("filterFav", false).toBool())
        ui->chkFavsOnly->toggle();

    connect(ui->lstShowView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(on_selectionChanged(QItemSelection,QItemSelection)));

    /* Setting up download manager */
    connect(&download_manager, SIGNAL(showProgress(int,int)), SLOT(onShowProgress(int, int)));
    connect(&download_manager, SIGNAL(showQueued(int)), SLOT(onShowQueued(int)));
    connect(&download_manager, SIGNAL(showUnqueued(int)), SLOT(onShowUnqueued(int)));
    connect(&download_manager, SIGNAL(showCanceled(int)), SLOT(onShowUnqueued(int)));
    connect(&download_manager, SIGNAL(showStarted(int)), SLOT(onShowStarted(int)));
    connect(&download_manager, SIGNAL(showFinished(int)), SLOT(onShowFinished(int)));
    connect(&download_manager, SIGNAL(showError(int)), SLOT(onShowError(int)));
    connect(&download_manager, SIGNAL(thumbFinished(QByteArray,QUrl)), SLOT(onThumbFinished(QByteArray,QUrl)));

    /* Create tray icon */
    tray_icon = new QSystemTrayIcon(this);
    tray_menu = new QMenu(this);

    QAction * action_show = new QAction(tr("Show/Hide"), this);
    connect(action_show, SIGNAL(triggered(bool)), SLOT(onShowHide(bool)));

    tray_menu->addAction(action_show);
    tray_menu->addSeparator();
    tray_menu->addAction(ui->actionExit);

    tray_icon->setContextMenu(tray_menu);
    tray_icon->setIcon(QIcon(":/icons/onsen.png"));

    connect(tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(onTray(QSystemTrayIcon::ActivationReason)));
    tray_icon->show();

    /* Set up timers */
    setupRefreshTimer();

#ifdef USE_QT_MULTIMEDIA
    /* Set up audio player */
    ui->sliPosition->setStyle(new JumpStyle(ui->sliPosition->style()));
    int volume = settings.value("playerVolume", 100).toInt();
    player.setVolume(volume);
    ui->sliVolume->setValue(volume);
    connect(&player, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(onMediaState(QMediaPlayer::State)));
    connect(&player, SIGNAL(audioAvailableChanged(bool)), SLOT(onMediaAvailable(bool)));
    connect(&player, SIGNAL(seekableChanged(bool)), SLOT(onMediaSeekable(bool)));
    connect(&player, SIGNAL(durationChanged(qint64)), SLOT(onDurationChanged(qint64)));
    connect(&player, SIGNAL(positionChanged(qint64)), SLOT(onPositionChanged(qint64)));
    connect(&antinoise, SIGNAL(timeout()), SLOT(onAntinoise()));

    antinoise.setInterval(200);
    antinoise.setSingleShot(true);
#endif

    /* Schedule the API init */
    QTimer::singleShot(0, this, SLOT(onInit()));
}

MainWindow::~MainWindow()
{
#ifdef USE_QT_MULTIMEDIA
    // Crashes on *nix (GStreamer) if we don't unload the player first
    player.setMedia(QMediaContent());

    // Save the player's volume
    settings.setValue("playerVolume", player.volume());
#endif

    delete ui;
}

void MainWindow::onShowHide(bool checked)
{
    Q_UNUSED(checked);

    if(isVisible()) {
        hide();
    } else {
        show();
        raise();
        setFocus();
    }
}

void MainWindow::onTray(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
        onShowHide(true);
}

void MainWindow::status(QString msg)
{
    qDebug() << msg;
    status_text->setText(msg);
}

void MainWindow::setupRefreshTimer()
{
    if(settings.value("refresh", true).toBool()) {
        int minutes = settings.value("refreshInterval", 60).toInt();

        refresh_timer.setInterval(minutes * 60000);
        //refresh_timer.setInterval(minutes * 1000);
        refresh_timer.setSingleShot(false);
        connect(&refresh_timer, SIGNAL(timeout()), SLOT(on_btnUpdate_clicked()));

        qDebug() << "Starting refresh timer. Interval: " << refresh_timer.interval();

        refresh_timer.start();

#ifdef USE_QT_MULTIMEDIA
        // Setup sound
        if(settings.value("sound", true).toBool()) {
            QString filename = settings.value("soundLocation", "audio/alert.wav").toString();
            alert_sound.setSource(QUrl::fromLocalFile(filename));
            alert_sound.setVolume(1);
        }
#endif
    } else {
        qDebug() << "Stopping refresh timer.";
        if(refresh_timer.isActive())
            refresh_timer.stop();
    }
}

void MainWindow::setInfo()
{
    if(selected_show.valid) {
        ui->txtTitle->setText(selected_show.title + " #" + QString::number(selected_show.ep).rightJustified(2, '0'));
        ui->txtPersonality->setText(selected_show.personality);
        ui->txtUpdate->setText(selected_show.update.toString("MM.dd UP"));
        ui->txtWeb->setText(QString("<a href=\"%1/\">%1</a>").arg(selected_show.url));

        ui->chkFav->setChecked(data_store.isFavorite(selected_show));
        if(!selected_show.file.isEmpty()) {
            ui->chkViewed->setChecked(data_store.isWatched(selected_show));
            ui->chkViewed->setEnabled(true);
        } else {
            ui->chkViewed->setChecked(false);
            ui->chkViewed->setEnabled(false);
        }
        ui->btnDownload->setEnabled(!selected_show.file.isEmpty());

        // Load thumbnail from cache; download it otherwise
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString filename = dataDir + "/cache/" + QUrl(selected_show.img_url).fileName();
        if(QFileInfo(filename).exists()) {
            QPixmap pix;
            pix.load(filename);
            ui->pixLogo->setPixmap(pix);
        } else {
            ui->pixLogo->setText(tr("Loading..."));
            download_manager.downloadThumb(selected_show.img_url);
        }

        // Set download button
        switch(model->progress(selected_row).first) {
            case DOWNLOAD_NA:
                ui->btnDownload->setEnabled(false);
                ui->btnDownload->setText(tr("Download episode"));
                break;
            case DOWNLOAD_READY:
            case DOWNLOAD_ERROR:
                ui->btnDownload->setEnabled(true);
                ui->btnDownload->setText(tr("Download episode"));
                break;
            case DOWNLOAD_FINISHED:
                ui->btnDownload->setEnabled(true);
                ui->btnDownload->setText(tr("Play episode"));
                break;
            case DOWNLOAD_QUEUED:
            case DOWNLOAD_STARTED:
            default:
                ui->btnDownload->setEnabled(true);
                ui->btnDownload->setText(tr("Cancel download"));
                break;
        }

        ui->chkFav->setEnabled(true);
    } else {
        // TODO invalid show
        ui->chkFav->setEnabled(false);
        ui->chkViewed->setEnabled(false);
        ui->btnDownload->setEnabled(false);

        ui->pixLogo->setPixmap(QPixmap(":/icons/onsen.jpg"));

    }

    qDebug() << selected_show.file;
}

int MainWindow::selectedRow()
{
    QItemSelectionModel *selmodel = ui->lstShowView->selectionModel();
    if(selmodel->hasSelection()) {
        QItemSelection selected = selmodel->selection();
        if(!selected.isEmpty()) {
            QItemSelection real_selected = proxyModel->mapSelectionToSource(selected);
            return real_selected.indexes().first().row();
        }
    }
    return -1;
}

Show & MainWindow::selectedShow()
{
    int row = selectedRow();
    if(row > -1)
        return model->show(row);
    else
        return invalid_show;
}

void MainWindow::enableElements(bool enable)
{
    qDebug() << "elements" << enable;
    ui->txtRegex->setEnabled(enable);
    ui->btnUpdate->setEnabled(enable);
    ui->lstShowView->setEnabled(enable);
    ui->chkFavsOnly->setEnabled(enable);

    if(enable && selected_show.valid) {
        ui->chkFav->setEnabled(true);
        ui->chkViewed->setEnabled(true);
        ui->btnDownload->setEnabled(true);
    } else {
        ui->chkFav->setEnabled(false);
        ui->chkViewed->setEnabled(false);
        ui->btnDownload->setEnabled(false);
    }
}

void MainWindow::onInit()
{
    /* Start API */
    status("Init API...");
    data_store.init(settings.value("refreshStart", false).toBool());
}

void MainWindow::onInitted()
{

}

void MainWindow::on_btnUpdate_clicked()
{
    // Reload data from the web
    data_store.init(true);
}

void MainWindow::on_chkFavsOnly_toggled(bool checked)
{
    proxyModel->setFilterFav(checked);

    settings.setValue("filterFav", checked);
}

void MainWindow::on_selectionChanged(const QItemSelection & /*selected*/, const QItemSelection & /*deselected*/)
{
    selected_row = selectedRow();
    selected_show = selectedShow();

    setInfo();
}

void MainWindow::on_chkFav_toggled(bool checked)
{
    if(selected_show.valid)
        model->setFavorite(selected_row, checked);
}

void MainWindow::on_btnDownload_clicked()
{
    if(selected_show.valid) {
        switch(model->progress(selected_row).first) {
            case DOWNLOAD_NA:
                return;
            case DOWNLOAD_READY:
            case DOWNLOAD_ERROR:
                download_manager.queueShowDownload(selected_row, selected_show);
                break;
            case DOWNLOAD_FINISHED:
                setPlayer(selected_show, true);
                break;
            case DOWNLOAD_QUEUED:
            case DOWNLOAD_STARTED:
            default:
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, tr("Cancel Download"), tr("Do you want to cancel the download?"),
                                            QMessageBox::Yes|QMessageBox::No);

                if(reply == QMessageBox::Yes)
                    download_manager.cancelShowDownload(selected_show);

                break;
        }


    }
}

void MainWindow::onShowQueued(int row)
{
    model->setProgress(row, DOWNLOAD_QUEUED, tr("Queued"));
    if(selected_row == row)
        setInfo();
}

void MainWindow::onShowUnqueued(int row)
{
    model->resetProgress(row);
    if(selected_row == row)
        setInfo();
}

void MainWindow::onShowStarted(int row)
{
    model->setProgress(row, DOWNLOAD_STARTED, tr("Starting..."));
    if(selected_row == row)
        setInfo();
}

void MainWindow::onShowFinished(int row)
{
    model->resetProgress(row);
    if(selected_row == row)
        setInfo();

    //Show cur_show = data_store.shows[row];
    // TODO implement archives
    // data_store.addToArchive(cur_show);
}

void MainWindow::onShowError(int row)
{
    model->setProgress(row, DOWNLOAD_ERROR, tr("Error"));
    if(selected_row == row)
        setInfo();
}

void MainWindow::onShowProgress(int row, int progress)
{
    model->setProgress(row, progress, QString::number(progress)+"%");
}

void MainWindow::onThumbFinished(QByteArray data, QUrl url) {
    // Load our freshly downloaded thumbnail from memory
    if(url.url() == selected_show.img_url) {
        QPixmap pix;
        pix.loadFromData(data);
        ui->pixLogo->setPixmap(pix);
    }

    if(playing_show_row != -1 && url.url() == model->show(playing_show_row).img_url) {
        QPixmap pix;
        pix.loadFromData(data);
        ui->pixPlayer->setPixmap(pix);
    }
}

void MainWindow::onProgressStart()
{
    // Since the server doesn't tell the size of the HTML file,
    // this is just an estimate.
    status_progress->setMaximum(100000);
    status_progress->setValue(0);
    status_progress->setVisible(true);
    enableElements(false);
}

void MainWindow::onProgress(qint64 value, qint64 /*total*/)
{
    status_progress->setValue(value);
}

void MainWindow::onProgressFinish(const QVector<Show> & new_shows, const QVector<Show> & new_fav_shows)
{
    qDebug() << "Progress finished";
    enableElements(true);
    status_progress->setVisible(false);

    /* Show desktop notification depending on the setting (and play sound) */
    SettingsDialog::NotificationSetting notify = (SettingsDialog::NotificationSetting)settings.value("notification", SettingsDialog::NOTIFICATION_FAVS).toInt();

    if(notify != SettingsDialog::NOTIFICATION_OFF) {
        if(
          (notify == SettingsDialog::NOTIFICATION_FAVS && !new_fav_shows.isEmpty()) ||
          (notify == SettingsDialog::NOTIFICATION_ALL && !new_shows.isEmpty())
          )
        {
            tray_icon->showMessage(tr("New Onsen episodes"), tr("There are %1 new episodes available, of which %2 are from your favorites.").arg(new_shows.size()).arg(new_fav_shows.size()));

#ifdef USE_QT_MULTIMEDIA
            if(settings.value("sound", true).toBool())
                alert_sound.play();
#endif
        }
    }

    /* Autodownload necessary shows */
    SettingsDialog::NotificationSetting autodownload = (SettingsDialog::NotificationSetting)settings.value("autodownload", SettingsDialog::NOTIFICATION_FAVS).toInt();

    if(autodownload != SettingsDialog::NOTIFICATION_OFF) {
        qDebug() << "Will autodownload...";
        QVector<Show> list = ((autodownload == SettingsDialog::NOTIFICATION_FAVS) ? new_fav_shows : new_shows);
        QVector<Show>::const_iterator i;

        if(!list.isEmpty()) {
            qDebug() << "List is not empty, let's go";
            for(i = list.constBegin(); i != list.constEnd(); ++i) {
                qDebug() << "Queueing" << i->id;
                download_manager.queueShowDownload(model->showRow(*i), (*i));
            }
        }
    }
}

void MainWindow::setPlayer(const Show &cur_show, bool play)
{
    QString filename = DownloadManager::makeShowFilename(cur_show);
    /* Attempt to play episode */
#ifdef USE_QT_MULTIMEDIA
    SettingsDialog::PlayerSetting player_setting = (SettingsDialog::PlayerSetting)settings.value("player", SettingsDialog::PLAYER_INTERNAL).toInt();
    if(player_setting == SettingsDialog::PLAYER_INTERNAL) {
        status(QString("Playing %1 #%2.").arg(cur_show.id, QString::number(cur_show.ep)));

        // Set thumbnail
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString thumb = dataDir + "/cache/" + QUrl(cur_show.img_url).fileName();
        if(QFileInfo(thumb).exists()) {
            QPixmap pix;
            pix.load(thumb);
            ui->pixPlayer->setPixmap(pix);
        }

        // Set basic information and media
        ui->lblPlayer->setText(cur_show.title + " #" + QString::number(cur_show.ep).rightJustified(2, '0'));
        ui->lblPlayerArtist->setText("  " + cur_show.personality);
        player.setMedia(QUrl::fromLocalFile(QFileInfo(filename).absoluteFilePath()));

        playing_show_row = selected_row;

        if(play)
            player.play();
    } else
#else
    SettingsDialog::PlayerSetting player_setting = (SettingsDialog::PlayerSetting)settings.value("player", SettingsDialog::PLAYER_EXTERNAL).toInt();
#endif
    if(player_setting == SettingsDialog::PLAYER_DEFAULT) {
        // Launch
        status(QString("Launching %1 #%2.").arg(cur_show.id, QString::number(cur_show.ep)));
        QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
    } else if(player_setting == SettingsDialog::PLAYER_EXTERNAL) {
        // Launch external
        status(QString("Launching %1 #%2.").arg(cur_show.id, QString::number(cur_show.ep)));

        QString process_filename = settings.value("playerLocation", "").toString();
        if(!process_filename.isEmpty()) {
            QStringList args;
            args << filename;

            QProcess::startDetached(process_filename, args);
        } else {
            QMessageBox::critical(this, tr("Error"), tr("No external player specified in settings."));
        }
    }

    if(!data_store.isWatched(cur_show))
        data_store.setWatched(cur_show, QDate::currentDate());
}

void MainWindow::on_lstShowView_clicked(const QModelIndex &index)
{
    if(index.column() == ShowListModel::COLUMN_DOWNLOAD)
    {
        int row = proxyModel->mapToSource(index).row();
        ProgressPair pair = model->progress(row);
        Show cur_show = model->show(row);

        switch(pair.first) {
            case DOWNLOAD_READY:
            case DOWNLOAD_ERROR:
                download_manager.queueShowDownload(row, cur_show);
                break;
            case DOWNLOAD_FINISHED:
                setPlayer(cur_show, true);
                break;
            case DOWNLOAD_QUEUED:
                download_manager.unqueueShowDownload(cur_show);
                break;
            case DOWNLOAD_NA:
                return;
            case DOWNLOAD_STARTED:
            default:
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, tr("Cancel Download"), tr("Do you want to cancel the download?"),
                                            QMessageBox::Yes|QMessageBox::No);

                if(reply == QMessageBox::Yes)
                    download_manager.cancelShowDownload(cur_show);
        }
    }
}

void MainWindow::on_txtRegex_textChanged(const QString &arg1)
{
    proxyModel->setFilterRegExp(arg1);
}

void MainWindow::on_chkViewed_toggled(bool checked)
{
    Show cur_show = selectedShow();
    if(!cur_show.valid) return;

    model->setWatched(selected_row, (checked ? QDate::currentDate() : QDate()));
}

void MainWindow::on_lstShowView_doubleClicked(const QModelIndex & index)
{
    if(!ui->chkFavsOnly->isChecked() && index.column() == ShowListModel::COLUMN_TITLE)
        ui->chkFav->toggle();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog dialog;
    if(dialog.exec() == QDialog::Accepted) {
        setupRefreshTimer();
    }
}

#ifdef USE_QT_MULTIMEDIA
void MainWindow::onDurationChanged(qint64 pos)
{
    ui->sliPosition->setMaximum(pos);
}

void MainWindow::onPositionChanged(qint64 pos)
{
    if(!ui->sliPosition->isSliderDown()) {
        ui->sliPosition->blockSignals(true);
        ui->sliPosition->setValue(pos);
        ui->sliPosition->blockSignals(false);
        ui->lblPlayerTime->setText(QTime::fromMSecsSinceStartOfDay(pos).toString("mm:ss") + " / " + QTime::fromMSecsSinceStartOfDay(player.duration()).toString("mm:ss"));
    }
}

void MainWindow::onAntinoise()
{
    if(!ui->btnMute->isChecked())
        player.setMuted(false);
}

void MainWindow::on_sliPosition_sliderMoved(int position)
{
    ui->lblPlayerTime->setText(QTime::fromMSecsSinceStartOfDay(position).toString("mm:ss") + " / " + QTime::fromMSecsSinceStartOfDay(player.duration()).toString("mm:ss"));
}

void MainWindow::on_sliVolume_valueChanged(int value)
{
    player.setVolume(value);
}

void MainWindow::on_sliPosition_valueChanged(int value)
{
    player.setMuted(true);
    player.setPosition(value);
    antinoise.start();
}

void MainWindow::on_btnStop_clicked()
{
    player.stop();
}

void MainWindow::on_btnMute_toggled(bool checked)
{
    player.setMuted(checked);
    ui->btnMute->setIcon(QIcon((checked ? ":/icons/muted.png" : ":/icons/unmuted.png")));
}

void MainWindow::on_btnPlay_clicked()
{
    player.play();
}

void MainWindow::on_btnPause_clicked()
{
    player.pause();
}

void MainWindow::onMediaAvailable(bool available)
{
    ui->btnFocusPlayingShow->setEnabled(available);
}

void MainWindow::onMediaSeekable(bool seekable)
{
    ui->sliPosition->setEnabled(seekable);
}

void MainWindow::onMediaState(QMediaPlayer::State state)
{
    switch(state)
    {
        case QMediaPlayer::StoppedState:
            ui->btnPlay->setEnabled(player.isAudioAvailable());
            ui->btnPause->setEnabled(false);
            ui->btnStop->setEnabled(false);
            ui->lblPlayerState->setText(tr("Stopped"));
            break;
        case QMediaPlayer::PlayingState:
            player.setMuted(true);
            antinoise.start();
            ui->btnPlay->setEnabled(false);
            ui->btnPause->setEnabled(true);
            ui->btnStop->setEnabled(true);
            ui->lblPlayerState->setText(tr("Playing"));
            break;
        case QMediaPlayer::PausedState:
            ui->btnPlay->setEnabled(true);
            ui->btnPause->setEnabled(false);
            ui->btnStop->setEnabled(true);
            ui->lblPlayerState->setText(tr("Paused"));
            break;
    }
}

void MainWindow::on_btnFocusPlayingShow_clicked()
{
    if(playing_show_row != -1) {
        QModelIndex index = proxyModel->mapFromSource(model->index(playing_show_row, 0));
        ui->lstShowView->selectRow(index.row());
        ui->lstShowView->setFocus();
    }
}
#endif

void MainWindow::on_actionOpenDownloads_triggered()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if(QDir(dataDir + "/download").exists())
        QDesktopServices::openUrl(QUrl(dataDir + "/download"));
}

void MainWindow::on_actionAbout_triggered()
{
    QString text =
            "<h3>Onsen Watch %1</h3><p>Author: z411 (z411@omaera.org)</p>"
            "<p><a href=\"https://github.com/z411/onsen-watch\">https://github.com/z411/onsen-watch</a></p>"
            "<p><small>onsen-watch (C) 2016 z411<br>This program comes with ABSOLUTELY NO WARRANTY; for details see LICENSE.</small></p>";
    QMessageBox::about(this, "Onsen Watch v0.1", text.arg(APP_VERSION));
}
