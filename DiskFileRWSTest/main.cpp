#include <assert.h>
#include <crtdbg.h>
#include "../VFS/IFile.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile* pFile = OpenDiskFile("test.txt", IFile::O_Write);
    char buffer[20] = "";
    offset_type read = pFile->Read(buffer, offset_type(sizeof(buffer)));
    assert(read == offset_type(10));
    delete pFile;
    return 0;
}