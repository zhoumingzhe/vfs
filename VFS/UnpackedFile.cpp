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
    FlushCache();
    m_pFS->OnFileDestory(this);
}

int UnpackedFile::Read( void* buffer, int size )
{
    assert(0&&"not implemented");
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
    assert(size>=0);
    if(size <= 0)
        return 0;

    int offset = CalcOffsetInCurrentCache(m_Current);
    assert(offset >= 0);
    int blocksize = m_pFS->GetBlockDataSize();

    //Seek to the right position
    if (offset >= blocksize)
    {
        FlushCache();
        while (offset >= blocksize)
        {
            //we reach the last block,
            //allocate blocks at the end
            if(m_Cache.header.next == m_Beginid)
            {
                m_Cacheid = AppendBlock(m_Beginid, m_Cacheid);
            }

            ++m_CacheSeq;
            //need some optimization, just load the block header
            m_pFS->LoadCache(m_Cacheid, m_Cache);
            offset = CalcOffsetInCurrentCache(m_Current);
        }
    }

    //Write in Loop
    while (size > 0)
    {
        //Write in cache
        assert(offset < blocksize);
        int size_to_write = (blocksize - offset)<size?
            (blocksize - offset):size;
        assert((int)m_Cache.data.size() >= offset + size_to_write);
        memcpy(&m_Cache.data[offset], buffer, size_to_write);
        m_CacheChanged = true;
        FlushCache();

        //advance data
        m_Current += size_to_write;
        size -= size_to_write;

        if(size > 0)
        {
            buffer = (char*)buffer + size_to_write;
            //we reach the last block,
            //allocate blocks at the end
            if(m_Cache.header.next == m_Beginid)
            {
                m_Cacheid = AppendBlock(m_Beginid, m_Cacheid);
            }
            else
            {
                m_Cacheid = m_Cache.header.next;
            }
            ++m_CacheSeq;
            m_pFS->LoadCache(m_Cacheid, m_Cache);
            offset = CalcOffsetInCurrentCache(m_Current);
            assert(offset==0);
        }
    }

    if(m_Current > GetDataSize())
    {
        SetDataSize(m_Current);
        FlushHeaderToCache();
    }

    return size;
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
    assert(0&&"not implemented");
    return 0;
}

int UnpackedFile::GetSize()
{
    assert(0&&"not implemented");
    return 0;
}

int UnpackedFile::ReserveSpace( int size )
{
    assert(0&&"not implemented");
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
    FlushCache();

    //we reach the last block
    if(m_Cache.header.next == m_Beginid)
        return false;

    m_Cacheid = m_Cache.header.next;
    ++m_CacheSeq;
    m_pFS->LoadCache(m_Cacheid, m_Cache);
    return true;
}

int UnpackedFile::CalcOffsetInCurrentCache( int offset )
{
    return offset + sizeof(UnpackedFileHeader) - m_CacheSeq*m_pFS->GetBlockDataSize();
}

void UnpackedFile::FlushCache()
{
    if(m_CacheChanged)
    {
        m_pFS->FlushCache(m_Cacheid, m_Cache);
        m_CacheChanged = false;
    }
}

int UnpackedFile::AppendBlock( int begin, int end )
{
    BlockHeader header = {begin, end};
    int i = m_pFS->AllocBlock(header);

    m_pFS->LoadBlockHeader(end, header);
    assert(header.next == begin);
    header.next = i;
    m_pFS->FlushBlockHeader(end, header);

    m_pFS->LoadBlockHeader(begin, header);
    assert(header.prev == end);
    header.prev = i;
    m_pFS->FlushBlockHeader(begin, header);

    return i;
}

void UnpackedFile::FlushHeaderToDisk()
{
    if(m_Cacheid != m_Beginid)
    {
        m_pFS->LoadCache(m_Beginid, m_Cache);
        m_Cacheid = m_Beginid;
        FlushHeaderToCache();
    }
    assert(m_Beginid == m_Cacheid);
    m_pFS->FlushBlockData(m_Cacheid, m_Cache.data);

}
