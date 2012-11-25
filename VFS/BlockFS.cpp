#include <assert.h>
#include "BlockFS.h"
#include "BlockManager.h"
#include "UnpackedFile.h"
#include "PackedFile.h"

BlockFS::BlockFS(BlockManager* pMgr, IFile::OpenMode mode):m_pMgr(pMgr),m_pFirst(0)
{
    if(m_pMgr->GetBlocks()>0)
    {
        m_pFirst = CreateBlockFile(0, mode);
    }
    else
    {
        int i = m_pMgr->AllocBlock();
        assert(i==0);
        m_pFirst = CreateBlockFile(0, mode);
    }
}


BlockFS::~BlockFS(void)
{
    assert(m_opened.empty()&&"not all file closed");
}

IFile* BlockFS::CreateBlockFile( const char* name, IFile::OpenMode mode )
{
    assert(0&&"not implemented");
    return 0;
}

IFile* BlockFS::CreateBlockFile( int blockid, IFile::OpenMode mode )
{
    IFile* pFile = new UnpackedFile(this, mode, blockid);
    m_opened.insert(pFile);
    return pFile;
}

void BlockFS::OnFileDestory( IFile* pFile )
{
    std::set<IFile*>::iterator it = m_opened.find(pFile);
    assert(it != m_opened.end());
    m_opened.erase(it);
}

bool BlockFS::LoadCache( int id, BlockCache& cache )
{
    LoadBlockHeader(id, cache.header);
    LoadBlockData(id, cache.data);
    return true;
}

bool BlockFS::LoadBlockHeader( int id, BlockHeader &header )
{
    m_pMgr->ReadPartialBlockData(id, &header, 0, sizeof(header));
    return true;
}

bool BlockFS::LoadBlockData( int id, std::vector<char>& data )
{
    data.resize(m_pMgr->GetBlockSize() - sizeof(BlockHeader));
    m_pMgr->ReadPartialBlockData(id, &data[0], sizeof(BlockHeader), data.size());
    return true;
}

bool BlockFS::FlushBlockHeader( int id, const BlockHeader &header )
{
    m_pMgr->WritePartialBlockData(id, &header, 0, sizeof(header));
    return true;
}

bool BlockFS::FlushBlockData( int id, const std::vector<char>& data )
{
    assert(data.size() == m_pMgr->GetBlockSize() - sizeof(BlockHeader));
    m_pMgr->WritePartialBlockData(id, &data[0], sizeof(BlockHeader), data.size());
    return true;
}

bool BlockFS::FlushCache( int id, BlockCache& cache )
{
    FlushBlockHeader(id, cache.header);
    FlushBlockData(id, cache.data);
    return true;
}

int BlockFS::GetBlockDataSize()
{
    return m_pMgr->GetBlockSize() - sizeof(BlockHeader);
}

int BlockFS::AllocBlock(const BlockHeader& header)
{
    int i = m_pMgr->AllocBlock();
    FlushBlockHeader(i, header);
    return i;
}
