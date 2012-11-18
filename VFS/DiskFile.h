#pragma once
#include "IFile.h"
class DiskFile :
    public IFile
{
public:
    DiskFile(const char* name, OpenMode mode);
    virtual ~DiskFile(void);

    int Read(void* buffer, int size);
    int Write(const void* buffer, int size);
    int Seek(int pos, enum SeekMode mode);
    int GetSize();
    int ReserveSpace(int size);
private:
    DiskFile(const DiskFile&);
    const DiskFile& operator =(const DiskFile&);
    HANDLE m_hFile;
    OpenMode m_eMode;
};
