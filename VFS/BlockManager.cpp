#include <assert.h>
#include "BlockManager.h"
#include "IFile.h"

BlockManager::BlockManager(IFile* pFile, bool truncate, int blocksize)
    :m_pFile(pFile), m_Header(blocksize, -1, 0)
{
    if(truncate)
    {
        CreateNew(blocksize);
    }
    else
    {
        LoadExist(blocksize);
    }
}

BlockManager::~BlockManager(void)
{
}

void BlockManager::CreateNew(int blocksize)
{
    m_pFile->Seek(0, IFile::S_Begin);
    m_pFile->Write(&m_Header, sizeof(m_Header));
    m_pFile->ReserveSpace(sizeof(m_Header));
}

void BlockManager::LoadExist(int blocksize)
{
    m_pFile->Seek(0, IFile::S_Begin);
    m_pFile->Read(&m_Header, sizeof(m_Header));
    assert(m_Header.BlockSize == blocksize && "Block Size Error");
    CheckHeader();
}

int BlockManager::GetBlockSize()
{
    return m_Header.BlockSize;
}

int BlockManager::GetBlocks()
{
    return m_Header.Blocks;
}

int BlockManager::GetUnused()
{
    return m_Header.Unused;
}

void BlockManager::CheckHeader()
{
    assert(m_Header.Unused<m_Header.Blocks && "Unused Should be Less than Blocks");
    assert(m_pFile->GetSize() == m_Header.BlockSize*m_Header.Blocks + sizeof(m_Header)
        && "Wrong Header");
}

void BlockManager::FlushHeader()
{
    CheckHeader();
    m_pFile->Seek(0, IFile::S_Begin);
    m_pFile->Write(&m_Header, sizeof(m_Header));
}

int BlockManager::CalcOffset( int blockid )
{
    return sizeof(m_Header)+m_Header.BlockSize*blockid;
}

int BlockManager::AllocBlock()
{
    if(m_Header.Unused < 0)
        return AllocNewBlock();
    else
        return AllocExistBlock();
}

void BlockManager::RecycleBlock( int blockid )
{
    assert(blockid >=0 && blockid < m_Header.Blocks && "Recycle out of range");
    assert(blockid != m_Header.Unused && "Recycle again?");
    if(blockid == m_Header.Blocks -1)//shrink file size
    {
        --m_Header.Blocks;
        while (m_Header.Unused == m_Header.Blocks -1 && m_Header.Unused >= 0)
        {
            int header = -1;
            ReadEmptyBlockHeader(m_Header.Unused, header);
            m_Header.Unused = header;
            --m_Header.Blocks;
            assert(m_Header.Blocks >=0);
            assert(m_Header.Unused < m_Header.Blocks);
        }
        m_pFile->ReserveSpace(CalcOffset(m_Header.Blocks));
        FlushHeader();
    }
    else
    {
        if(blockid > m_Header.Unused)
        {
            int header = m_Header.Unused;
            WriteEmptyBlockHeader(blockid, header);
            m_Header.Unused = blockid;
            FlushHeader();
        }
        else
        {
            int unused = m_Header.Unused;
            int found = 0;
            int header = -1;
            while (blockid < unused)
            {
                ReadEmptyBlockHeader(unused, header);
                found = unused;
                unused = header;
            }
            WriteEmptyBlockHeader(found, blockid);
            WriteEmptyBlockHeader(blockid, unused);
        }
    }
}

void BlockManager::ReadEmptyBlockHeader( int blockid, int &header )
{
    assert(blockid < m_Header.Blocks && "out of range");
    m_pFile->Seek(CalcOffset(blockid), IFile::S_Begin);
    m_pFile->Read(&header, sizeof(header));
}

void BlockManager::WriteEmptyBlockHeader( int blockid, int header )
{
    assert(blockid < m_Header.Blocks && "out of range");
    m_pFile->Seek(CalcOffset(blockid), IFile::S_Begin);
    m_pFile->Write(&header, sizeof(header));
}

int BlockManager::AllocNewBlock()
{
    CheckHeader();
    assert(m_Header.Unused < 0);
    int blockid = m_Header.Blocks++;
    m_pFile->ReserveSpace(CalcOffset(m_Header.Blocks));
    FlushHeader();
    return blockid;
}

int BlockManager::AllocExistBlock()
{
    CheckHeader();
    assert(m_Header.Unused >= 0);
    int result = m_Header.Unused;
    int header;
    ReadEmptyBlockHeader(result, header);
    m_Header.Unused = header;
    header = result;
    ZeroBlock(result);
    FlushHeader();
    return result;
}

void BlockManager::ZeroBlock( int blockid )
{
    m_pFile->Seek(CalcOffset(blockid), IFile::S_Begin);
    std::vector<char>buffer(m_Header.BlockSize);//performance penalties
    m_pFile->Write(&buffer[0], m_Header.BlockSize);
}

