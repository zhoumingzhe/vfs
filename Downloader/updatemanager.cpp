#include <stdlib.h>
#include "updatemanager.h"
#include "qtextstream.h"
#include "../Md5Utils/Md5Utils.h"
bool UpdateEntry::operator==( const UpdateEntry & rhs ) const
{
    return name == rhs.name && size == rhs.size && !memcmp(md5, rhs.md5, sizeof(md5));
}

UpdateManager::UpdateManager(DownloadManager* manager, HashChecker* checker, QObject *parent)
    : QObject(parent), m_pMgr(manager), m_pChecker(checker)
{
    QObject::connect(m_pChecker, SIGNAL(finishcheck(const CheckedEntry&)),
        this, SLOT(FileCheckComplete(const CheckedEntry&)));
    QFile version("versionurl_local.txt");
    if(version.open(QIODevice::ReadOnly))
    {
        m_LocalVersion = version.readAll().constData();
        LoadUpdateEntry(m_mapLocalEntry, "hash_local.txt");
    }
}

UpdateManager::~UpdateManager()
{

}

void UpdateManager::StartUpdate()
{
    QList<DownloadEntry> arguments;

    QList<QPair<qint64, qint64> > lst;
    lst.push_back(QPair<qint64, qint64>(0,4));
    lst.push_back(QPair<qint64, qint64>(5,2));

    arguments.append(DownloadEntry(
        QUrl::fromEncoded(QString("http://localhost/versionurl.txt").toLocal8Bit())));
    m_pMgr->append(arguments);
    QObject::disconnect(m_pMgr, SIGNAL(filecomplete(const QString&)), this, 0);
    QObject::connect(m_pMgr, SIGNAL(filecomplete(const QString&)),
        this, SLOT(VersionDownloadURLComplete(const QString &)));

}

void UpdateManager::VersionDownloadURLComplete( const QString &name )
{
    printf("version %s downloaded\n", name.toStdString().c_str());
    QFile version(name);
    version.open(QIODevice::ReadOnly);

    QObject::disconnect(m_pMgr, SIGNAL(filecomplete(const QString&)), this, 0);

    m_RemoteVersion = version.readAll().constData();
    if(m_RemoteVersion != m_LocalVersion)
    {
        QString base = QString("http://localhost/") + m_RemoteVersion + "/hash.txt";
        QList<DownloadEntry> arguments;
        arguments.append(DownloadEntry(QUrl::fromEncoded(base.toLocal8Bit())));

        QObject::connect(m_pMgr, SIGNAL(filecomplete(const QString&)),
            this, SLOT(HashFileDownloadComplete(const QString &)));
        m_pMgr->append(arguments);
    }
}

void UpdateManager::HashFileDownloadComplete( const QString &name )
{
    printf("hash file %s downloaded\n", name.toStdString().c_str());

    QObject::disconnect(m_pMgr, SIGNAL(filecomplete(const QString&)), this, 0);
    LoadUpdateEntry(m_mapRemoteEntry, name);
    GenerateUpdateList();
    DoUpdate();
}

void UpdateManager::HashSetDownloadComplete( const QString &name )
{

}

void UpdateManager::FileDownloadComplete( const QString &name )
{

}

void UpdateManager::LoadUpdateEntry( QMap<QString, UpdateEntry>& entry, QString name )
{
    entry.clear();
    QFile hash_file(name);
    hash_file.open(QIODevice::ReadOnly);
    QTextStream stream(&hash_file);
    UpdateEntry e;
    while(!stream.atEnd())
    {
        stream>>e.name;
        if(stream.atEnd())
            break;
        stream>>e.size;
        if(stream.atEnd())
            break;
        char buffer [33];
        stream>>buffer;
        buffer[32] = 0;
        if(stream.atEnd())
            break;
        StringToHash(buffer, e.md5);
        entry.insert(e.name, e);
    }
}

void UpdateManager::GenerateUpdateList()
{
    m_mapUpdate.clear();
    for(QMap<QString, UpdateEntry>::iterator it = m_mapRemoteEntry.begin();
        it != m_mapRemoteEntry.end(); ++it)
    {
        QMap<QString, UpdateEntry>::iterator itlocal = m_mapLocalEntry.find(it->name);
        if(itlocal != m_mapLocalEntry.end() && *it == *itlocal)
        {
            printf("skipped %s \n",it->name.toStdString().c_str());
            continue;
        }
        else
        {
            printf("need to download %s \n",it->name.toStdString().c_str());
            m_mapUpdate.insert(it->name, *it);
        }
    }
}

void UpdateManager::DoUpdate()
{
    QList<QString> entry;

    for(QMap<QString, UpdateEntry>::iterator it = m_mapUpdate.begin(); 
        it != m_mapUpdate.end(); ++it)
    {
        QString name = it->name + ".hs";
        if(m_mapUpdate.find(name) == m_mapUpdate.end())
        {
            entry.append(it->name);
            printf("to be downloaded %s\n", it->name.toStdString().c_str());
        }
        else
        {
            printf("pending %s\n", it->name.toStdString().c_str());
        }
    }
    m_pChecker->append(entry);
}

void UpdateManager::FileCheckComplete(const CheckedEntry& entry )
{
    QList<DownloadEntry> entries;
    QString base = QString("http://localhost/") + m_RemoteVersion + "/" + entry.name;
    entries.append(DownloadEntry(QUrl::fromEncoded(base.toLocal8Bit()), entry.filesize, entry.chunks));
    m_pMgr->append(entries);
}

