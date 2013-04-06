#ifndef HASHCHECKER_H
#define HASHCHECKER_H

#include <QObject>
#include <QQueue>
#include <QFuture>
#include <QFutureWatcher>

struct CheckedEntry
{
    QString name;
    qint64 filesize;
    bool needdownload;
    QList<QPair<qint64, qint64> > chunks;
    CheckedEntry(
        const QString & u=QString(""),
        bool n = true,
        qint64 fsize = -1,
        const QList<QPair<qint64, qint64> > &cks = QList<QPair<qint64, qint64> >() )
        :name(u), needdownload(n), filesize(fsize), chunks(cks)
    {}
//     CheckedEntry(const CheckedEntry& e):
//         name(e.name),
//         filesize(e.filesize),
//         needdownload(e.needdownload),
//         chunks(e.chunks)
//     {}
};

class HashChecker : public QObject
{
    Q_OBJECT

public:
    HashChecker(QObject *parent = 0);
    ~HashChecker();
    void append(const QList<QString> &names);
    CheckedEntry check(QString name);
signals:
    void finished();
    void finishcheck(const CheckedEntry& entry);
private slots:
    void checkNext();
    void OnCheckFinish();
private:
    QQueue<QString> checkQueue;
    QFuture<CheckedEntry> working;
    QFutureWatcher<CheckedEntry> watcher;
};

#endif // HASHCHECKER_H
