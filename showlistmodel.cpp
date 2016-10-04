#include "showlistmodel.h"

#include "globs.h"
#include <downloadmanager.h>

#include <QDebug>
#include <QBrush>
#include <QFileInfo>

ShowListModel::ShowListModel(QObject *parent, DataStore & store, bool /*archive*/)
    :QAbstractTableModel(parent), data_store(store)
{
    columns_names << tr("Title") << "#" << "UP" << tr("Personality") << "ID" << tr("Download");

    //list = &data_store.shows;

    connect(&data_store, SIGNAL(showsChanged()), SLOT(onShowsChanged()));
    //connect(&data_store, SIGNAL(favorited(int)), SLOT(onFavorited(int)));
    //connect(&data_store, SIGNAL(watched(int)), SLOT(onWatched(int)));
}

QVariant ShowListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            return columns_names[section];
        }
    }
    return QVariant();
}

int ShowListModel::rowCount(const QModelIndex & /*parent*/) const
{
    return list.size();
}

int ShowListModel::columnCount(const QModelIndex & /*parent*/) const
{
    return COLUMNS_SIZE;
}

QVariant ShowListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    Show cur_show = list.at(row);

    if (role == Qt::DisplayRole)
    {
        //qDebug() << index.row();
        switch(index.column()) {
            case COLUMN_TITLE:
                return cur_show.title;
            case COLUMN_EP:
                return cur_show.ep;
            case COLUMN_DATE:
                //return cur_show.update;
                return cur_show.update.toString("MM.dd");
            case COLUMN_PERSO:
                return cur_show.personality;
            case COLUMN_ID:
                return cur_show.id;
            case COLUMN_DOWNLOAD:
                if(percentages.contains(row))
                    return QVariant::fromValue(percentages[row]);
            default:
                return QVariant();
        }
    } else if(role == Qt::BackgroundRole) {
        if(!data_store.isWatched(cur_show))
            return QBrush(QColor(255, 255, 180));
    } else if(role == Qt::DecorationRole) {
        if(index.column() == COLUMN_TITLE && data_store.isFavorite(cur_show))
            return QPixmap(":/icons/fav.png");
    } else if(role == Qt::TextAlignmentRole) {
        if(index.column() == COLUMN_DATE || index.column() == COLUMN_EP)
            return Qt::AlignCenter;
    } else if(role == Qt::UserRole) {
        switch(index.column()) {
            case COLUMN_TITLE:
                return cur_show.title;
            case COLUMN_EP:
                return cur_show.ep;
            case COLUMN_DATE:
                return cur_show.update;
            case COLUMN_PERSO:
                return cur_show.personality;
            case COLUMN_ID:
                return cur_show.id;
            case COLUMN_DOWNLOAD:
                if(percentages.contains(row))
                    return percentages[row].first;
        }

        if(index.column() == COLUMN_DATE)
            return cur_show.update;
        else if(index.column() == COLUMN_TITLE)
            return cur_show.title;
    }
    return QVariant();
}

Show & ShowListModel::show(int row)
{
    return list[row];
}

int ShowListModel::showRow(const Show & cur_show) {
    int i;
    for(i = 0; i < list.size(); ++i) {
        const Show & show = list.at(i);
        if(cur_show.id == show.id && cur_show.ep == show.ep)
            return i;
    }
    return -1;
}

ProgressPair & ShowListModel::progress(int row)
{
    return percentages[row];
}

void ShowListModel::setProgress(int row, int percentage, const QString &text)
{
    if(!percentages.contains(row)) {
        ProgressPair pair = qMakePair(percentage, text);
        percentages.insert(row, pair);
    } else {
        percentages[row].first = percentage;
        percentages[row].second = text;
    }

    emit dataChanged(index(row, COLUMN_DOWNLOAD), index(row, COLUMN_DOWNLOAD));
}

bool ShowListModel::isWatched(int row)
{
    return data_store.isWatched(show(row));
}

bool ShowListModel::isFavorite(int row)
{
    return data_store.isFavorite(show(row));
}

void ShowListModel::setFavorite(int row, bool fav)
{
    data_store.setFavorite(show(row), fav);
    emit dataChanged(index(row, COLUMN_TITLE), index(row, COLUMN_TITLE));
}

void ShowListModel::setWatched(int row, QDate date)
{
    data_store.setWatched(show(row), date);
    // Data changed?
    //emit dataChanged(index(row, COLUMN_TITLE), index(row, COLUMN_PERSO));
}

/*void ShowListModel::onWatched(int row)
{
    //emit dataChanged(index(row, COLUMN_TITLE), index(row, COLUMN_PERSO));
}*/

void ShowListModel::onShowsChanged()
{
    beginResetModel();

    /* Load new shows */
    list.clear();
    QHash<QString, Show>::const_iterator it;
    for(it = data_store.shows.constBegin(); it != data_store.shows.constEnd(); ++it) {
        list.append(it.value());
    }

    /* Check if we can download shows */
    int i;
    for(i = 0; i < list.size(); i++) {
        resetProgress(i);
    }

    endResetModel();
}

void ShowListModel::resetProgress(int row)
{
    const Show & cur_show = show(row);

    if(cur_show.file.isEmpty()) {
        setProgress(row, DOWNLOAD_NA, "N/A");
    } else {
        QString filename = DownloadManager::makeShowFilename(cur_show);

        if(QFileInfo(filename).exists())
            setProgress(row, DOWNLOAD_FINISHED, tr("Play"));
        else
            setProgress(row, DOWNLOAD_READY, tr("Download"));
    }
}
