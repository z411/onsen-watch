#include "mainwindow.h"
#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSettings::setDefaultFormat(QSettings::IniFormat);
    QApplication::setApplicationName("onsen-watch");

    QSettings settings;
    QString locale = settings.value("locale", QLocale::system().name()).toString();
    if(locale.isEmpty())
        locale = QLocale::system().name();

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator onsenTranslator;
    onsenTranslator.load("onsen_" + locale);
    a.installTranslator(&onsenTranslator);

    a.setWindowIcon(QIcon(":/icons/onsen.png"));
    a.setApplicationVersion(APP_VERSION);

    MainWindow w;
    w.show();

    return a.exec();
}
