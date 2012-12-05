#pragma once
#include <vector>

class IFile;
class FileLock;
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


    int AllocBlock();
    void RecycleBlock(int blockid);

    void ReadPartialBlockData(int blockid, void* buffer, int offset, int length);
    void WritePartialBlockData(int blockid, const void* buffer, int offset, int length);
private:

    void CheckHeader();
    void FlushHeader();
    void ZeroBlock(int blockid);
    void ReadEmptyBlockHeader(int blockid, int &header);
    void WriteEmptyBlockHeader(int blockid, int header);

    void CreateNew(int blocksize);
    void LoadExist(int blocksize);

    int AllocNewBlock();
    int AllocExistBlock();

    int CalcOffset(int blockid);
    IFile* m_pFile;
    FileHeader m_Header;
    FileLock* m_pLock;
};

