#include "datastore.h"

#include <QFile>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>

#define MAGIC_NUMBER 0x6F6E7365
#define MAGIC_VERSION 10

DataStore::DataStore()
{
    connect(&download_manager, SIGNAL(gotData(QByteArray)), SLOT(onGotData(QByteArray)));
    connect(&download_manager, SIGNAL(gotProgress(qint64,qint64)), SLOT(onGotProgress(qint64,qint64)));

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    shows_file.setFileName(dataDir + "/shows.dat");
    user_file.setFileName(dataDir + "/user.dat");
}


int DataStore::init(bool force)
{
    data_version = QDataStream::Qt_5_6;

    loadUser();

    emit statusChanged(tr("Getting from cache..."));
    load();
    emit showsChanged();
    emit statusChanged(tr("Ready."));

    if(force || shows.empty()) {
        emit statusChanged(tr("Downloading from network..."));
        emit downloadStarted();
        download_manager.singleShot("http://www.onsen.ag/easy/easy.php");
    }

    return 1;
}

int DataStore::load()
{
    shows.clear();

    if(!shows_file.exists())
        return 1;

    if(shows_file.open(QIODevice::ReadOnly)) {
        QDataStream in(&shows_file);

        /*quint32 magic;
        in >> magic;
        if (magic != MAGIC_NUMBER)
            return 0; // invalid format
        qint32 version;
        in >> version;
        if (version != MAGIC_VERSION)
            return 0; // invalid version*/

        in.setVersion(data_version);

        in >> last_sync;
        in >> shows;

        shows_file.close();

        return 1;
    }

    return 0;
}

void DataStore::save()
{
    // Save show information
    if (shows_file.open(QIODevice::WriteOnly)) {
        QDataStream out(&shows_file);

        /*out << (quint32)MAGIC_NUMBER;
        out << (qint32)MAGIC_VERSION;*/

        out.setVersion(data_version);

        out << last_sync;
        out << shows;

        shows_file.flush();
        shows_file.close();
    } else
        qDebug() << "Warning: Couldn't write data store";
}

int DataStore::loadUser()
{
    favorites.clear();
    viewDates.clear();

    if(!user_file.exists())
        return 1;

    if (user_file.open(QIODevice::ReadOnly)) {
        QDataStream in(&user_file);

        /*quint32 magic;
        in >> magic;
        if (magic != MAGIC_NUMBER)
            return 0; // invalid format
        qint32 version;
        in >> version;
        if (version != MAGIC_VERSION)
            return 0; // invalid version*/

        in.setVersion(data_version);

        in >> favorites;
        in >> viewDates;

        user_file.flush();
        user_file.close();

        return 1;
    }
    return 0;
}

void DataStore::saveUser()
{
    if (user_file.open(QIODevice::WriteOnly))
    {
        QDataStream out(&user_file);

        /*out << (quint32)MAGIC_NUMBER;
        out << (qint32)MAGIC_VERSION;*/

        out.setVersion(data_version);

        out << favorites;
        out << viewDates;

        user_file.flush();
        user_file.close();
    }
}

/*void DataStore::addToArchive(const Show & cur_show) {
    //shows_archive.append(cur_show);
}*/

void DataStore::setFavorite(const Show & cur_show, bool fav)
{
    if(fav)
        favorites.insert(cur_show.id);
    else
        favorites.remove(cur_show.id);

    saveUser();
}

bool DataStore::isFavorite(const Show & cur_show)
{
    return favorites.contains(cur_show.id);
}

void DataStore::setWatched(const Show & cur_show, QDate date)
{
    QString key = cur_show.id + QString::number(cur_show.ep);

    if(!date.isNull())
        viewDates.insert(key, date);
    else
        viewDates.remove(key);

    saveUser();
}

bool DataStore::isWatched(const Show & cur_show)
{
    QString key = cur_show.id + QString::number(cur_show.ep);

    return cur_show.file.isEmpty() || (viewDates.contains(key) && viewDates[key] > cur_show.update);
}

void DataStore::onGotProgress(qint64 value, qint64 total)
{
    emit downloadProgress(value, total);
}

void DataStore::onGotData(QByteArray data)
{
    emit statusChanged(tr("Parsing..."));
    QVector<Show> latest_shows = api_onsen.read(data);

    /* Calculate new shows and report them */
    QVector<Show> new_shows;
    QVector<Show> new_fav_shows;

    QMutableVectorIterator<Show> i(latest_shows);
    while(i.hasNext()) {
        Show & cur_show = i.next();
        QString key = cur_show.id + QString::number(cur_show.ep);

        if(!shows.contains(key)) {
            shows.insert(key, cur_show);

            new_shows.append(cur_show);
            if(isFavorite(cur_show))
                new_fav_shows.append(cur_show);
        }
    }

    last_sync = api_onsen.latestDate();

    emit statusChanged(tr("Saving to cache..."));
    save();
    emit showsChanged();

    qDebug() << "New shows found:" << new_shows.size() << " Favorites:" << new_fav_shows.size();

    emit downloadFinished(new_shows, new_fav_shows);
    emit statusChanged(tr("Ready."));
}
