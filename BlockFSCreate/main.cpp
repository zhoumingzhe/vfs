#include <crtdbg.h>
#include <assert.h>
#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"
#include "../VFS/BlockFS.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile *pFile = OpenDiskFile("test.pkg", IFile::O_Truncate);
    BlockManager* pMgr = new BlockManager(pFile, true, 1024);
    BlockFS *pFS = new BlockFS(pMgr, IFile::O_Truncate);

    char* buffer = new char[4096];
    for(int i = 0; i<4096; ++i)
        buffer[i] = i%10;
    pFS->First()->Write(buffer, 4096);
    delete[]buffer;

    delete pFS;
    delete pMgr;
    delete pFile;
}