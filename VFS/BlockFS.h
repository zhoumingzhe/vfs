#pragma once
#include <set>
#include <vector>
#include "IFile.h"

class BlockManager;

struct BlockHeader
{
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
    bool FlushCache(int id, BlockCache& cache);

    bool LoadBlockHeader(int id, BlockHeader &header);
    bool LoadBlockData(int id, std::vector<char>& data);

    bool FlushBlockHeader(int id, const BlockHeader &header);
    bool FlushBlockData(int id, const std::vector<char>& data);

    int GetBlockDataSize();
    int AllocBlock(const BlockHeader& header);

    //test only;
    IFile* First() const { return m_pFirst; }
private:
    BlockManager *m_pMgr;
    std::set<IFile*> m_opened;
    IFile* m_pFirst;
};

