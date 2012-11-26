#pragma once
#include "IFile.h"
#include "BlockFS.h"

struct UnpackedFileHeader
{
    int refcount;
    int datasize;
};

class UnpackedFile :
    public IFile
{
public:
    UnpackedFile(BlockFS* pFS, OpenMode mode, int beginid);
    ~UnpackedFile(void);

    virtual int Read(void* buffer, int size);
    virtual int Write(const void* buffer, int size);
    virtual int Seek(int pos, enum SeekMode mode);
    virtual int GetSize();
    virtual int ReserveSpace(int size);

private:

    int GetRefCount();
    void SetRefCount(int refcount);

    int GetDataSize();
    void SetDataSize(int datasize);

    int CalcOffsetInCurrentCache(int offset);
    void FlushHeaderToCache();
    bool AdvancedToNextCache();

    void FlushCache();
    int AppendBlock(int begin, int end);
    BlockFS* m_pFS;

    const int m_Beginid; //begin block id

    UnpackedFileHeader m_Header; //Header in the first block

    int m_Current; //current offset in the file

    bool m_CacheChanged; //if this is true, cache have to be write back
    int m_CacheSeq; //the sequence of the current cache
    int m_Cacheid;      //cached block id
    BlockCache m_Cache; //cached block
};

