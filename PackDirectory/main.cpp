#include <crtdbg.h>
#include <windows.h>
#include <algorithm>
#include <assert.h>
#include <Shlwapi.h>

#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"
#include "../VFS/BlockFS.h"
#include "../VFS/UnpackedFile.h"
#include "../VFS/GUtMd5.h"

void ScanFileRecursively(const std::string dir, BlockFS* pFS, std::map<std::string, MD5Index>& mapEntries,
                         std::vector<std::string>& add)
{
    WIN32_FIND_DATAA finddata;
    std::string searchstring = dir + "/*";
    HANDLE hfine = FindFirstFileA(searchstring.c_str(), &finddata);
    if (hfine != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (finddata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            {
                if(strcmp(finddata.cFileName, ".svn")!=0&&
                    strcmp(finddata.cFileName, ".")!=0&&
                    strcmp(finddata.cFileName, "..")!=0)
                    ScanFileRecursively(dir + "/" + finddata.cFileName, pFS, mapEntries, add);
                continue;
            }
            std::string name = dir;
            name += "/";
            name += finddata.cFileName;
            std::transform(name.begin(), name.end(), name.begin(), tolower);
            std::map<std::string, MD5Index>::iterator it = mapEntries.find(name);
            if(it != mapEntries.end())
            {
                IFile* pTemp = OpenDiskFile(name.c_str(), IFile::O_ReadOnly);
                offset_type length = pTemp->GetSize();
                if(it->second.size==length)
                {
                    char* buff = new char[(size_t)length.offset];
                    pTemp->Read(buff, length);
                    std::transform(name.begin(), name.end(), name.begin(), tolower);
                    printf("checking %s, %d bytes\n", name.c_str(), length);

                    unsigned char ucCheckSum[16];
                    GetMD5CheckSum((unsigned char*)buff, (unsigned)length.offset, ucCheckSum);
                    if(!memcmp(ucCheckSum, it->second.md5, sizeof(ucCheckSum)))
                    {
                        printf("file %s, ignored update\n", it->first.c_str());
                        mapEntries.erase(it);
                    }
                    else
                    {
                        printf("file %s, need update\n", it->first.c_str());
                        add.push_back(it->first);
                    }
                    //pFS->AddFile(name.c_str(), buff, length, length > pFS->GetBlockDataSize());
                    delete[]buff;
                }
                else
                {
                    printf("file %s, with different size %d, original %d", name.c_str(), length, it->second.size);
                    add.push_back(name);
                }
                delete pTemp;
            }
            else
            {
                printf("file %s will be added\n",name.c_str());
                add.push_back(name);
            }

        }
        while (FindNextFileA(hfine, &finddata));
    }
    FindClose(hfine);
}

int main(int argc, char* argv[])
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    if(argc < 3)
    {
        printf("usage: %s directory package", argv[0]);
        return 0;
    }
    IFile::OpenMode mode = IFile::O_Write;
    if(!PathFileExistsA(argv[1]))
        mode = IFile::O_Truncate;
    IFile *pFile = OpenDiskFile(argv[1], mode);
    BlockManager* pMgr = new BlockManager(pFile, mode==IFile::O_Truncate, offset_type(1024));
    BlockFS *pFS = new BlockFS(pMgr, mode);
    std::vector<BlockFileEntry> vecEntries = pFS->GetEntries();
    std::map<std::string, MD5Index> mapEntries;
    for(std::vector<BlockFileEntry>::iterator it = vecEntries.begin();
        it != vecEntries.end(); ++it)
    {
        if(std::string("")!=it->name)
        {
            assert(mapEntries.find(it->name)==mapEntries.end());
            mapEntries[it->name] = it->index;
        }
    }
    std::vector<std::string> add;
    ScanFileRecursively(argv[2], pFS, mapEntries, add);

    for(std::map<std::string, MD5Index>::iterator it = mapEntries.begin(); it != mapEntries.end(); ++it)
        pFS->RemoveFile(it->first.c_str());

    for(std::vector<std::string>::iterator it = add.begin(); it != add.end(); ++it)
    {
        IFile* pTemp = OpenDiskFile(it->c_str(), IFile::O_ReadOnly);
        offset_type length = pTemp->GetSize();
        char* buff = new char[(size_t)length.offset];
        pTemp->Read(buff, length);
        printf("adding %s, %d bytes\n", it->c_str(), length);
        pFS->AddFile(it->c_str(), buff, length, length > pFS->GetBlockDataSize());
        delete[]buff;
        delete pTemp;
    }
    delete pFS;
    delete pMgr;
    delete pFile;
    return 0;
}