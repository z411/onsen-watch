#include "dateitem.h"

DateItem::DateItem()
{

}

DateItem::DateItem( const QDate & date )
{
    setDate(date);
}

void DateItem::setDate( const QDate & date )
{
    m_date = date;
    setText(m_date.toString("MM.dd"));
}

QDate DateItem::date() const
{
    return m_date;
}

bool DateItem::operator< ( const DateItem & other ) const
{
    return (m_date < other.date());
}
