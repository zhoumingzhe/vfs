#include <crtdbg.h>
#include <assert.h>
#include <windows.h>
#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"
#include "../VFS/BlockFS.h"
#include "../VFS/UnpackedFile.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile::OpenMode mode = IFile::O_Truncate;
    IFile *pFile = OpenDiskFile("test.pkg", mode);
    BlockManager* pMgr = new BlockManager(pFile, mode==IFile::O_Truncate, 1024);
    BlockFS *pFS = new BlockFS(pMgr, mode);

    WIN32_FIND_DATAA finddata;
    HANDLE hfine = FindFirstFileA("input\\*", &finddata);
    if (hfine != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (finddata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            {
                continue;
            }
            printf("%s\n", finddata.cFileName);
            std::string name = "input\\";
            name += finddata.cFileName;
            IFile* pTemp = OpenDiskFile(name.c_str(), IFile::O_ReadOnly);
            int length = pTemp->GetSize();
            char* buff = new char[length];
            pTemp->Read(buff, length);
            pFS->AddUnpackedFile(name.c_str(), buff, length);
            delete[]buff;
            delete pTemp;
        }
        while (FindNextFileA(hfine, &finddata));
    }
    FindClose(hfine);
    delete pFS;
    delete pMgr;
    delete pFile;
}