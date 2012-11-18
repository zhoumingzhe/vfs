#include <assert.h>
#include <crtdbg.h>
#include "../VFS/IFile.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile* pFile = OpenDiskFile("test.txt", IFile::O_Write);
    char buffer[20] = "";
    int read = pFile->Read(buffer, sizeof(buffer));
    assert(read == 10);
    delete pFile;
    return 0;
}