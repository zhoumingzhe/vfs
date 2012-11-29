#include <crtdbg.h>
#include <assert.h>
#include <time.h>
#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"
#include "../VFS/BlockFS.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile::OpenMode mode = IFile::O_Truncate;
    IFile *pFile = OpenDiskFile("test.pkg", mode);
    BlockManager* pMgr = new BlockManager(pFile, mode==IFile::O_Truncate, 1024);
    BlockFS *pFS = new BlockFS(pMgr, mode);

    pFS->AddUnpackedFile("test123", "test123", sizeof("test123"));
    pFS->AddUnpackedFile("test1234", "test123", sizeof("test123"));
    delete pFS;
    delete pMgr;
    delete pFile;
}