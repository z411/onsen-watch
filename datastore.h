#ifndef DATASTORE_H
#define DATASTORE_H

#include "apionsen.h"
#include "downloadmanager.h"

#include <QHash>
#include <QList>
#include <QDate>
#include <QFile>
#include <QSet>

class DataStore : public QObject
{
    Q_OBJECT

public:
    QHash<QString, Show> shows;

    DataStore();
    int init(bool force = false);
    void setFavorite(const Show & cur_show, bool fav);
    void setWatched(const Show & cur_show, QDate date);
    bool isFavorite(const Show & cur_show);
    bool isWatched(const Show & cur_show);

    //void addToArchive(const Show &);

    int load();
    void save();
    int loadUser();
    void saveUser();

private:
    ApiOnsen api_onsen;
    DownloadManager download_manager;

    QFile shows_file;
    QFile user_file;

    QHash<QString, QDate> viewDates;
    QSet<QString> favorites;

    QDate last_sync;

    QDataStream::Version data_version;

private slots:
    void onGotProgress(qint64, qint64);
    void onGotData(QByteArray data);

signals:
    void showsChanged();
    void downloadStarted();
    void downloadProgress(qint64, qint64);
    void downloadFinished(const QVector<Show> & new_shows, const QVector<Show> & new_fav_shows);
    void statusChanged(QString);
    void favorited(int);
    void watched(int);
};

#endif // DATASTORE_H
