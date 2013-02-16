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
    BlockManager* pMgr = new BlockManager(pFile, mode==IFile::O_Truncate, offset_type(1024));
    BlockFS *pFS = new BlockFS(pMgr, mode);

    const int size = 2048;
    char* buffer = new char[size];
    for(int i = 0; i<size; ++i)
        buffer[i] = i%10;
    offset_type size_write = pFS->First()->Write(buffer, offset_type(size));
    assert(offset_type(size)==size_write);
    srand((unsigned)time(0));
    for(int i = 0; i<4096; ++i)
    {
        int offset = rand()%size;
        pFS->First()->Seek(offset_type(offset), IFile::S_Begin);
        char c;
        pFS->First()->Read(&c, offset_type(sizeof(c)));
        assert(c==offset%10);
    }

    pFS->First()->Seek(offset_type(0), IFile::S_Begin);
    char* buffer1 = new char[size];
    offset_type size_read = pFS->First()->Read(buffer1, offset_type(size));
    assert(size_read == offset_type(size));
    int cmp_result = memcmp(buffer1, buffer, size);
    assert(!cmp_result);

    for(int i = 0; i<4096; ++i)
    {
        int offset = rand()%size;
        pFS->First()->Seek(offset_type(offset), IFile::S_Begin);
        char c;
        pFS->First()->Read(&c, offset_type(sizeof(c)));
        assert(c==offset%10);
    }
    delete[]buffer1;
    delete[]buffer;

    delete pFS;
    delete pMgr;
    delete pFile;
}