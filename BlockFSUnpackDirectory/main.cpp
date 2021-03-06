#include <crtdbg.h>
#include <assert.h>
#include <windows.h>
#include "../VFS/BlockManager.h"
#include "../VFS/IFile.h"
#include "../VFS/BlockFS.h"
#include "../VFS/UnpackedFile.h"

int main(int argc, char** argv)
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    IFile::OpenMode mode = IFile::O_ReadOnly;

    assert(argc>1);
    IFile *pFile = OpenDiskFile(argv[1], mode);
    BlockManager* pMgr = new BlockManager(pFile, mode==IFile::O_Truncate, offset_type(1024));

    BlockFS *pFS = new BlockFS(pMgr, mode);

    std::vector<std::string> names;
    pFS->ExportFileNames(names);
    for (std::vector<std::string>::iterator it = names.begin();
        it != names.end(); ++it)
    {
        IFile* pUnpackedFile = pFS->OpenFileInPackage(it->c_str());
        IFile* pTemp = OpenDiskFile(it->c_str(), IFile::O_Truncate);

        offset_type length = pUnpackedFile->GetSize();
        char* buff = new char[(size_t)length.offset];
        printf("Unpacking %s\n", it->c_str());

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