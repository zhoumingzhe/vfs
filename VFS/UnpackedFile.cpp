#include "UnpackedFile.h"
#include "BlockFS.h"

UnpackedFile::UnpackedFile(BlockFS* pFS, OpenMode mode, int beginid):
    m_pFS(pFS),
    m_Current(0),
    m_Blockid(beginid),
    m_Beginid(beginid)
{
    m_pFS->LoadCache(m_Beginid, m_Cache);
}


UnpackedFile::~UnpackedFile(void)
{
    m_pFS->OnFileDestory(this);
}

int UnpackedFile::Read( void* buffer, int size )
{
    return 0;
}
