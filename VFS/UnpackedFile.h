#pragma once
#include "IFile.h"
class BlockFS;
class UnpackedFile :
    public IFile
{
public:
    UnpackedFile(BlockFS* pFS, OpenMode mode);
    ~UnpackedFile(void);

    virtual int Read(void* buffer, int size);
    virtual int Write(const void* buffer, int size);
    virtual int Seek(int pos, enum SeekMode mode);
    virtual int GetSize();
    virtual int ReserveSpace(int size);

private:
    BlockFS* m_pFS;
};

