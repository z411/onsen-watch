#ifndef SHOWLISTMODEL_H
#define SHOWLISTMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

#include <datastore.h>

typedef QPair<int, QString> ProgressPair;
Q_DECLARE_METATYPE(ProgressPair)

class ShowListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ShowListModel(QObject *parent, DataStore & store, bool archive);

    enum columns
    {
        COLUMN_TITLE,
        COLUMN_EP,
        COLUMN_DATE,
        COLUMN_PERSO,
        COLUMN_ID,
        COLUMN_DOWNLOAD,
        COLUMNS_SIZE
    };

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool isFavorite(int row);
    bool isWatched(int row);

    Show & show(int row);
    int showRow(const Show & cur_show);
    ProgressPair & progress(int row);
    void setProgress(int row, int percentage, const QString &text);
    void resetProgress(int row);
    void setFavorite(int row, bool checked);
    void setWatched(int row, QDate date);

    DataStore & dataStore();

private:
    bool is_archive;

    QStringList columns_names;
    QHash<int, ProgressPair> percentages;

    DataStore & data_store;
    QVector<Show> list;

private slots:
    //void onWatched(int row);
    void onShowsChanged();

};

#endif // SHOWLISTMODEL_H
