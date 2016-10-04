#ifndef APIONSEN_H
#define APIONSEN_H

#include <QXmlStreamReader>
#include <QDate>

#include <libxml/tree.h>
#include <libxml/HTMLparser.h>

#include <show.h>

class ApiOnsen
{
public:
    ApiOnsen();

    QVector<Show> getShows() { return shows; }
    QDate latestDate() { return latest_date; }

    QVector<Show> read(QByteArray data);
    void parseWeek(xmlNode *node);
    void parseShow(xmlNode *node);

private:
    QXmlStreamReader xml;
    QVector<Show> shows;
    QDate latest_date;
};

#endif // APIONSEN_H
