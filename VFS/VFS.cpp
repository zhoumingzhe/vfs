#include "stdafx.h"
#include <algorithm>
#include <assert.h>
#include "VFS.h"
#include "IFile.h"
#include "UnpackedFile.h"
#include "BlockManager.h"
#include "BlockFS.h"

struct VFSData
{
    IFile *pFile;
    BlockManager *pMgr;
    BlockFS* pFS;
    VFSData():pFile(0), pMgr(0), pFS(0)
    {
    }
    ~VFSData()
    {
        delete pFS;
        delete pMgr;
        delete pFile;
    }
};

VFSData& GetVfsData()
{
    static VFSData data;
    return data;
}

void VFS::SetPackage( const char* name )
{
    GetVfsData().pFile = OpenDiskFile(name, IFile::O_ReadOnly);
    GetVfsData().pMgr = new BlockManager(GetVfsData().pFile, IFile::O_ReadOnly, offset_type(1024));
    GetVfsData().pFS = new BlockFS(GetVfsData().pMgr, IFile::O_ReadOnly);
}

static std::string GetPathInPackage(std::string path)
{
    std::string result = path;
    std::transform(result.begin(), result.end(), result.begin(), tolower);

    for(std::string::iterator it = result.begin();
        it != result.end(); ++it)
    {
        if(*it == '\\')
            *it = '/';
    }

    int index = result.find("../");
    if(index != std::string::npos)
    {
        assert(index == 0);
        result.replace(index,3,"");
    }
    for(index = result.find("//"); index != std::string::npos; index = result.find("//"))
    {
        result.replace(index, 2, "/");
    }

    return result;
}

IFile* VFS::Open( const char* name )
{
    std::string name_inpackage  = GetPathInPackage(name);

    BlockFS *pFs = GetVfsData().pFS;
    if(pFs)
    {
        return pFs->OpenFileInPackage(name_inpackage.c_str());
    }
    else
    {
        return OpenDiskFile(name, IFile::O_ReadOnly);
    }
}
