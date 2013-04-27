#include "stdafx.h"
#include <assert.h>
#include "UnpackedFile.h"
#include "BlockFS.h"

UnpackedFile::UnpackedFile(BlockFS* pFS, OpenMode mode, offset_type beginid):
    m_pFS(pFS),
    m_Cacheid(beginid),
    m_Beginid(beginid),
    m_CacheChanged(false)
{
    m_Current.offset = 0;
    m_CacheSeq.offset = 0;
    m_pFS->LoadBlockData(m_Cacheid, m_Cache);

    //for simplicity;
    assert(m_Cache.size() >= sizeof(UnpackedFileHeader));

    memcpy(&m_Header, &m_Cache[0], sizeof(m_Header));

}


UnpackedFile::~UnpackedFile(void)
{
    FlushCache();
    m_pFS->OnFileDestory(this);
}

offset_type UnpackedFile::Read( void* buffer, offset_type size )
{
    offset_type ret;
    assert(size.offset>=0);
    assert(size.offset<0xFFFFFFFFF);
    if(size.offset <= 0)
    {
        ret.offset = 0;
        return ret;
    }

    if(size.offset > GetSize().offset - m_Current.offset)
        size.offset = GetSize().offset - m_Current.offset;

    offset_type offset = CalcOffsetInCache(m_Current, GetCacheSeq());
    assert(offset.offset >= 0);
    offset_type blocksize = m_pFS->GetBlockDataSize();

    //Seek to the right position
    if (offset >= blocksize)
    {
        FlushCache();
        offset_type cacheid = GetCacheid();
        offset_type cacheseq = GetCacheSeq();
        while (offset >= blocksize)
        {
            //we reach the last block,
            //nothing to read
            cacheid = m_pFS->GetNextBlockId(cacheid, m_Beginid);
            if(cacheid.offset == -1)
                break;
            ++cacheseq.offset;
            //need some optimization, just load the block header
            offset = CalcOffsetInCache(m_Current, cacheseq);
        }
        if(cacheid.offset == -1)
        {
            offset_type ret;
            ret.offset = 0;
            return ret;
        }
        SetCacheState(cacheid, cacheseq);
        m_pFS->LoadBlockData(GetCacheid(), m_Cache);

    }
    offset_type size_left = size;
    //read in Loop
    while (size_left.offset > 0)
    {
        //read in cache
        assert(offset < blocksize);
        int size_to_read = (blocksize.offset - offset.offset)<size_left.offset?
            (int)(blocksize.offset - offset.offset):(int)size_left.offset;
        assert(m_Cache.size() >= offset.offset + size_to_read);
        memcpy(buffer, &m_Cache[(size_t)offset.offset], size_to_read);
        //this is necessary because of previous write
        FlushCache();

        //advance data
        m_Current.offset += size_to_read;
        size_left.offset -= size_to_read;

        if(size_left.offset > 0)
        {
            buffer = (char*)buffer + size_to_read;
            //we reach the last block,
            //return the bytes read
            offset_type cacheid = m_pFS->GetNextBlockId(m_Cacheid, m_Beginid);
            if(cacheid.offset == -1)
            {
                offset_type ret;
                ret.offset = size.offset - size_left.offset;
                return ret;
            }
            offset_type next_seq;
            next_seq.offset = GetCacheSeq().offset + 1;
            SetCacheState(cacheid, next_seq);
            m_pFS->LoadBlockData(GetCacheid(), m_Cache);
            offset = CalcOffsetInCache(m_Current, GetCacheSeq());
            assert(offset.offset==0);
        }
    }
    return size;
}

