#ifndef SHOWVIEWDELEGATE_H
#define SHOWVIEWDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class ShowViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ShowViewDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // SHOWVIEWDELEGATE_H
