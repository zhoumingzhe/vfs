#pragma once
#include <set>
#include <vector>
#include "IFile.h"

class BlockManager;

struct BlockHeader
{
    int begin;
    int next;
    int prev;
};

struct BlockCache
{
    BlockHeader header;
    std::vector<char> data;
};

class BlockFS
{
public:
    BlockFS(BlockManager* pMgr, IFile::OpenMode mode);
    ~BlockFS(void);

    IFile* CreateBlockFile(const char* name, IFile::OpenMode mode);

    IFile* CreateBlockFile(int blockid, IFile::OpenMode mode);
    void OnFileDestory(IFile* pFile);
    bool LoadCache(int id, BlockCache& cache);
private:
    BlockManager *m_pMgr;
    std::set<IFile*> m_opened;
    IFile* m_pFirst;
};

