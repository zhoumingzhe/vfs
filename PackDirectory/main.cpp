#include <crtdbg.h>
#include <windows.h>
#include <algorithm>
#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"
#include "../VFS/BlockFS.h"
#include "../VFS/UnpackedFile.h"

void AddFileRecursively(const std::string dir, BlockFS* pFS)
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
                    AddFileRecursively(dir + "/" + finddata.cFileName, pFS);
                continue;
            }
            std::string name = dir;
            name += "/";
            name += finddata.cFileName;
            IFile* pTemp = OpenDiskFile(name.c_str(), IFile::O_ReadOnly);
            int length = pTemp->GetSize();
            char* buff = new char[length];
            pTemp->Read(buff, length);
            std::transform(name.begin(), name.end(), name.begin(), tolower);
            pFS->AddFile(name.c_str(), buff, length);
            delete[]buff;
            delete pTemp;
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
    IFile::OpenMode mode = IFile::O_Truncate;
    IFile *pFile = OpenDiskFile(argv[1], mode);
    BlockManager* pMgr = new BlockManager(pFile, mode==IFile::O_Truncate, 1024);
    BlockFS *pFS = new BlockFS(pMgr, mode);

    AddFileRecursively(argv[2], pFS);
    delete pFS;
    delete pMgr;
    delete pFile;
    return 0;
}