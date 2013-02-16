#include "stdafx.h"
#include <assert.h>
#include "PackedFile.h"
#include "UnpackedFile.h"

PackedFile::PackedFile(UnpackedFile* pFile, offset_type size):
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

offset_type PackedFile::Read(void* buffer, offset_type size)
{
    if(size.offset<=0)
    {
        offset_type z;
        z.offset = 0;
        return z;
    }
    m_Stream.avail_out = (unsigned int)size.offset;
    m_Stream.next_out = (unsigned char*)buffer;

    while(m_Stream.avail_out>0)
    {
        if(m_Stream.avail_in<=0)
        {
            offset_type sizetoread;
            sizetoread.offset = m_Buffer.size();
            m_Stream.avail_in = (unsigned int)m_pFile->Read(&m_Buffer[0], sizetoread).offset;
            m_Stream.next_in = &m_Buffer[0];
        }
        if(m_Stream.avail_in<=0)
            break;
        int result = inflate(&m_Stream, Z_NO_FLUSH);
        if(result != Z_OK)
            break;
    }

    offset_type sizeread;
    sizeread.offset = m_Stream.next_out - (unsigned char*)buffer;
    return sizeread;
}
offset_type PackedFile::Write(const void* buffer, offset_type size)
{
    assert(0&&"not implemented");
    offset_type z;
    z.offset = 0;
    return z;
}
offset_type PackedFile::Seek(offset_type pos, enum SeekMode mode)
{
    assert(0&&"not implemented");
    offset_type z;
    z.offset = 0;
    return z;
}
offset_type PackedFile::GetSize()
{
    return m_Size;
}
void PackedFile::ReserveSpace(offset_type size)
{
    assert(0&&"not implemented");
}