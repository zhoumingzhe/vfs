#pragma once
#include <set>
#include "IFile.h"

class BlockManager;
class BlockFS
{
public:
    BlockFS(BlockManager* pMgr, IFile::OpenMode mode);
    ~BlockFS(void);
    IFile* CreateBlockFile(const char* name, IFile::OpenMode mode);

    IFile* CreateBlockFile(int blockid, IFile::OpenMode mode);
    void OnFileDestory(IFile* pFile);
private:
    BlockManager *m_pMgr;
    std::set<IFile*> m_opened;
    IFile* m_pFirst;
};

