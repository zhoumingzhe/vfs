#pragma once
#include "IFile.h"

class BlockManager;
class BlockFS
{
public:
    BlockFS(BlockManager* pMgr, IFile::OpenMode mode);
    ~BlockFS(void);
    IFile* CreateBlockFile(const char* name, IFile::OpenMode mode);
private:
    BlockManager *m_pMgr;
};

