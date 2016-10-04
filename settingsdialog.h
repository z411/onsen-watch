#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLocale>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    enum PlayerSetting {
        PLAYER_INTERNAL,
        PLAYER_DEFAULT,
        PLAYER_EXTERNAL
    };

    enum NotificationSetting {
        NOTIFICATION_OFF,
        NOTIFICATION_FAVS,
        NOTIFICATION_ALL
    };

    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

private slots:

    void on_sliRefreshTime_valueChanged(int value);

    void on_cmbPlayer_currentIndexChanged(int index);

    void on_buttonBox_accepted();

    void on_btnPlayerLocation_clicked();

    void on_btnSound_clicked();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
