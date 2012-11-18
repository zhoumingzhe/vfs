#include <crtdbg.h>
#include "../VFS/IFile.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile* pFile = OpenDiskFile("test.txt", IFile::O_Truncate);
    char buffer[] = "123456";
    pFile->Write(buffer, sizeof(buffer));
    pFile->ReserveSpace(10);
    delete pFile;
    return 0;
}