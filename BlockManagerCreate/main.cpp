#include <crtdbg.h>
#include <assert.h>
#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile *pFile = OpenDiskFile("test.pkg", IFile::O_Truncate);
    BlockManager* pMgr = new BlockManager(pFile, true, 1024);

    assert(pMgr->GetBlocks() == 0);
    assert(pMgr->GetBlockSize() == 1024);
    assert(pMgr->GetUnused() == -1);

    delete pMgr;
    delete pFile;

    pFile = OpenDiskFile("test.pkg", IFile::O_Write);
    pMgr = new BlockManager(pFile, false, 1024);


    assert(pMgr->GetBlocks() == 0);
    assert(pMgr->GetBlockSize() == 1024);
    assert(pMgr->GetUnused() == -1);

    delete pMgr;
    delete pFile;
}