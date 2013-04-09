#include <QFileInfo>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QTimer>
#include <QDir>
#include <stdio.h>
#include "downloadmanager.h"
DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent),currentDownload(0)
{
}

void DownloadManager::append(const QList<DownloadEntry> &downloadlist)
{
    if (downloadQueue.isEmpty() && !this->currentDownload)
        QTimer::singleShot(0, this, SLOT(startNextDownload()));

    foreach (DownloadEntry entry, downloadlist)
    {
        downloadQueue.enqueue(entry);
    }
}

void DownloadManager::startNextDownload()
{
    if (downloadQueue.isEmpty()) {
        emit finished();
        return;
    }
    DownloadEntry entry = downloadQueue.dequeue();
    currenturl = QUrl::fromEncoded((entry.base + "/" + entry.name).toLocal8Bit());
    chunks = entry.chunks;
    QDir d;
    d.mkpath(QFileInfo(entry.name).dir().path());
    output.setFileName(entry.name);
    if (!output.open(QIODevice::ReadWrite)) {
        fprintf(stderr, "Problem opening save file '%s' for download '%s': %s\n",
                qPrintable(entry.name), currenturl.toEncoded().constData(),
                qPrintable(output.errorString()));

        startNextDownload();
        return;                 // skip this download
    }
    if(entry.filesize >= 0)
        output.resize(entry.filesize);
    else
        output.resize(0);

    // prepare the output
    printf("Downloading %s...\n", currenturl.toEncoded().constData());
    startNextChunk();
}

void DownloadManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{

}

void DownloadManager::downloadFinished()
{
    if (currentDownload->error()) {
        // download failed
        fprintf(stderr, "Failed: %s\n", qPrintable(currentDownload->errorString()));
    } else {
        output.write(currentDownload->readAll());
        printf("Succeeded.\n");
    }

    currentDownload->deleteLater();
    currentDownload = 0;

    if(chunks.size()>0)
        startNextChunk();
    else
    {
        output.close();
        QString name = output.fileName();
        emit filecomplete(name);
        startNextDownload();
    }
}

void DownloadManager::downloadReadyRead()
{
    if(!currentDownload->error()&& currentDownload->bytesAvailable()>1024)
        output.write(currentDownload->readAll());
}

void DownloadManager::startNextChunk()
{
    QNetworkRequest request(currenturl);
    if(chunks.size()>0)
    {
        const QPair<qint64, qint64> & range = chunks.front();
        QString r("bytes=%1-%2");
        r = r.arg(range.first).arg(range.first+range.second - 1);
        request.setRawHeader("Range", r.toLatin1());
        output.seek(range.first);
        chunks.pop_front();
    }
    currentDownload = manager.get(request);
    //     connect(currentDownload, SIGNAL(downloadProgress(qint64,qint64)),
    //             SLOT(downloadProgress(qint64,qint64)));
    connect(currentDownload, SIGNAL(finished()),
        SLOT(downloadFinished()));
    connect(currentDownload, SIGNAL(readyRead()),
        SLOT(downloadReadyRead()));
}
