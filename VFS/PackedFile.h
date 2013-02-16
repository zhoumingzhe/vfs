#pragma once
#include "IFile.h"
#include "../zlib/zlib.h"
#include <vector>
class UnpackedFile;
class PackedFile: public IFile
{
public:
    PackedFile(UnpackedFile* pFile, offset_type size);
    virtual ~PackedFile(void);


    virtual offset_type Read(void* buffer, offset_type size);
    virtual offset_type Write(const void* buffer, offset_type size);
    virtual offset_type Seek(offset_type pos, enum SeekMode mode);
    virtual offset_type GetSize();
    virtual void ReserveSpace(offset_type size);
private:
    offset_type m_Size;
    UnpackedFile* m_pFile;
    z_stream m_Stream;

    std::vector<unsigned char> m_Buffer;
};
