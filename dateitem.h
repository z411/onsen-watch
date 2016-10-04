#ifndef DATEITEM_H
#define DATEITEM_H

#include <QDate>
#include <QTableWidgetItem>

class DateItem : public QTableWidgetItem
{
public:
    DateItem();
    DateItem( const QDate & date );

    QDate date() const;

    void setDate( const QDate & date );
    bool operator< ( const DateItem & other ) const;

private:
    QDate m_date;
};

#endif // DATEITEM_H
