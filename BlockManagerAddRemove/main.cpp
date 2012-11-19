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

    for(int i = 0; i< 10; ++i)
    {
        int id = pMgr->AllocBlock();
        assert(id == i);
    }

    pMgr->RecycleBlock(7);
    pMgr->RecycleBlock(8);
    pMgr->RecycleBlock(9);

    pMgr->RecycleBlock(4);
    pMgr->RecycleBlock(5);
    int j = pMgr->AllocBlock();
    pMgr->RecycleBlock(6);
    pMgr->RecycleBlock(j);

    pMgr->RecycleBlock(2);
    pMgr->RecycleBlock(3);
    pMgr->RecycleBlock(1);
    pMgr->RecycleBlock(0);

    assert(pMgr->GetBlocks() == 0);
    assert(pMgr->GetUnused() == -1);

    delete pMgr;
    delete pFile;
}