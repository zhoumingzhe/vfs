#include <assert.h>
#include "UnpackedFile.h"
#include "BlockFS.h"

UnpackedFile::UnpackedFile(BlockFS* pFS, OpenMode mode, int beginid):
    m_pFS(pFS),
    m_Current(0),
    m_Cacheid(beginid),
    m_Beginid(beginid)
{
    m_pFS->LoadCache(beginid, m_Cache);
    //for simplicity;
    assert(m_Cache.data.size() >= sizeof(UnpackedFileHeader));
    memcpy(&m_Header, &m_Cache.data[0], sizeof(m_Header));

}


UnpackedFile::~UnpackedFile(void)
{
    m_pFS->OnFileDestory(this);
}

int UnpackedFile::Read( void* buffer, int size )
{
    return 0;
}

int UnpackedFile::Write( const void* buffer, int size )
{
    return 0;
}

int UnpackedFile::Seek( int pos, enum SeekMode mode )
{
    switch (mode)
    {
    case IFile::S_Current:
        break;
    case IFile::S_End:
        break;
    case IFile::S_Begin:
        break;
    default:
        assert(0);
        break;
    }
    return 0;
}

int UnpackedFile::GetSize()
{
    return 0;
}

int UnpackedFile::ReserveSpace( int size )
{
    return 0;
}

int UnpackedFile::GetRefCount()
{
    return m_Header.refcount;
}

void UnpackedFile::SetRefCount(int refcount)
{
    m_Header.refcount = refcount;
    FlushHeaderToCache();
}

int UnpackedFile::GetDataSize()
{
    return m_Header.datasize;
}

void UnpackedFile::SetDataSize( int datasize )
{
    m_Header.datasize = datasize;
    FlushHeaderToCache();
}

void UnpackedFile::FlushHeaderToCache()
{
    if(m_Cacheid == m_Beginid) //flush the header to cache
    {
        assert(m_Cache.data.size() >= sizeof(m_Header));
        memcpy(&m_Cache.data[0], &m_Header, sizeof(m_Header));
    }
}
