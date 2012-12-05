#pragma once
class IFile;
class VFS
{
public:
    static void SetPackage(const char* name);
    static IFile* Open(const char* name);
};

