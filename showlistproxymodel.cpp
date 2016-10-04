#include "showlistproxymodel.h"

#include "showlistmodel.h"
#include <QDebug>

ShowListProxyModel::ShowListProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent), filter_fav(false)
{

}

bool ShowListProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{            
    QModelIndex index_id = sourceModel()->index(sourceRow, ShowListModel::COLUMN_ID, sourceParent);
    QModelIndex index_title = sourceModel()->index(sourceRow, ShowListModel::COLUMN_TITLE, sourceParent);
    QModelIndex index_perso = sourceModel()->index(sourceRow, ShowListModel::COLUMN_PERSO, sourceParent);

    if(filter_fav) {
        // If we have our favorite filter enabled, don't show the item if it's not a favorite
        ShowListModel *model = (ShowListModel*)sourceModel();
        if(!model->isFavorite(sourceRow))
            return false;
    }

    if(!filterRegExp().isEmpty()) {
        return (sourceModel()->data(index_id).toString().contains(filterRegExp())
                || sourceModel()->data(index_title).toString().contains(filterRegExp())
                || sourceModel()->data(index_perso).toString().contains(filterRegExp()));
    }

    return true;
}

void ShowListProxyModel::setFilterFav(bool filter)
{
    filter_fav = filter;
    invalidateFilter();
}
