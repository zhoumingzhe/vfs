#pragma once
#include <vector>
#include "IFile.h"
class FileLock;
class BlockManager
{
    struct FileHeader
    {
        offset_type BlockSize;  //sizeof one block
        offset_type Unused;     //the last unused blockid
        offset_type Blocks;
        offset_type Reserved;
        FileHeader(offset_type blocksize, offset_type unused, offset_type blocks):
            BlockSize(blocksize),
            Unused(unused),
            Blocks(blocks)
        {
            Reserved.offset = 0;
        }
    };

public:
    BlockManager(IFile* pFile, bool truncate, offset_type blocksize);
    ~BlockManager(void);

    offset_type GetBlockSize();
    offset_type GetBlocks();
    offset_type GetUnused();


    offset_type AllocBlock();
    void RecycleBlock(offset_type blockid);

    void ReadPartialBlockData(offset_type blockid, void* buffer, offset_type offset, offset_type length);
    void WritePartialBlockData(offset_type blockid, const void* buffer, offset_type offset, offset_type length);
private:

    void CheckHeader();
    void FlushHeader();
    void ZeroBlock(offset_type blockid);
    void ReadEmptyBlockHeader(offset_type blockid, offset_type &header);
    void WriteEmptyBlockHeader(offset_type blockid, offset_type header);

    void CreateNew(offset_type blocksize);
    void LoadExist(offset_type blocksize);

    offset_type AllocNewBlock();
    offset_type AllocExistBlock();

    offset_type CalcOffset(offset_type blockid);
    IFile* m_pFile;
    FileHeader m_Header;
    FileLock* m_pLock;
};

