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

    static const int compress_uncompressed = 0;
    static const int compress_zlib = 1;

    char name[max_length];
    int start_id;
    int compress_method;
    MD5Index index;
    BlockFileEntry();
};


class BlockFS
{
public:
    BlockFS(BlockManager* pMgr, IFile::OpenMode mode);
    ~BlockFS(void);

    UnpackedFile* CreateBlockFile(int blockid, IFile::OpenMode mode);

    void AddFile(const char* name, char* data, int length,
        int compression = BlockFileEntry::compress_uncompressed);
    void RemoveFile(const char* name);

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

    int GetNextBlockId(int blockid, int beginid);
    int AppendOrGetNextBlockId(int blockid, int beginid);

    int AppendBlock(int end, int begin);
    void ExportFileNames(std::vector<std::string>& names);
    //test only;
    IFile* First() const { return m_pFirst; }
private:

    int FindFirstUnusedEntry();
    void FlushEntry(int entry);

    BlockManager *m_pMgr;
    std::set<IFile*> m_opened;
    IFile* m_pFirst;

    std::vector<BlockFileEntry> m_entry;
    std::map<std::string, int> m_nameIndex;
    std::map<MD5Index, std::set<int> > m_md5Index;
};

