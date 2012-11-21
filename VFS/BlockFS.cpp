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
    std::vector<char> data;
    m_pMgr->ReadBlockData(id, data);
    int headersize = sizeof(cache.header);
    //2 memory copies, performance penalties
    memcpy(&cache.header, &data[0], headersize);
    memcpy(&cache.data[0], &data[headersize], data.size() - headersize);
    return true;
}
