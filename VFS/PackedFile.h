#pragma once
#include "IFile.h"
#include "../zlib/zlib.h"
#include <vector>
class UnpackedFile;
class PackedFile: public IFile
{
public:
    PackedFile(UnpackedFile* pFile, int size);
    virtual ~PackedFile(void);


    virtual int Read(void* buffer, int size);
    virtual int Write(const void* buffer, int size);
    virtual int Seek(int pos, enum SeekMode mode);
    virtual int GetSize();
    virtual int ReserveSpace(int size);
private:
    int m_Size;
    UnpackedFile* m_pFile;
    z_stream m_Stream;

    std::vector<unsigned char> m_Buffer;
};
