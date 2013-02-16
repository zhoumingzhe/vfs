#pragma once
#include "IFile.h"
class DiskFile :
    public IFile
{
public:
    DiskFile(const char* name, OpenMode mode);
    virtual ~DiskFile(void);

    offset_type Read(void* buffer, offset_type size);
    offset_type Write(const void* buffer, offset_type size);
    offset_type Seek(offset_type pos, enum SeekMode mode);
    offset_type GetSize();
    void ReserveSpace(offset_type size);
private:
    DiskFile(const DiskFile&);
    const DiskFile& operator =(const DiskFile&);
    HANDLE m_hFile;
    OpenMode m_eMode;
};
