#include <crtdbg.h>
#include <assert.h>
#include <time.h>
#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"
#include "../VFS/BlockFS.h"
#include "../VFS/UnpackedFile.h"

char data[1024*1024]="\0";
char data1[1024]="\0";
int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile::OpenMode mode = IFile::O_Truncate;
    IFile *pFile = OpenDiskFile("test.pkg", mode);
    BlockManager* pMgr = new BlockManager(pFile, mode==IFile::O_Truncate, offset_type(1024));
    BlockFS *pFS = new BlockFS(pMgr, mode);

    for(int i = 0; i< 4*1024; ++i)
    {
        char buff[30];
        sprintf_s(buff, "%d", i);
        memset(data1, 0, sizeof(data1));
        sprintf_s(data1, "%d", i);
        pFS->AddFile(buff, data, offset_type(sizeof(data)+sizeof(data1)));
    }
    delete pFS;
    delete pMgr;
    delete pFile;
}