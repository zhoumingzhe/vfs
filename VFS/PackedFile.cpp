#include "stdafx.h"
#include <assert.h>
#include "PackedFile.h"
#include "UnpackedFile.h"

PackedFile::PackedFile(UnpackedFile* pFile, int size):
    m_pFile(pFile),m_Size(size),m_Buffer(4*1024)
{
    m_Stream.zalloc = Z_NULL;
    m_Stream.zfree = Z_NULL;
    m_Stream.opaque = Z_NULL;
    m_Stream.avail_in = 0;
    m_Stream.next_in = Z_NULL;
    m_Stream.avail_out = 0;
    m_Stream.next_out = 0;
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
    if(size<=0)
        return 0;

    m_Stream.avail_out = size;
    m_Stream.next_out = (unsigned char*)buffer;

    while(m_Stream.avail_out>0)
    {
        if(m_Stream.avail_in<=0)
        {
            m_Stream.avail_in = m_pFile->Read(&m_Buffer[0], m_Buffer.size());
            m_Stream.next_in = &m_Buffer[0];
        }
        if(m_Stream.avail_in<=0)
            break;
        int result = inflate(&m_Stream, Z_NO_FLUSH);
        if(result != Z_OK)
            break;
    }

    return m_Stream.next_out - (unsigned char*)buffer;
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
    return m_Size;
}
int PackedFile::ReserveSpace(int size)
{
    assert(0&&"not implemented");
    return 0;
}