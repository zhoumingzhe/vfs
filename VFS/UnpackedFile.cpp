#include "UnpackedFile.h"
#include "BlockFS.h"

UnpackedFile::UnpackedFile(BlockFS* pFS, OpenMode mode):m_pFS(pFS)
{
}


UnpackedFile::~UnpackedFile(void)
{
    m_pFS->OnFileDestory(this);
}
