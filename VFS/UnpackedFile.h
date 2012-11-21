#pragma once
#include "IFile.h"
#include "BlockFS.h"
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
    bool LoadCache();
    BlockFS* m_pFS;
    int m_Current;
    BlockCache m_Cache;
    int m_Blockid;
    int m_Beginid;
};

