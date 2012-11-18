#pragma once
#include "IFile.h"
class UnpackedFile :
    public IFile
{
public:
    UnpackedFile(void);
    ~UnpackedFile(void);

    virtual int Read(void* buffer, int size);
    virtual int Write(const void* buffer, int size);
    virtual int Seek(int pos, enum SeekMode mode);
    virtual int GetSize();
    virtual int ReserveSpace(int size);


};

