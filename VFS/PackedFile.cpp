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
    if(size < (int)m_Buffer.size())
    {
        memcpy(buffer, &m_Buffer[0], size);
        for(int i = 0; i<(int)m_Buffer.size() - size; ++i)
        {
            m_Buffer[i] = m_Buffer[i+size];
        }
        m_Buffer.resize(m_Buffer.size() - size);
        return size;
    }
    if(m_Buffer.size() > 0)
        memcpy(buffer, &m_Buffer[0], m_Buffer.size());

    int size_left = size - m_Buffer.size();
    buffer =(char*)buffer + m_Buffer.size();

    m_Buffer.resize(0);

    std::vector<unsigned char> data;
    data.resize(4*1024);
    int result;
    m_Stream.avail_out = size_left;
    m_Stream.next_out = (unsigned char*)buffer;
    do
    {
        m_Stream.avail_in = m_pFile->Read(&data[0], data.size());
        m_Stream.next_in = &data[0];
        result = inflate(&m_Stream, Z_NO_FLUSH);
    }
    while(result != Z_STREAM_END && m_Stream.avail_out>0);

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
    return m_Size;
}
int PackedFile::ReserveSpace(int size)
{
    assert(0&&"not implemented");
    return 0;
}