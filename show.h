#ifndef SHOW_H
#define SHOW_H

#include <QString>
#include <QDate>
#include <QDataStream>

class Show
{
public:
    Show();

    //quint16 num;
    QString id;
    QString title;
    QString personality;
    QString url;
    QString img_url;
    QString email;
    quint16 ep;
    QString file;
    QDate update;

    bool valid;

};

QDataStream & operator>>(QDataStream & in, Show & show);
QDataStream & operator<<(QDataStream & out, const Show & show);

#endif // SHOW_H
