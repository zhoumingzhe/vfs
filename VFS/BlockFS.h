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
    offset_type next;
    offset_type prev;
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
    offset_type size;
    MD5Index();
    bool operator<(const MD5Index& rhs)const;
};

struct BlockFileEntry
{
    static const int max_length = 256;

    static const int compress_uncompressed = 0;
    static const int compress_zlib = 1;

    char name[max_length];
    offset_type start_id;
    int compress_method;
    MD5Index index;
    BlockFileEntry();
};


class BlockFS
{
public:
    BlockFS(BlockManager* pMgr, IFile::OpenMode mode);
    ~BlockFS(void);

    UnpackedFile* CreateBlockFile(offset_type blockid, IFile::OpenMode mode);

    void AddFile(const char* name, char* data, offset_type length,
        int compression = BlockFileEntry::compress_uncompressed);
    void RemoveFile(const char* name);

    IFile* OpenFileInPackage(const char* name);
    void OnFileDestory(IFile* pFile);
    bool LoadCache(offset_type id, BlockCache& cache);
    bool FlushCache(offset_type id, BlockCache& cache);

    bool LoadBlockHeader(offset_type id, BlockHeader &header);
    bool LoadBlockData(offset_type id, std::vector<char>& data);

    bool FlushBlockHeader(offset_type id, const BlockHeader &header);
    bool FlushBlockData(offset_type id, const std::vector<char>& data);

    offset_type GetBlockDataSize();
    offset_type AllocBlock(const BlockHeader& header);

    offset_type GetNextBlockId(offset_type blockid, offset_type beginid);
    offset_type AppendOrGetNextBlockId(offset_type blockid, offset_type beginid);

    offset_type AppendBlock(offset_type end, offset_type begin);
    void ExportFileNames(std::vector<std::string>& names);
    const std::vector<BlockFileEntry> &GetEntries();
    //test only;
    IFile* First() const { return m_pFirst; }
private:

    offset_type FindFirstUnusedEntry();
    void FlushEntry(offset_type entry);

    BlockManager *m_pMgr;
    std::set<IFile*> m_opened;
    IFile* m_pFirst;

    std::vector<BlockFileEntry> m_entry;
    std::map<std::string, offset_type> m_nameIndex;
    std::map<MD5Index, std::set<offset_type> > m_md5Index;
};

