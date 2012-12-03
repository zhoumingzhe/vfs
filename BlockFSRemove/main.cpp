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
    BlockManager* pMgr = new BlockManager(pFile, mode==IFile::O_Truncate, 1024);
    BlockFS *pFS = new BlockFS(pMgr, mode);

    pFS->AddUnpackedFile("test123", "test123", sizeof("test123"));
    pFS->AddUnpackedFile("test1234", "test1234", sizeof("test1234"));

    UnpackedFile* pf1 = pFS->OpenUnpackedFile("test123");
    UnpackedFile* pf2 = pFS->OpenUnpackedFile("test1234");
    assert(pf1->GetSize()==sizeof("test123"));
    assert(pf2->GetSize()==sizeof("test1234"));
    char buffer[16];
    int r = pf1->Read(buffer, sizeof(buffer));
    assert(r == sizeof("test123"));
    assert(strcmp(buffer, "test123")==0);

    r = pf2->Read(buffer, sizeof(buffer));
    assert(r == sizeof("test1234"));
    assert(strcmp(buffer, "test1234")==0);

    delete pf1;
    delete pf2;

    pFS->RemoveFile("test123");
    pFS->RemoveFile("test1234");

    delete pFS;
    delete pMgr;
    delete pFile;
}