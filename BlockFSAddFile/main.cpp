#include <crtdbg.h>
#include <assert.h>
#include <time.h>
#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"
#include "../VFS/BlockFS.h"
#include "../VFS/UnpackedFile.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile::OpenMode mode = IFile::O_Truncate;
    IFile *pFile = OpenDiskFile("test.pkg", mode);
    BlockManager* pMgr = new BlockManager(pFile, mode==IFile::O_Truncate, offset_type(1024));
    BlockFS *pFS = new BlockFS(pMgr, mode);

    pFS->AddFile("test123", "test123", offset_type(sizeof("test123")));
    pFS->AddFile("test1234", "test1234", offset_type(sizeof("test1234")));

    IFile* pf1 = pFS->OpenFileInPackage("test123");
    IFile* pf2 = pFS->OpenFileInPackage("test1234");
    assert(pf1->GetSize()==offset_type(sizeof("test123")));
    assert(pf2->GetSize()==offset_type(sizeof("test1234")));
    char buffer[16];
    offset_type r = pf1->Read(buffer, offset_type(sizeof(buffer)));
    assert(r == offset_type(sizeof("test123")));
    assert(strcmp(buffer, "test123")==0);

    r = pf2->Read(buffer, offset_type(sizeof(buffer)));
    assert(r == offset_type(sizeof("test1234")));
    assert(strcmp(buffer, "test1234")==0);

    delete pf1;
    delete pf2;
    delete pFS;
    delete pMgr;
    delete pFile;
}