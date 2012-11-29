#pragma once
#include <set>
#include <vector>
#include <map>
#include <string>
#include "IFile.h"
class UnpackedFile;
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

typedef unsigned char MD5[16];

struct MD5Index
{
    MD5 md5;
    int size;
    MD5Index();
    bool operator<(const MD5Index& rhs)const;
};

struct BlockFileEntry
{
    static const int max_length = 256;

    char name[max_length];
    int start_id;
    MD5Index index;
    BlockFileEntry();
};


class BlockFS
{
public:
    BlockFS(BlockManager* pMgr, IFile::OpenMode mode);
    ~BlockFS(void);

    UnpackedFile* CreateBlockFile(int blockid, IFile::OpenMode mode);

    void AddUnpackedFile(const char* name, char* data, int length);

    UnpackedFile* OpenUnpackedFile(const char* name);
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

    int FindFirstUnusedEntry();
    BlockManager *m_pMgr;
    std::set<IFile*> m_opened;
    IFile* m_pFirst;

    std::vector<BlockFileEntry> m_entry;
    std::map<std::string, int> m_nameIndex;
    std::map<MD5Index, std::set<int> > m_md5Index;
};

