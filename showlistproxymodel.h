#ifndef SHOWLISTPROXYMODEL_H
#define SHOWLISTPROXYMODEL_H

#include <QSortFilterProxyModel>

class ShowListProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ShowListProxyModel(QObject *parent);

    bool filterFav() { return filter_fav; }
    void setFilterFav(bool filter);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    bool filter_fav;
};

#endif // SHOWLISTPROXYMODEL_H
