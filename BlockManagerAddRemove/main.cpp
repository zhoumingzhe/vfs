#include <crtdbg.h>
#include <assert.h>
#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

    IFile *pFile = OpenDiskFile("test.pkg", IFile::O_Truncate);
    BlockManager* pMgr = new BlockManager(pFile, true, offset_type(1024));

    assert(pMgr->GetBlocks() == offset_type(0));
    assert(pMgr->GetBlockSize() == offset_type(1024));
    assert(pMgr->GetUnused() == offset_type(-1));

    for(int i = 0; i< 10; ++i)
    {
        offset_type id = pMgr->AllocBlock();
        assert(id.offset == i);
    }

    pMgr->RecycleBlock(offset_type(7));
    pMgr->RecycleBlock(offset_type(8));
    pMgr->RecycleBlock(offset_type(9));

    pMgr->RecycleBlock(offset_type(4));
    pMgr->RecycleBlock(offset_type(5));
    offset_type j = pMgr->AllocBlock();
    pMgr->RecycleBlock(offset_type(6));
    pMgr->RecycleBlock((j));

    pMgr->RecycleBlock(offset_type(2));
    pMgr->RecycleBlock(offset_type(3));
    pMgr->RecycleBlock(offset_type(1));
    pMgr->RecycleBlock(offset_type(0));

    assert(pMgr->GetBlocks() == offset_type(0));
    assert(pMgr->GetUnused() == offset_type(-1));

    delete pMgr;
    delete pFile;
}