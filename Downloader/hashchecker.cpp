#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QFile>
#include "hashchecker.h"
#include "../VFS/IFile.h"
#include "../Md5Utils/Md5Utils.h"
#include <assert.h>

HashChecker::HashChecker(QObject *parent)
    : QObject(parent)
{
    connect(&watcher, SIGNAL(finished()), this, SLOT(OnCheckFinish()));
    assert(!working.isRunning());
}

HashChecker::~HashChecker()
{

}

void HashChecker::append(const QList<QString> &names)
{
    if (checkQueue.isEmpty() && !working.isRunning())
        QTimer::singleShot(0, this, SLOT(checkNext()));

    foreach (QString entry, names)
    {
        checkQueue.enqueue(entry);
    }
}

void HashChecker::checkNext()
{
    if (checkQueue.isEmpty()) {
        emit finished();
        return;
    }
    QString entry = checkQueue.dequeue();
    working = QtConcurrent::run(this, &HashChecker::check, entry);
    watcher.setFuture(working);
}

CheckedEntry HashChecker::check( QString name )
{
    unsigned char hash[16];
    std::vector<Md5Digest> hash_set;
    offset_type length_remote;
    bool needdownload = false;
    QList<QPair<qint64, qint64> > chunks;
    std::string stdname = name.toStdString();
    if(QFile::exists(name + ".hs"))
    {
        //compare hash set generate partial download
        offset_type length;
        std::vector<Md5Digest>hash_set_remote;
        LoadHashSet((name + ".hs").toStdString(), hash_set_remote, length_remote);
        GenerateHash(stdname, hash, hash_set, length);
        size_t i = 0;
        for(i = 0; i<hash_set.size() && i<hash_set_remote.size(); ++i)
        {
            if(hash_set[i] != hash_set_remote[i])
            {
                if(!chunks.empty())
                {
                    QPair<qint64, qint64>& p = chunks.last();
                    if(p.first + p.second == i*block_size.offset)
                        p.second+=block_size.offset;
                    else
                        chunks.append(QPair<qint64, qint64>(i*block_size.offset, block_size.offset));
                }
                else
                    chunks.append(QPair<qint64, qint64>(i*block_size.offset, block_size.offset));
            }
        }
        if(i<hash_set_remote.size())
        {
            if(!chunks.empty())
            {
                QPair<qint64, qint64>& p = chunks.last();
                if(p.first + p.second == i*block_size.offset)
                    p.second = length_remote.offset - p.first;
                else
                    chunks.append(QPair<qint64, qint64>(i*block_size.offset,
                    length_remote.offset - i*block_size.offset));
            }
            else
                chunks.append(QPair<qint64, qint64>(i*block_size.offset,
                    length_remote.offset - i*block_size.offset));
        }
        needdownload = chunks.size()>0;
    }
    else
    {
        length_remote.offset = -1;
        needdownload = true;
    }
    return CheckedEntry(name, needdownload, length_remote.offset, chunks);
}

void HashChecker::OnCheckFinish()
{
    CheckedEntry entry = working.result();
    if(entry.needdownload)
    {
        printf("pendding download %s\n", entry.name.toStdString().c_str());
        emit finishcheck(entry);
    }
    else
    {
        printf("skipped download %s\n", entry.name.toStdString().c_str());
    }

    checkNext();
}
