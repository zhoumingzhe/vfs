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
            int cacheid;
            if(m_Cache.header.next == m_Beginid)
            {
                return 0;
            }
            else
            {
                cacheid = m_Cache.header.next;
            }
            SetCacheState(cacheid, GetCacheSeq()+1);
            //need some optimization, just load the block header
            m_pFS->LoadCache(GetCacheid(), m_Cache);
            offset = CalcOffsetInCurrentCache(m_Current);
        }
    }
    int size_left = size;
    //read in Loop
    while (size_left > 0)
    {
        //read in cache
        assert(offset < blocksize);
        int size_to_read = (blocksize - offset)<size_left?
            (blocksize - offset):size_left;
        assert((int)m_Cache.data.size() >= offset + size_to_read);
        memcpy(buffer, &m_Cache.data[offset], size_to_read);
        //this is necessary because of previous write
        FlushCache();

        //advance data
        m_Current += size_to_read;
        size_left -= size_to_read;

        if(size_left > 0)
        {
            buffer = (char*)buffer + size_to_read;
            //we reach the last block,
            //return the read bytes
            int cacheid;
            if(m_Cache.header.next == m_Beginid)
            {
                return size-size_left;
            }
            else
            {
                cacheid = m_Cache.header.next;
            }
            SetCacheState(cacheid, GetCacheSeq()+1);
            m_pFS->LoadCache(GetCacheid(), m_Cache);
            offset = CalcOffsetInCurrentCache(m_Current);
            assert(offset==0);
        }
    }
    return size;
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
            int cacheid;
            if(m_Cache.header.next == m_Beginid)
            {
                cacheid = AppendBlock(m_Beginid, GetCacheid());
            }
            else
            {
                cacheid = m_Cache.header.next;
            }

            SetCacheState(cacheid, GetCacheSeq()+1);
            //need some optimization, just load the block header
            m_pFS->LoadCache(GetCacheid(), m_Cache);
            offset = CalcOffsetInCurrentCache(m_Current);
        }
    }

    int size_left = size;
    //Write in Loop
    while (size_left > 0)
    {
        //Write in cache
        assert(offset < blocksize);
        int size_to_write = (blocksize - offset)<size_left?
            (blocksize - offset):size_left;
        assert((int)m_Cache.data.size() >= offset + size_to_write);
        memcpy(&m_Cache.data[offset], buffer, size_to_write);
        m_CacheChanged = true;
        FlushCache();

        //advance data
        m_Current += size_to_write;
        size_left -= size_to_write;

        if(size_left > 0)
        {
            buffer = (char*)buffer + size_to_write;
            //we reach the last block,
            //allocate blocks at the end
            int cacheid;
            if(m_Cache.header.next == m_Beginid)
            {
                cacheid = AppendBlock(m_Beginid, GetCacheid());
            }
            else
            {
                cacheid = m_Cache.header.next;
            }
            SetCacheState(cacheid, GetCacheSeq()+1);
            m_pFS->LoadCache(GetCacheid(), m_Cache);
            offset = CalcOffsetInCurrentCache(m_Current);
            assert(offset==0);
        }
    }

    if(m_Current > GetDataSize())
    {
        SetDataSize(m_Current);
        FlushHeaderToDisk();
    }

    return size;
}

int UnpackedFile::Seek( int pos, enum SeekMode mode )
{

    int old = m_Current;
    switch (mode)
    {
    case IFile::S_Current:
        m_Current += pos;
        break;
    case IFile::S_End:
        m_Current = GetDataSize() - pos;
        break;
    case IFile::S_Begin:
        m_Current = pos;
        break;
    default:
        assert(0&&"Wrong parameter");
        break;
    }
    SetCacheState(0, 0);
    m_pFS->LoadCache(GetCacheid(), m_Cache);
    return old;
}

int UnpackedFile::GetSize()
{
    return GetDataSize();
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
    if(GetCacheid() == m_Beginid) //flush the header to cache
    {
        assert(m_Cache.data.size() >= sizeof(m_Header));
        memcpy(&m_Cache.data[0], &m_Header, sizeof(m_Header));
        m_CacheChanged = true;
    }
}

int UnpackedFile::CalcOffsetInCurrentCache( int offset )
{
    return offset + sizeof(UnpackedFileHeader) - GetCacheSeq()*m_pFS->GetBlockDataSize();
}

void UnpackedFile::FlushCache()
{
    if(m_CacheChanged)
    {
        m_pFS->FlushCache(GetCacheid(), m_Cache);
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
    //todo: not necessarily change the cache state
    if(GetCacheid() != m_Beginid)
    {
        m_pFS->LoadCache(m_Beginid, m_Cache);
        SetCacheState(m_Beginid, 0);
        FlushHeaderToCache();
    }
    assert(m_Beginid == GetCacheid());
    m_pFS->FlushBlockData(GetCacheid(), m_Cache.data);

}

void UnpackedFile::SetCacheState( int id, int seq )
{
    m_Cacheid = id;
    m_CacheSeq = seq;
}
