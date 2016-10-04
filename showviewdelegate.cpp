#include "showviewdelegate.h"

#include "globs.h"
#include "showlistmodel.h"
#include <QApplication>
#include <QPainter>

ShowViewDelegate::ShowViewDelegate()
{

}

void ShowViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int m = 2;
    if (index.column() == ShowListModel::COLUMN_DOWNLOAD) {
        if(index.data().canConvert<ProgressPair>()) {
            ProgressPair progress = qvariant_cast<ProgressPair>(index.data());
            int x = option.rect.x();
            int y = option.rect.y();
            int w = option.rect.width();
            int h = option.rect.height();

            painter->save();
            painter->setPen(Qt::NoPen);

            switch(progress.first)
            {
                case DOWNLOAD_STARTED:
                    painter->setBrush(QBrush(QColor(160, 240, 160)));
                    break;
                case DOWNLOAD_QUEUED:
                    painter->setBrush(QBrush(QColor(240, 240, 160)));
                    break;
                case DOWNLOAD_FINISHED:
                    painter->setBrush(QBrush(QColor(160, 250, 160)));
                    break;
                case DOWNLOAD_ERROR:
                    painter->setBrush(QBrush(QColor(240, 160, 160)));
                    break;
                case DOWNLOAD_READY:
                default:
                    painter->setBrush(QBrush(QColor(240, 240, 240)));
            }

            painter->drawRect(x+m, y+m, w-m*2, h-m*2);

            if(progress.first >= 0 && progress.first <= 100) {
                // Draw percent bar
                painter->setBrush(QBrush(QColor(160, 160, 240)));
                painter->drawRect(x+m, y+m, ((w-m*2) * progress.first / 100), h-m*2);
            }

            QPen pen(Qt::SolidLine);


            if(progress.first == DOWNLOAD_READY || progress.first == DOWNLOAD_FINISHED)
            {
                pen.setColor(QColor(Qt::blue));
                QFont font = painter->font();
                font.setUnderline(true);
                painter->setFont(font);
            } else {
                pen.setColor(QColor(Qt::black));
            }

            painter->setPen(pen);
            painter->drawText(option.rect, Qt::AlignCenter | Qt::AlignHCenter, progress.second);

            painter->restore();
        }
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
