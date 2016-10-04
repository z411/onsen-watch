#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "datastore.h"
#include "downloadmanager.h"
#include "showlistmodel.h"
#include "showlistproxymodel.h"
#include "show.h"

#include <QMainWindow>
#include <QMediaPlayer>
#include <QSoundEffect>
#include <QHash>
#include <QTimer>
#include <QItemSelection>
#include <QLabel>
#include <QProgressBar>
#include <QSystemTrayIcon>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void status(QString msg);

private slots:
    void onInit();
    void onInitted();
    void onShowHide(bool checked);
    void onTray(QSystemTrayIcon::ActivationReason);
    void onShowProgress(int row, int progress);
    void onShowQueued(int row);
    void onShowUnqueued(int row);
    void onShowStarted(int row);
    void onShowFinished(int row);
    void onShowError(int row);
    void onThumbFinished(QByteArray data, QUrl url);
    void onDurationChanged(qint64 pos);
    void onPositionChanged(qint64 pos);
    void onAntinoise();
    void onProgressStart();
    void onProgress(qint64, qint64);
    void onProgressFinish(const QVector<Show> & new_shows, const QVector<Show> & new_fav_shows);
    void onMediaState(QMediaPlayer::State state);
    void onMediaAvailable(bool available);
    void onMediaSeekable(bool seekable);

    void on_btnUpdate_clicked();
    void on_chkFavsOnly_toggled(bool checked);
    void on_selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void on_chkFav_toggled(bool checked);
    void on_btnDownload_clicked();
    void on_lstShowView_clicked(const QModelIndex &index);
    void on_txtRegex_textChanged(const QString &arg1);
    void on_chkViewed_toggled(bool checked);
    void on_lstShowView_doubleClicked(const QModelIndex &);
    void on_actionAbout_Qt_triggered();
    void on_actionSettings_triggered();
    void on_sliPosition_sliderMoved(int position);
    void on_sliVolume_valueChanged(int value);
    void on_sliPosition_valueChanged(int value);
    void on_btnStop_clicked();
    void on_btnMute_toggled(bool checked);
    void on_btnPlay_clicked();
    void on_btnPause_clicked();
    void on_actionOpenDownloads_triggered();
    void on_btnFocusPlayingShow_clicked();
    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;

    DataStore data_store;
    DownloadManager download_manager;
    ShowListModel *model;
    ShowListProxyModel *proxyModel;

    QSettings settings;
    QMediaPlayer player;
    QSoundEffect alert_sound;

    QTimer antinoise;
    QTimer refresh_timer;

    QLabel * status_text;
    QProgressBar * status_progress;
    QSystemTrayIcon * tray_icon;
    QMenu * tray_menu;

    void updateList();
    void updateRow(int row, Show & cur_show);
    void enableElements(bool enable);
    void setInfo();
    void setPlayer(const Show & cur_show, bool play = false);
    void setupRefreshTimer();
    int selectedRow();
    Show & selectedShow();


    Show & selected_show;
    int selected_row;
    Show invalid_show;
    int playing_show_row;
};

#endif // MAINWINDOW_H
