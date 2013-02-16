#pragma once

union offset_type
{
    long long offset;
    struct
    {
        long low;
        long high;
    };
    explicit offset_type(long long o)
        :offset(o)
    {}
    offset_type (const offset_type & o)
        :offset(o.offset)
    {}
    offset_type():
        offset(0)
    {}
    bool operator==(const offset_type & rhs) const
    {
        return offset == rhs.offset;
    }

    bool operator!=(const offset_type & rhs) const
    {
        return offset != rhs.offset;
    }

    bool operator<(const offset_type & rhs) const
    {
        return offset < rhs.offset;
    }

    bool operator<=(const offset_type & rhs) const
    {
        return offset <= rhs.offset;
    }

    bool operator>(const offset_type & rhs) const
    {
        return offset > rhs.offset;
    }

    bool operator>=(const offset_type & rhs) const
    {
        return offset >= rhs.offset;
    }
    offset_type operator+(const offset_type & rhs) const
    {
        return offset_type(offset + rhs.offset);
    }
};

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
    virtual offset_type Read(void* buffer, offset_type size) = 0;
    virtual offset_type Write(const void* buffer, offset_type size) = 0;
    virtual offset_type Seek(offset_type pos, enum SeekMode mode) = 0;
    virtual offset_type GetSize() = 0;
    virtual void ReserveSpace(offset_type size) = 0;


};


IFile* OpenDiskFile(const char* name, IFile::OpenMode mode);