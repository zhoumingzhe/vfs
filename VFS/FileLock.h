#pragma once

class FileLock
{
public:
    FileLock(void);
    ~FileLock(void);

    void Lock();
    void Unlock();
private:
    class Member;
    Member* m_p;
};

class FileAutoLock
{
public:
    FileAutoLock(FileLock* pLock);
    ~FileAutoLock();
private:
    FileLock* m_pLock;
};