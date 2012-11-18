#pragma once
#include "IFile.h"

class PackedFile :
    public IFile
{
public:
    PackedFile(void);
    virtual ~PackedFile(void);
};
