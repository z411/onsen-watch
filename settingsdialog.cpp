#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    /* Fill missing languages */
    ui->cmbLanguage->addItem("English", "en");
    ui->cmbLanguage->addItem("日本語", "ja");

    /* Fill players */
#ifdef USE_QT_MULTIMEDIA
    ui->cmbPlayer->addItem(tr("Internal"));
#endif
    ui->cmbPlayer->addItem(tr("External (default)"));
    ui->cmbPlayer->addItem(tr("External (custom location)"));

    /* Load settings into widgets */
    QSettings settings;

    ui->chkRefresh->setChecked(settings.value("refresh", true).toBool());
    ui->sliRefreshTime->setValue(settings.value("refreshInterval", 60).toInt());
    ui->chkRefreshStart->setChecked(settings.value("refreshStart", false).toBool());
    ui->cmbNotification->setCurrentIndex(settings.value("notification", NOTIFICATION_FAVS).toInt());
    ui->cmbAutodownload->setCurrentIndex(settings.value("autodownload", NOTIFICATION_FAVS).toInt());
    ui->chkSound->setChecked(settings.value("sound", true).toBool());
    ui->txtSoundLocation->setText(settings.value("soundLocation", "audio/alert.wav").toString());
#ifdef USE_QT_MULTIMEDIA
    ui->cmbPlayer->setCurrentIndex(settings.value("player", PLAYER_INTERNAL).toInt());
#else
    ui->cmbPlayer->setCurrentIndex(settings.value("player", PLAYER_EXTERNAL).toInt());
#endif
    ui->txtPlayerLocation->setText(settings.value("playerLocation", "").toString());
    ui->lblMinutes->setText(tr("%1 minutes").arg(ui->sliRefreshTime->value()));

    int i;
    for(i = 0; i < ui->cmbLanguage->count(); i++) {
        if(settings.value("locale") == ui->cmbLanguage->itemData(i).toString()) {
            ui->cmbLanguage->setCurrentIndex(i);
            break;
        }
    }
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_sliRefreshTime_valueChanged(int value)
{
    ui->lblMinutes->setText(tr("%1 minutes").arg(value));
}

void SettingsDialog::on_cmbPlayer_currentIndexChanged(int index)
{
    if(index == PLAYER_EXTERNAL) {
        ui->txtPlayerLocation->setEnabled(true);
        ui->btnPlayerLocation->setEnabled(true);
    } else {
        ui->txtPlayerLocation->setEnabled(false);
        ui->btnPlayerLocation->setEnabled(false);
    }
}

void SettingsDialog::on_buttonBox_accepted()
{
    QSettings settings;

    settings.setValue("refresh", ui->chkRefresh->isChecked());
    settings.setValue("refreshInterval", ui->sliRefreshTime->value());
    settings.setValue("refreshStart", ui->chkRefreshStart->isChecked());
    settings.setValue("notification", ui->cmbNotification->currentIndex());
    settings.setValue("autodownload", ui->cmbAutodownload->currentIndex());
    settings.setValue("sound", ui->chkSound->isChecked());
    settings.setValue("soundLocation", ui->txtSoundLocation->text());
    settings.setValue("player", ui->cmbPlayer->currentIndex());
    settings.setValue("playerLocation", ui->txtPlayerLocation->text());
    settings.setValue("locale", ui->cmbLanguage->currentData().toString());
}

void SettingsDialog::on_btnPlayerLocation_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose file..."), QFileInfo(ui->txtPlayerLocation->text()).absolutePath());
    if(!filename.isEmpty())
        ui->txtPlayerLocation->setText(filename);
}

void SettingsDialog::on_btnSound_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose file..."), QFileInfo(ui->txtSoundLocation->text()).absolutePath(), "Audio (*.wav)");
    if(!filename.isEmpty())
        ui->txtSoundLocation->setText(filename);
}
