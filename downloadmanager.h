#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQueue>
#include <QFile>

#include <show.h>

typedef QPair<int, Show> QueuePair;

class DownloadManager : public QObject
{
    Q_OBJECT

public:
    explicit DownloadManager(QObject *parent = 0);

    enum Attribs {
        ATTRIB_FILE = 1000,
        ATTRIB_ID,
        ATTRIB_ROW
    };

    void singleShot(QString url);
    void queueShowDownload(int row, const Show & cur_show);
    void unqueueShowDownload(const Show & cur_show);
    void cancelShowDownload(const Show & cur_show);
    void downloadThumb(QString url);

    static QString makeShowFilename(const Show & cur_show, bool incomplete = false);

signals:
    void gotProgress(qint64 bytesReceived, qint64 bytesTotal);
    void gotData(QByteArray data);
    void thumbFinished(QByteArray data, QUrl url);

    void queueStarted();
    void showQueued(int show_num);
    void showUnqueued(int show_num);
    void showStarted(int show_num);
    void showProgress(int show_num, int percent);
    void showFinished(int show_num);
    void showCanceled(int show_num);
    void showError(int show_num);
    void queueFinished();

public slots:

private slots:
    void onSingleShotProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onSingleShotFinished();
    void onThumbFinished();
    void onShowProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onShowReadyRead();
    void onShowFinished();
    void onShowError(QNetworkReply::NetworkError);

private:
    QNetworkAccessManager manager;

    QHash<QString, QFile*> files;
    QHash<QString, QNetworkReply*> downloads;
    QQueue<QueuePair> queue;

    QString dataDir;

    void nextShowDownload();
    void finishFile(const QString & filename, bool rename);
};

#endif // DOWNLOADMANAGER_H
