#pragma once

class IFile
{
public:

    enum OpenMode
    {
        O_ReadOnly,
        O_Write,
        O_Truncate
    };

    enum SeekMode
    {
        S_Current,
        S_End,
        S_Begin
    };

    virtual ~IFile(void);
    virtual int Read(void* buffer, int size) = 0;
    virtual int Write(const void* buffer, int size) = 0;
    virtual int Seek(int pos, enum SeekMode mode) = 0;
    virtual int GetSize() = 0;
    virtual int ReserveSpace(int size) = 0;


};


IFile* OpenDiskFile(const char* name, IFile::OpenMode mode);