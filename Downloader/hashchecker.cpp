#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QFile>
#include "hashchecker.h"
#include "../VFS/IFile.h"
#include "../Md5Utils/Md5Utils.h"

HashChecker::HashChecker(QObject *parent)
    : QObject(parent)
{
    connect(&watcher, SIGNAL(finished()), this, SLOT(OnCheckFinish()));
}

HashChecker::~HashChecker()
{

}

void HashChecker::append(const QList<QString> &names)
{
    if (checkQueue.isEmpty())
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
    offset_type length;
    bool needdownload = false;
    QList<QPair<qint64, qint64> > chunks;
    std::string stdname = name.toStdString();
    if(QFile::exists(name + ".hs"))
    {
        //compare hash set generate partial download
        std::vector<Md5Digest>hash_set_remote;
        offset_type length_remote;
        LoadHashSet((name + ".hs").toStdString(), hash_set_remote, length_remote);
        GenerateHash(stdname, hash, hash_set, length);
        printf("todo: check hashset %s and download \n", name.toStdString().c_str());
        size_t i = 0;
        for(i = 0; i<hash_set.size() && i<hash_set_remote.size(); ++i)
        {
            if(hash_set[i] != hash_set_remote[i])
            {
                if(!chunks.empty())
                {
                    QPair<qint64, qint64>& p = chunks.last();
                    if(p.first + p.second == i*block_size.offset)
                        p.second+=i*block_size.offset;
                    else
                        chunks.append(QPair<qint64, qint64>(i*block_size.offset, block_size.offset));
                }
            }
        }
        needdownload = chunks.size()>0;
    }
    else
    {
        //todo: download directly
        length.offset = -1;
        needdownload = true;
        printf("todo: direct download %s \n", name.toStdString().c_str());
    }
    return CheckedEntry(name, needdownload, length.offset, chunks);
}

void HashChecker::OnCheckFinish()
{
    CheckedEntry entry = working.result();
    printf("check result %s \n", entry.name.toStdString().c_str());

    if(entry.needdownload)
        emit finishcheck(entry);

    checkNext();
}
