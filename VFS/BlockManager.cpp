#include "stdafx.h"
#include <assert.h>
#include "BlockManager.h"
#include "IFile.h"
#include "FileLock.h"

BlockManager::BlockManager(IFile* pFile, bool truncate, offset_type blocksize)
    :m_pFile(pFile), m_Header(blocksize, offset_type(-1), offset_type(0)), m_pLock(new FileLock)
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
    delete m_pLock;
}

void BlockManager::CreateNew(offset_type blocksize)
{
    m_pFile->Seek(offset_type(0), IFile::S_Begin);

    m_pFile->Write(&m_Header, offset_type(sizeof(m_Header)));
    m_pFile->ReserveSpace(offset_type(sizeof(m_Header)));
}

void BlockManager::LoadExist(offset_type blocksize)
{
    m_pFile->Seek(offset_type(0), IFile::S_Begin);

    offset_type sizetoread;
    sizetoread.offset = sizeof(m_Header);
    m_pFile->Read(&m_Header, sizetoread);
    assert(m_Header.BlockSize == blocksize && "Block Size Error");
    CheckHeader();
}

offset_type BlockManager::GetBlockSize()
{
    return m_Header.BlockSize;
}

offset_type BlockManager::GetBlocks()
{
    return m_Header.Blocks;
}

offset_type BlockManager::GetUnused()
{
    return m_Header.Unused;
}

void BlockManager::CheckHeader()
{
    assert(m_Header.Unused<m_Header.Blocks && "Unused Should be Less than Blocks");
    assert(m_pFile->GetSize().offset == m_Header.BlockSize.offset*m_Header.Blocks.offset + sizeof(m_Header)
        && "Wrong Header");
}

void BlockManager::FlushHeader()
{
    CheckHeader();
    m_pFile->Seek(offset_type(0), IFile::S_Begin);

    m_pFile->Write(&m_Header, offset_type(sizeof(m_Header)));
}

offset_type BlockManager::CalcOffset( offset_type blockid )
{
    return offset_type(sizeof(m_Header)+m_Header.BlockSize.offset*blockid.offset);
}

offset_type BlockManager::AllocBlock()
{
    FileAutoLock lock(m_pLock);
    if(m_Header.Unused.offset < 0)
        return AllocNewBlock();
    else
        return AllocExistBlock();
}

void BlockManager::RecycleBlock( offset_type blockid )
{
    FileAutoLock lock(m_pLock);
    assert(blockid.offset >=0 && blockid < m_Header.Blocks && "Recycle out of range");
    assert(blockid != m_Header.Unused && "Recycle again?");
    if(blockid.offset == m_Header.Blocks.offset -1)//shrink file size
    {
        --m_Header.Blocks.offset;
        while (m_Header.Unused.offset == m_Header.Blocks.offset -1 && m_Header.Unused.offset >= 0)
        {
            offset_type header(-1);
            ReadEmptyBlockHeader(m_Header.Unused, header);
            m_Header.Unused = header;
            --m_Header.Blocks.offset;
            assert(m_Header.Blocks.offset >=0);
            assert(m_Header.Unused < m_Header.Blocks);
        }
        m_pFile->ReserveSpace(CalcOffset(m_Header.Blocks));
        FlushHeader();
    }
    else
    {
        if(blockid > m_Header.Unused)
        {
            offset_type header = m_Header.Unused;
            WriteEmptyBlockHeader(blockid, header);
            m_Header.Unused = blockid;
            FlushHeader();
        }
        else
        {
            offset_type unused = m_Header.Unused;
            offset_type found(0);
            offset_type header(-1);
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

void BlockManager::ReadEmptyBlockHeader( offset_type blockid, offset_type &header )
{
    //FileAutoLock lock(m_pLock);
    assert(blockid < m_Header.Blocks && "out of range");
    m_pFile->Seek(CalcOffset(blockid), IFile::S_Begin);
    m_pFile->Read(&header, offset_type(sizeof(header)));
}

void BlockManager::WriteEmptyBlockHeader( offset_type blockid, offset_type header )
{
    assert(header<blockid&&"Wrong Empty Block header");
    assert(blockid < m_Header.Blocks && "out of range");
    m_pFile->Seek(CalcOffset(blockid), IFile::S_Begin);
    m_pFile->Write(&header, offset_type(sizeof(header)));
}

offset_type BlockManager::AllocNewBlock()
{
    CheckHeader();
    assert(m_Header.Unused.offset < 0);
    offset_type blockid = m_Header.Blocks;
    ++m_Header.Blocks.offset;
    m_pFile->ReserveSpace(CalcOffset(m_Header.Blocks));
    FlushHeader();
    return blockid;
}

offset_type BlockManager::AllocExistBlock()
{
    CheckHeader();
    assert(m_Header.Unused.offset >= 0);
    offset_type result = m_Header.Unused;
    offset_type header;
    ReadEmptyBlockHeader(result, header);
    m_Header.Unused = header;
    header = result;
    ZeroBlock(result);
    FlushHeader();
    return result;
}

void BlockManager::ZeroBlock( offset_type blockid )
{
    m_pFile->Seek(CalcOffset(blockid), IFile::S_Begin);
    std::vector<char>buffer((size_t)m_Header.BlockSize.offset);//performance penalties
    m_pFile->Write(&buffer[0], m_Header.BlockSize);
}

void BlockManager::ReadPartialBlockData( offset_type blockid, void* buffer, offset_type offset, offset_type length )
{
    FileAutoLock lock(m_pLock);
    m_pFile->Seek(CalcOffset(blockid) + offset, IFile::S_Begin);
    offset_type result = m_pFile->Read(buffer, length);
    assert(result == length);
}

void BlockManager::WritePartialBlockData( offset_type blockid, const void* buffer, offset_type offset, offset_type length )
{
    FileAutoLock lock(m_pLock);
    m_pFile->Seek(CalcOffset(blockid) + offset, IFile::S_Begin);
    offset_type result = m_pFile->Write(buffer, length);
    assert(result == length);
}

