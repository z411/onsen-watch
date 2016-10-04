#include "downloadmanager.h"
#include "globs.h"

#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>
#include <QDir>

DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
{
}

void DownloadManager::singleShot(QString url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(onSingleShotProgress(qint64,qint64)));
    connect(reply, SIGNAL(finished()), SLOT(onSingleShotFinished()));
}

void DownloadManager::downloadThumb(QString url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    connect(reply, SIGNAL(finished()), SLOT(onThumbFinished()));
}

void DownloadManager::queueShowDownload(int row, const Show & cur_show)
{
    // Queue the following show for download and include its main attributes
    qDebug() << "Queueing show download.";

    if(queue.empty()) emit queueStarted();

    QueuePair item = qMakePair(row, cur_show);
    queue.enqueue(item);
    emit showQueued(row);
    nextShowDownload();
}

void DownloadManager::unqueueShowDownload(const Show & cur_show)
{
    QMutableListIterator<QueuePair> i(queue);
    while(i.hasNext()) {
        QueuePair & item = i.next();

        int row = item.first;
        Show & i_show = item.second;

        if(i_show.id == cur_show.id && i_show.ep == cur_show.ep) {
            emit showUnqueued(row);
            i.remove();
            return;
        }
    }
}

void DownloadManager::cancelShowDownload(const Show & cur_show)
{
    QString filename = DownloadManager::makeShowFilename(cur_show, true);
    if(downloads.contains(filename)) {
        downloads.value(filename)->abort();
    }
}

void DownloadManager::nextShowDownload()
{
    if(!queue.empty()) {
         // Don't download if we hit the maximum download limit
        if(downloads.size() >= 1) {
            qDebug() << "Queued for later.";
            return;
        }

        qDebug() << "Initializing download.";

        // Get the show to download and choose our URL and filename
        QueuePair item = queue.dequeue();

        int row = item.first;
        Show cur_show = item.second;

        QString filename = DownloadManager::makeShowFilename(cur_show, true);
        qDebug() << "Downloading to " + filename;

        QUrl url(cur_show.file);

        if (downloads.contains(filename))
            return;

        qDebug() << "Opening file.";
        QDir().mkdir("download");
        QFile *file = new QFile(filename);

        if (!file->open(QIODevice::WriteOnly)) {
            qErrnoWarning("Error while opening %s for write", qPrintable(filename));
            delete file;
            return;
        }

        // Create our download request and fill it with file and show info
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::Attribute(ATTRIB_FILE), filename);
        request.setAttribute(QNetworkRequest::Attribute(ATTRIB_ID), cur_show.id);
        request.setAttribute(QNetworkRequest::Attribute(ATTRIB_ROW), row);

        // Start download and connect the show signals
        qDebug() << "Starting download." << request.url().fileName();
        QNetworkReply *reply = manager.get(request);
        downloads.insert(filename, reply);
        files.insert(filename, file);

        connect(reply, SIGNAL(readyRead()), SLOT(onShowReadyRead()));
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(onShowProgress(qint64,qint64)));
        connect(reply, SIGNAL(finished()), SLOT(onShowFinished()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onShowError(QNetworkReply::NetworkError)));

        emit showStarted(row);
    } else {
        qDebug() << "Queue is finished.";
        emit queueFinished();
    }
}

void DownloadManager::finishFile(const QString & filename, bool rename)
{
    QFile *file = files[filename];
    qDebug() << file;
    if(file) {
        file->close();

        if(rename) {
            QString new_filename(filename);
            new_filename.replace("inc_", "");
            file->rename(new_filename);
        }
        file->deleteLater();
        files.remove(filename);
    }
}

void DownloadManager::onSingleShotProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit gotProgress(bytesReceived, bytesTotal);
}

void DownloadManager::onSingleShotFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    emit gotData(reply->readAll());

    reply->deleteLater();
}

void DownloadManager::onThumbFinished()
{
    if (QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender())) {
        if(reply->error() == QNetworkReply::NoError) {
            // Save image file to cache
            QString filename = reply->request().url().fileName();
            QByteArray data = reply->readAll();

            QDir().mkdir("cache");
            QFile file("cache/" + filename);
            file.open(QIODevice::WriteOnly);
            file.write(data);
            file.flush();
            file.close();

            emit thumbFinished(data, reply->request().url());
        }

        reply->deleteLater();
    }
}

void DownloadManager::onShowReadyRead()
{
    if (QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender())) {
        QString filename = reply->request().attribute(QNetworkRequest::Attribute(ATTRIB_FILE)).toString();
        if(files.contains(filename)) {
            QFile *file = files.value(filename);
            file->write(reply->readAll());
        }
    }
}

void DownloadManager::onShowProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender())) {
        float prog = (float)bytesReceived / (float)bytesTotal;
        int progress = prog * 100;

        if(reply->error() == QNetworkReply::NoError)
            emit showProgress(reply->request().attribute(QNetworkRequest::Attribute(ATTRIB_ROW)).toInt(), progress);
    }
}

void DownloadManager::onShowFinished()
{
    qDebug() << "hit finish";

    if (QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender())) {
        QString filename = reply->request().attribute(QNetworkRequest::Attribute(ATTRIB_FILE)).toString();

        // Rename our file only if it was fully downloaded
        switch(reply->error())
        {
            case QNetworkReply::NoError:
                finishFile(filename, true);
                emit showFinished(reply->request().attribute(QNetworkRequest::Attribute(ATTRIB_ROW)).toInt());
                break;
            case QNetworkReply::OperationCanceledError:
                emit showCanceled(reply->request().attribute(QNetworkRequest::Attribute(ATTRIB_ROW)).toInt());
            default:
                finishFile(filename, false);
        }

        downloads.remove(filename);
        reply->deleteLater();
        nextShowDownload();
    }
}

void DownloadManager::onShowError(QNetworkReply::NetworkError code) {
    if (QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender())) {
        //qDebug() << code.errorString();
        if(code != QNetworkReply::OperationCanceledError)
            emit showError(reply->request().attribute(QNetworkRequest::Attribute(ATTRIB_ROW)).toInt());
    }
}

QString DownloadManager::makeShowFilename(const Show & cur_show, bool incomplete)
{
    return (incomplete ? "download/inc_" : "download/") + cur_show.id + "_" + QString::number(cur_show.ep) + ".mp3";
}
