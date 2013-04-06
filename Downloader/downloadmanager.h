#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QFile>
#include <QObject>
#include <QQueue>
#include <QUrl>
#include <QNetworkAccessManager>

struct DownloadEntry
{
    QUrl url;
    qint64 filesize;
    QList<QPair<qint64, qint64> > chunks;
    DownloadEntry(
        const QUrl & u,
        qint64 fsize = -1,
        const QList<QPair<qint64, qint64> > &cks = QList<QPair<qint64, qint64> >() )
    :url(u), filesize(fsize), chunks(cks)
    {}
};

class DownloadManager: public QObject
{
    Q_OBJECT
public:
    DownloadManager(QObject *parent = 0);

    void append(const QList<DownloadEntry> &downloadlist);
    QString saveFileName(const QUrl &url);

signals:
    void finished();
    void filecomplete(const QString& name);

private slots:
    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();

    void startNextChunk();
private:
    QNetworkAccessManager manager;
    QQueue<DownloadEntry> downloadQueue;

    QUrl currenturl;
    QNetworkReply *currentDownload;
    QFile output;
    QList<QPair<qint64, qint64> > chunks;
};

#endif
