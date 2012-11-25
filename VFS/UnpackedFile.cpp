#include <assert.h>
#include "UnpackedFile.h"
#include "BlockFS.h"

UnpackedFile::UnpackedFile(BlockFS* pFS, OpenMode mode, int beginid):
    m_pFS(pFS),
    m_Current(0),
    m_Cacheid(beginid),
    m_Beginid(beginid),
    m_CacheChanged(false),
    m_CacheSeq(0)
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
    //out of range
    if(m_Current >= m_Header.datasize)
        return 0;

    int offset_in_cache = m_Current + sizeof(UnpackedFileHeader) - m_CacheSeq*m_Cache.data.size();
    assert(offset_in_cache < (int)m_Cache.data.size());

    while((int)m_Cache.data.size() < offset_in_cache + size)
    {
        int bytes_to_copy = m_Cache.data.size() - offset_in_cache;
        memcpy(buffer, &m_Cache.data[0], bytes_to_copy);

        //advance to next block
        buffer =(char*)buffer + bytes_to_copy;
        size -= bytes_to_copy;
        offset_in_cache = 0;

        m_Current += bytes_to_copy;
    }
    return 0;
}

int UnpackedFile::Write( const void* buffer, int size )
{
    // 3. load the cache to be written
    // 4. mark the cache as modified
    // 5. write data in one cache
    // 6. if more data flush and goto 3
    //////////////////////////////////////////////////////////////////////////

    // 1. seek to right position,allocate disk space if necessary
    int offset = CalcOffsetInCache(m_Current);
    int blocksize = m_pFS->GetBlockDataSize();
    if (offset >= blocksize)
    {
        if(m_CacheChanged)
        {
            m_pFS->FlushCache(m_Cacheid, m_Cache);
        }
        while (offset >= blocksize)
        {
            //we reach the last block
            if(m_Cache.header.next == m_Beginid)
            {
                BlockHeader header = {m_Beginid, m_Cacheid};
                int i = m_pFS->AllocBlock(header);

            }

            m_Cacheid = m_Cache.header.next;
            ++m_CacheSeq;
            m_pFS->LoadCache(m_Cacheid, m_Cache);
            return true;
        }
    }
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
        m_CacheChanged = true;
    }
}

bool UnpackedFile::AdvancedToNextCache()
{
    if(m_CacheChanged)
    {
        m_pFS->FlushCache(m_Cacheid, m_Cache);
    }

    //we reach the last block
    if(m_Cache.header.next == m_Beginid)
        return false;

    m_Cacheid = m_Cache.header.next;
    ++m_CacheSeq;
    m_pFS->LoadCache(m_Cacheid, m_Cache);
    return true;
}

int UnpackedFile::CalcOffsetInCache( int offset )
{
    return offset + sizeof(UnpackedFileHeader) - m_CacheSeq*m_pFS->GetBlockDataSize();
}
