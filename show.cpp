#include "show.h"

Show::Show() : ep(0), valid(false)
{

}

QDataStream & operator>>(QDataStream & in, Show & show)
{
    //in >> show.num;
    in >> show.id;
    in >> show.title;
    in >> show.personality;
    in >> show.url;
    in >> show.img_url;
    in >> show.email;
    in >> show.ep;
    in >> show.file;
    in >> show.update;

    show.valid = true;

    return in;
}

QDataStream & operator<<(QDataStream & out, const Show & show)
{
    //out << show.num;
    out << show.id;
    out << show.title;
    out << show.personality;
    out << show.url;
    out << show.img_url;
    out << show.email;
    out << show.ep;
    out << show.file;
    out << show.update;

    return out;
}
