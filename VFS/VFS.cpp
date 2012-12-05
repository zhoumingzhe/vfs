#include "stdafx.h"
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
    GetVfsData().pMgr = new BlockManager(GetVfsData().pFile, IFile::O_ReadOnly, 1024);
    GetVfsData().pFS = new BlockFS(GetVfsData().pMgr, IFile::O_ReadOnly);
}

IFile* VFS::Open( const char* name )
{
    BlockFS *pFs = GetVfsData().pFS;
    if(pFs)
    {
        return pFs->OpenUnpackedFile(name);
    }
    else
    {
        return OpenDiskFile(name, IFile::O_ReadOnly);
    }
}
