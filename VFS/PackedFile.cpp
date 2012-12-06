#include "stdafx.h"
#include <assert.h>
#include "PackedFile.h"
#include "UnpackedFile.h"

PackedFile::PackedFile(UnpackedFile* pFile, int size):
    m_pFile(pFile),m_Size(size)
{
    m_Stream.zalloc = Z_NULL;
    m_Stream.zfree = Z_NULL;
    m_Stream.opaque = Z_NULL;
    m_Stream.avail_in = 0;
    m_Stream.next_in = Z_NULL;
    int ret = inflateInit(&m_Stream);
    assert(ret == Z_OK);
}

PackedFile::~PackedFile(void)
{
    inflateEnd(&m_Stream);
    delete m_pFile;
}
int PackedFile::Read(void* buffer, int size)
{
    return 0;
}
int PackedFile::Write(const void* buffer, int size)
{
    assert(0&&"not implemented");
    return 0;
}
int PackedFile::Seek(int pos, enum SeekMode mode)
{
    assert(0&&"not implemented");
    return 0;
}
int PackedFile::GetSize()
{
    assert(0&&"not implemented");
    return 0;
}
int PackedFile::ReserveSpace(int size)
{
    assert(0&&"not implemented");
    return 0;
}