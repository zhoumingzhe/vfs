#include <crtdbg.h>
#include <assert.h>
#include <windows.h>
#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"
#include "../VFS/BlockFS.h"
#include "../VFS/UnpackedFile.h"

int main()
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile::OpenMode mode = IFile::O_ReadOnly;
    IFile *pFile = OpenDiskFile("test.pkg", mode);
    BlockManager* pMgr = new BlockManager(pFile, mode==IFile::O_Truncate, 1024);
    BlockFS *pFS = new BlockFS(pMgr, mode);

    std::vector<std::string> names;
    pFS->ExportFileNames(names);
    for (std::vector<std::string>::iterator it = names.begin();
        it != names.end(); ++it)
    {
        UnpackedFile* pUnpackedFile = pFS->OpenUnpackedFile(it->c_str());
        IFile* pTemp = OpenDiskFile(it->c_str(), IFile::O_Truncate);
        int length = pUnpackedFile->GetSize();
        char* buff = new char[length];
        pUnpackedFile->Read(buff, length);
        pTemp->Write(buff, length);

        delete []buff;
        delete pTemp;
        delete pUnpackedFile;
    }
    delete pFS;
    delete pMgr;
    delete pFile;
}