#include "FileLock.h"
#include <windows.h>

class FileLock::Member
{
public:
    Member();
    ~Member();
    CRITICAL_SECTION m_section;
};

FileLock::Member::Member()
{
    InitializeCriticalSection(&m_section);
}

FileLock::Member::~Member()
{
    DeleteCriticalSection(&m_section);
}

FileLock::FileLock(void):m_p(new Member)
{
}


FileLock::~FileLock(void)
{
    delete m_p;
}

void FileLock::Lock()
{
    EnterCriticalSection(&m_p->m_section);
}

void FileLock::Unlock()
{
    LeaveCriticalSection(&m_p->m_section);
}

FileAutoLock::FileAutoLock( FileLock* pLock ):m_pLock(pLock)
{
    m_pLock->Lock();
}

FileAutoLock::~FileAutoLock()
{
    m_pLock->Unlock();
}
