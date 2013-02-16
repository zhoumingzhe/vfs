#include "stdafx.h"
#include <Windows.h>
#include <Shlwapi.h>
#include <assert.h>
#include "DiskFile.h"

IFile* OpenDiskFile(const char* name, IFile::OpenMode mode)
{
    if(!PathFileExistsA(name) && mode != IFile::O_Truncate)
        return 0;
    else
        return new DiskFile(name, mode);
}
DiskFile::DiskFile(const char* name, OpenMode mode):m_eMode(mode)
{
    DWORD access = GENERIC_READ;
    DWORD create = OPEN_EXISTING;
    switch(mode)
    {
    case O_ReadOnly:
        access = GENERIC_READ;
        create = OPEN_EXISTING;
        break;
    case O_Write:
        access = GENERIC_READ|GENERIC_WRITE;
        create = OPEN_EXISTING;
        break;
    case O_Truncate:
        access = GENERIC_READ|GENERIC_WRITE;
        create = CREATE_ALWAYS;
        break;
    default:
        assert(0);
    }
    m_hFile = CreateFileA(name, access, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, create,
        FILE_ATTRIBUTE_NORMAL, NULL);

    if(m_hFile == INVALID_HANDLE_VALUE)
        assert(0);


}

DiskFile::~DiskFile(void)
{
    CloseHandle(m_hFile);
}

offset_type DiskFile::Read( void* buffer, offset_type size )
{
    offset_type ret;
    DWORD bytes_read = 0;
    assert(size.offset < 0xffffffff);
    BOOL result = ReadFile(m_hFile, buffer, (DWORD)size.offset, &bytes_read, NULL);
    assert(result);
    ret.offset = bytes_read;
    return ret;
}

offset_type DiskFile::Write( const void* buffer, offset_type size )
{
    assert(m_eMode != IFile::O_ReadOnly);
    assert(size.offset<0xffffffff);
    offset_type ret;
    DWORD bytes_write = 0;
    BOOL result = WriteFile(m_hFile, buffer, (DWORD)size.offset, &bytes_write, NULL);
    assert(result);
    //FlushFileBuffers(m_hFile);
    ret.offset = bytes_write;
    return ret;
}

offset_type DiskFile::Seek( offset_type pos, enum SeekMode mode )
{
    int api_mode = FILE_BEGIN;
    switch(mode)
    {
    case S_Begin:
        api_mode = FILE_BEGIN;
        break;
    case S_Current:
        api_mode = FILE_CURRENT;
        break;
    case S_End:
        api_mode = FILE_END;
        break;
    default:
        assert(0);
        break;
    }
    long distancelow = pos.low;
    long distancehigh = pos.high;
    offset_type ret;
    DWORD result = SetFilePointer(m_hFile, distancelow, &distancehigh, api_mode);
    assert(result != INVALID_SET_FILE_POINTER || NO_ERROR == GetLastError());
    ret.low = result;
    ret.high = distancehigh;
    return ret;
}

offset_type DiskFile::GetSize()
{
    offset_type ret;

    ret.low = GetFileSize(m_hFile, (LPDWORD)&ret.high);
    return ret;
}

void DiskFile::ReserveSpace( offset_type size )
{
    assert(m_eMode != O_ReadOnly);
    Seek(size, S_Begin);
    DWORD result = SetEndOfFile(m_hFile);
    assert(result);
}
