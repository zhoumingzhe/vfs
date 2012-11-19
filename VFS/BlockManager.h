#pragma once
class IFile;
#include <vector>

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

    void ReadEmptyBlockHeader(int blockid, int &header);
    void WriteEmptyBlockHeader(int blockid, int header);

    void ZeroBlock(int blockid);

    void ReadBlockData(int blockid, std::vector<char>& data);
    void WriteBlockData(int blockid, std::vector<char>& data);
private:
    void CreateNew(int blocksize);
    void LoadExist(int blocksize);

    int AllocNewBlock();
    int AllocExistBlock();

    int CalcOffset(int blockid);
    IFile* m_pFile;

    FileHeader m_Header;
};

