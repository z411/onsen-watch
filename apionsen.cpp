#include "globs.h"
#include "apionsen.h"

#include <QFile>
#include <QDebug>
#include <QStringList>

#define ONSEN_URL "http://www.onsen.ag"

ApiOnsen::ApiOnsen()
{
    LIBXML_TEST_VERSION
}

void ApiOnsen::parseWeek(xmlNode *node)
{
    qDebug() << "Parsing week" << QString::fromUtf8((char*)xmlGetProp(node, (xmlChar*)"id"));

    xmlNode *cur_node = NULL;
    for(cur_node = node->children; cur_node; cur_node = cur_node->next) {
        /*qDebug() << QString::fromUtf8((char*)cur_node->name);
        qDebug() << QString::fromUtf8((char*)xmlGetProp(cur_node, (xmlChar*)"class"));*/

        if(xmlStrEqual(xmlGetProp(cur_node, (xmlChar*)"class"), (xmlChar*)"programContsWrap")) {
            cur_node = cur_node->children;
        } else if(xmlStrEqual(xmlGetProp(cur_node, (xmlChar*)"class"), (xmlChar*)"programConts")) {
            parseShow(cur_node);
        }
    }
}

void ApiOnsen::parseShow(xmlNode *node)
{
    //qDebug() << "Parsing show";
    Show cur_show;

    xmlNode *cur_node = NULL;
    for(cur_node = node->children; cur_node; cur_node = cur_node->next) {
        /*qDebug() << QString::fromUtf8((char*)cur_node->name);
        qDebug() << QString::fromUtf8((char*)xmlGetProp(cur_node, (xmlChar*)"class"));*/

        if(xmlStrEqual(xmlGetProp(cur_node, (xmlChar*)"class"), (xmlChar*)"programData")) {
            // Main info
            xmlNode *in_node = cur_node->children->next;
            cur_show.id    = QString::fromUtf8((char*)xmlGetProp(in_node, (xmlChar*)"id"));
            QString title = QString::fromUtf8((char*)in_node->children->content);

            in_node = in_node->next->next;

            if(in_node->children != NULL)
                cur_show.personality = QString::fromUtf8((char*)in_node->children->content);

            // Split title and episode number
            QStringList parts = title.split("#");
            cur_show.title = parts[0].trimmed();
            if(parts.length() > 1) {
                cur_show.ep = parts[1].toInt();
            }
        } else if(xmlStrEqual(xmlGetProp(cur_node, (xmlChar*)"class"), (xmlChar*)"programLogo")) {
            // Logo
            xmlNode *in_node = cur_node->children;
            cur_show.img_url = ONSEN_URL + QString::fromUtf8((char*)xmlGetProp(in_node, (xmlChar*)"src"));
        } else if(xmlStrEqual(xmlGetProp(cur_node, (xmlChar*)"class"), (xmlChar*)"programLink")) {
            // Page and email
            xmlNode *in_node = cur_node->children->next;
            cur_show.url = ONSEN_URL + QString::fromUtf8((char*)xmlGetProp(in_node->children, (xmlChar*)"href"));

            in_node = in_node->next->next;
            cur_show.email = QString::fromUtf8((char*)xmlGetProp(in_node->children, (xmlChar*)"href"));
        } else if(xmlStrEqual(xmlGetProp(cur_node, (xmlChar*)"class"), (xmlChar*)"playBtn")) {
            // Movie
            xmlNode *in_node = cur_node->children->next;
            cur_show.file = QString::fromUtf8((char*)xmlGetProp(in_node, (xmlChar*)"action"));
        } else if(xmlStrEqual(xmlGetProp(cur_node, (xmlChar*)"class"), (xmlChar*)"update")) {
            QString update = QString::fromUtf8((char*)xmlNodeGetContent(cur_node));
            update.chop(3);
            cur_show.update = QDate::fromString(update, "M/d");
            cur_show.update = cur_show.update.addYears(116);
            if(cur_show.update > QDate::currentDate())
                cur_show.update = cur_show.update.addYears(-1);
            if(cur_show.update > latest_date)
                latest_date = cur_show.update;
        }
    }

    cur_show.valid = true;
    shows.append(cur_show);
}

QVector<Show> ApiOnsen::read(QByteArray data)
{
    shows.clear();

    //xmlDoc *doc = htmlReadFile("easy.html", NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);

    const char *c_data = data.constData();
    xmlDoc *doc = htmlReadDoc((xmlChar*)c_data, NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc == NULL) {
        qDebug() << "could not parse HTML file";
    }

    xmlNode *r = xmlDocGetRootElement(doc);
    xmlNode *cur_node = NULL;

    for(cur_node = r->children; cur_node; cur_node = cur_node->next) {
        /*qDebug() << QString::fromUtf8((char*)cur_node->name);
        qDebug() << QString::fromUtf8((char*)xmlGetProp(cur_node, (xmlChar*)"class"));*/

        if(xmlStrEqual(cur_node->name, (xmlChar*)"body")) {
            // Starting
            qDebug() << "found body";
            cur_node = cur_node->children;
        } else if(xmlStrEqual(xmlGetProp(cur_node, (xmlChar*)"id"), (xmlChar*)"weekContsWrap")) {
            qDebug() << "found week contswrap";
            cur_node = cur_node->children;
            parseWeek(cur_node);
        } else if(xmlStrEqual(xmlGetProp(cur_node, (xmlChar*)"class"), (xmlChar*)"weekConts")) {
            parseWeek(cur_node);
        } else {

        }
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return shows;
}
