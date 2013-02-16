#include <crtdbg.h>
#include "../VFS/IFile.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile* pFile = OpenDiskFile("test.txt", IFile::O_Truncate);
    char buffer[] = "123456";
    pFile->Write(buffer, offset_type(sizeof(buffer)));
    pFile->ReserveSpace(offset_type(10));
    delete pFile;
    return 0;
}