#include <Windows.h>
#include <assert.h>
#include "DiskFile.h"

IFile* OpenDiskFile(const char* name, IFile::OpenMode mode)
{
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

int DiskFile::Read( void* buffer, int size )
{
    DWORD ret;
    BOOL result = ReadFile(m_hFile, buffer, size, &ret, NULL);
    assert(result);
    return (int)ret;
}

int DiskFile::Write( const void* buffer, int size )
{
    assert(m_eMode != IFile::O_ReadOnly);
    DWORD ret;
    BOOL result = WriteFile(m_hFile, buffer, size, &ret, NULL);
    assert(result);
    //FlushFileBuffers(m_hFile);
    return ret;
}

int DiskFile::Seek( int pos, enum SeekMode mode )
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
    DWORD result = SetFilePointer(m_hFile, pos, NULL, api_mode);
    assert(result != INVALID_SET_FILE_POINTER || NO_ERROR == GetLastError());
    return result;
}

int DiskFile::GetSize()
{
    return GetFileSize(m_hFile, NULL);
}

int DiskFile::ReserveSpace( int size )
{
    assert(m_eMode != O_ReadOnly);
    Seek(size, S_Begin);
    DWORD result = SetEndOfFile(m_hFile);
    assert(result);
    return 0;
}
