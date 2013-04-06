#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QMap>
#include "downloadmanager.h"
#include "hashchecker.h"
struct UpdateEntry
{
    QString name;
    qint64 size;
    unsigned char md5[16];
    bool operator == (const UpdateEntry & rhs) const;

};
class UpdateManager : public QObject
{
    Q_OBJECT

public:
    UpdateManager(DownloadManager* manager, HashChecker *checker, QObject *parent = 0);
    ~UpdateManager();
    void StartUpdate();
    void LoadUpdateEntry(QMap<QString, UpdateEntry>& entry, QString name);
    void GenerateUpdateList();
    void DoUpdate();
public slots:
    void VersionDownloadURLComplete(const QString &name);
    void HashFileDownloadComplete(const QString &name);
    void HashSetDownloadComplete(const QString &name);
    void FileDownloadComplete(const QString &name);

    void FileCheckComplete(const CheckedEntry& entry);
private:
    DownloadManager* m_pMgr;
    HashChecker* m_pChecker;
    QString m_LocalVersion;
    QString m_RemoteVersion;
    QMap<QString, UpdateEntry> m_mapLocalEntry;
    QMap<QString, UpdateEntry> m_mapRemoteEntry;
    QMap<QString, UpdateEntry> m_mapUpdate;
};

#endif // UPDATEMANAGER_H