offset_type UnpackedFile::Write( const void* buffer, offset_type size )
{
    assert(size.offset>=0);
    if(size.offset <= 0)
    {
        offset_type ret;
        ret.offset = 0;
        return ret;
    }
    offset_type offset = CalcOffsetInCache(m_Current, GetCacheSeq());
    assert(offset.offset >= 0);
    offset_type blocksize = m_pFS->GetBlockDataSize();

    //Seek to the right position
    if (offset >= blocksize)
    {
        FlushCache();
        while (offset >= blocksize)
        {
            //we reach the last block,
            //allocate blocks at the end
            offset_type cacheid = m_pFS->AppendOrGetNextBlockId(GetCacheid(), m_Beginid);
            offset_type next_seq;
            next_seq.offset = GetCacheSeq().offset + 1;
            SetCacheState(cacheid, next_seq);
            offset = CalcOffsetInCache(m_Current, GetCacheSeq());
        }
        m_pFS->LoadBlockData(GetCacheid(), m_Cache);
    }

    offset_type size_left = size;
    //Write in Loop
    while (size_left.offset > 0)
    {
        //Write in cache
        assert(offset < blocksize);
        offset_type size_to_write;
        size_to_write.offset = (blocksize.offset - offset.offset)<size_left.offset?
            (blocksize.offset - offset.offset):size_left.offset;
        assert(m_Cache.size() >= offset.offset + size_to_write.offset);
        memcpy(&m_Cache[(size_t)offset.offset], buffer, (size_t)size_to_write.offset);
        m_CacheChanged = true;
        FlushCache();

        //advance data
        m_Current.offset += size_to_write.offset;
        size_left.offset -= size_to_write.offset;

        if(size_left.offset > 0)
        {
            buffer = (char*)buffer + size_to_write.offset;
            //we reach the last block,
            //allocate blocks at the end
            offset_type cacheid = m_pFS->AppendOrGetNextBlockId(m_Cacheid, m_Beginid);
            offset_type next_seq;
            next_seq.offset = GetCacheSeq().offset + 1;
            SetCacheState(cacheid, next_seq);
            //if(size_left >= m_pFS->GetBlockDataSize())
                m_pFS->LoadBlockData(GetCacheid(), m_Cache);
            offset = CalcOffsetInCache(m_Current, GetCacheSeq());
            assert(offset.offset==0);
        }
    }

    if(m_Current.offset > GetDataSize())
    {
        SetDataSize((int)m_Current.offset);
        FlushHeaderToDisk();
    }

    return size;
}

offset_type UnpackedFile::Seek( offset_type pos, enum SeekMode mode )
{
    offset_type old = m_Current;
    switch (mode)
    {
    case IFile::S_Current:
        m_Current.offset += pos.offset;
        break;
    case IFile::S_End:
        m_Current.offset = GetDataSize() - pos.offset;
        break;
    case IFile::S_Begin:
        m_Current.offset = pos.offset;
        break;
    default:
        assert(0&&"Wrong parameter");
        break;
    }
    offset_type z;
    z.offset = 0;
    SetCacheState(m_Beginid, z);
    m_pFS->LoadBlockData(m_Beginid, m_Cache);
    return old;
}

offset_type UnpackedFile::GetSize()
{
    offset_type ret;
    ret.offset = GetDataSize();
    return ret;
}

void UnpackedFile::ReserveSpace( offset_type size )
{
    assert(0&&"not implemented");
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
        assert(m_Cache.size() >= sizeof(m_Header));
        memcpy(&m_Cache[0], &m_Header, sizeof(m_Header));
        m_CacheChanged = true;
    }
}

offset_type UnpackedFile::CalcOffsetInCache( offset_type offset, offset_type seq)
{
    offset_type ret;
    ret.offset = offset.offset + sizeof(UnpackedFileHeader) - seq.offset*m_pFS->GetBlockDataSize().offset;
    return ret;
}

void UnpackedFile::FlushCache()
{
    if(m_CacheChanged)
    {
        m_pFS->FlushBlockData(GetCacheid(), m_Cache);
        m_CacheChanged = false;
    }
}


void UnpackedFile::FlushHeaderToDisk()
{
    //todo: not necessarily change the cache state
    if(GetCacheid() != m_Beginid)
    {
        m_pFS->LoadBlockData(m_Beginid, m_Cache);
        offset_type z;
        z.offset = 0;
        SetCacheState(m_Beginid, z);
        FlushHeaderToCache();
    }
    assert(m_Beginid == GetCacheid());
    m_pFS->FlushBlockData(GetCacheid(), m_Cache);

}

void UnpackedFile::SetCacheState( offset_type id, offset_type seq )
{
    m_Cacheid = id;
    m_CacheSeq = seq;
}
