#pragma once
class IFile;
#include <vector>
struct BlockData
{
    struct BlockHeader
    {
        int Next;
        int Prev;
        int Begin;
        int Reserved;
        BlockHeader():Next(-1),Prev(-1),Begin(-1),Reserved(0)
        {}
    };
    BlockHeader m_Header;
    std::vector<char> m_Data;
};

class BlockManager
{
    struct FileHeader
    {
        int BlockSize;  //sizeof one block
        int Unused;     //the last unused blockid
        int Blocks;
        int Reserved;
        FileHeader(int blocksize, int unused, int blocks):
            BlockSize(blocksize),
            Unused(unused),
            Blocks(blocks),
            Reserved(0)
        {}
    };

public:
    BlockManager(IFile* pFile, bool truncate, int blocksize);
    ~BlockManager(void);

    int GetBlockSize();
    int GetBlocks();
    int GetUnused();

    void CheckHeader();
    void FlushHeader();

    int AllocBlock();
    void RecycleBlock(int blockid);

    int AllocNewBlock();
    int AllocExistBlock();

    void ReadBlockHeader(int blockid, BlockData::BlockHeader& header);
    void WriteBlockHeader(int blockid, const BlockData::BlockHeader &header);
    void ReadBlock(int blockid, BlockData& data);
    void WriteBlock(int blockid, const BlockData& data);
private:
    void CreateNew(int blocksize);
    void LoadExist(int blocksize);

    int CalcOffset(int blockid);
    IFile* m_pFile;

    FileHeader m_Header;
};

