#include <crtdbg.h>
#include <assert.h>
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

    const int size = 2048;
    char* buffer = new char[size];
    for(int i = 0; i<size; ++i)
        buffer[i] = i%10;
    int size_write = pFS->First()->Write(buffer, size);
    assert(size==size_write);

    int size_seek = pFS->First()->Seek(0, IFile::S_Begin);
    assert(size == size_seek);
    char* buffer1 = new char[size];
    int size_read = pFS->First()->Read(buffer1, size);
    assert(size_read == size);
    int cmp_result = memcmp(buffer1, buffer, size);
    assert(!cmp_result);
    delete[]buffer1;
    delete[]buffer;

    delete pFS;
    delete pMgr;
    delete pFile;
}