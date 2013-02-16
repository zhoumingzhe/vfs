#pragma once
#include <vector>
#include "IFile.h"

class BlockFS;
struct UnpackedFileHeader
{
    int refcount;
    int datasize;
};

class UnpackedFile :
    public IFile
{
public:
    UnpackedFile(BlockFS* pFS, OpenMode mode, offset_type beginid);
    ~UnpackedFile(void);

    virtual offset_type Read(void* buffer, offset_type size);
    virtual offset_type Write(const void* buffer, offset_type size);
    virtual offset_type Seek(offset_type pos, enum SeekMode mode);
    virtual offset_type GetSize();
    virtual void ReserveSpace(offset_type size);

    int GetRefCount();
    void SetRefCount(int refcount);

    int GetDataSize();
    void SetDataSize(int datasize);

    void FlushCache();

    void FlushHeaderToDisk();
private:


    offset_type CalcOffsetInCache(offset_type offset, offset_type seq);
    void FlushHeaderToCache();

    void SetCacheState(offset_type seq, offset_type id);
    offset_type GetCacheid(){return m_Cacheid;}
    offset_type GetCacheSeq(){return m_CacheSeq;}


    BlockFS* m_pFS;

    const offset_type m_Beginid; //begin block id

    UnpackedFileHeader m_Header; //Header in the first block

    offset_type m_Current; //current offset in the file

    bool m_CacheChanged; //if this is true, cache have to be write back
    offset_type m_CacheSeq; //the sequence of the current cache
    offset_type m_Cacheid;      //cached block id
    std::vector<char> m_Cache; //cached block
};

