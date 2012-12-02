#include <assert.h>
#include "BlockFS.h"
#include "BlockManager.h"
#include "UnpackedFile.h"
#include "PackedFile.h"
#include "GUtMd5.h"

MD5Index::MD5Index()
{
    memset(this, 0, sizeof(*this));
}

bool MD5Index::operator<( const MD5Index& rhs )const
{
    return memcmp(this, &rhs, sizeof(*this))<0;
}

BlockFileEntry::BlockFileEntry()
{
    memset(this, 0, sizeof(*this));
}

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

    int size = m_pFirst->GetSize();
    assert(size%sizeof(BlockFileEntry)==0);
    int entries = size/sizeof(BlockFileEntry);

    m_entry.resize(entries);
    if(entries>0)
        m_pFirst->Read(&m_entry[0], size);

    for (int i = 0; i < (int)m_entry.size(); ++i)
    {
        const BlockFileEntry& e = m_entry[i];
        if(e.name != "")//empty entry
        {
            std::map<std::string, int>::iterator itname
                = m_nameIndex.find(e.name);
            assert(itname==m_nameIndex.end());
            m_nameIndex[e.name] = i;

            m_md5Index[e.index].insert(i);
        }

    }
}


BlockFS::~BlockFS(void)
{
    delete m_pFirst;
    assert(m_opened.empty()&&"not all file closed");
}


UnpackedFile* BlockFS::CreateBlockFile( int blockid, IFile::OpenMode mode )
{
    UnpackedFile* pFile = new UnpackedFile(this, mode, blockid);
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

void BlockFS::AddUnpackedFile( const char* name, char* data, int length )
{
    size_t name_length = strlen(name);
    assert(name_length > 0);
    assert(name_length < BlockFileEntry::max_length);
    std::map<std::string, int>::iterator itname = m_nameIndex.find(name);
    assert(itname == m_nameIndex.end());
    BlockFileEntry e;
    GetMD5CheckSum((unsigned char*)data, (unsigned)length, e.index.md5);
    e.index.size = length;
    e.compress_method = BlockFileEntry::compress_uncompressed;
    strcpy_s(e.name, name);

    //if same md5 and size exist, we consider they're the same cotent;
    std::map<MD5Index, std::set<int> >::iterator itindex = m_md5Index.find(e.index);
    if(itindex == m_md5Index.end())
    {
        e.start_id = m_pMgr->AllocBlock();
        BlockHeader header = {e.start_id, e.start_id};
        FlushBlockHeader(e.start_id, header);
        UnpackedFile* pFile = CreateBlockFile(e.start_id, IFile::O_Truncate);
        pFile->Write(data, length);
        pFile->SetRefCount(1);
        pFile->FlushHeaderToDisk();
        delete pFile;
    }
    else
    {
        //just add refcount of the unpacked file;
        assert(!itindex->second.empty());
        int blockidx = *(itindex->second.begin());
        assert(blockidx < (int)m_entry.size() && blockidx >= 0);
        UnpackedFile* pFile = CreateBlockFile(m_entry[blockidx].start_id, IFile::O_Write);
        pFile->SetRefCount(pFile->GetRefCount()+1);
        pFile->FlushHeaderToDisk();
        delete pFile;
    }

    //need optimization to find the free entry
    int i = FindFirstUnusedEntry();
    if(i > 0)
    {
        assert(i < (int)m_entry.size());
        m_entry[i] = e;
    }
    else
    {
        i = m_entry.size();
        m_entry.push_back(e);
    }
    m_pFirst->Seek(i*sizeof(m_entry[0]), IFile::S_Begin);
    m_pFirst->Write(&m_entry[i], sizeof(m_entry[i]));

    m_nameIndex[name] = i;
    m_md5Index[e.index].insert(i);

}

int BlockFS::FindFirstUnusedEntry()
{
    int i = 0;
    for(; i< (int)m_entry.size(); ++i)
    {
        if(m_entry[i].name[0] == 0)
            return i;
    }
    return -1;
}

UnpackedFile* BlockFS::OpenUnpackedFile( const char* name )
{
    std::map<std::string, int>::iterator it = m_nameIndex.find(name);
    if(it == m_nameIndex.end())
        return 0;
    assert(it->second < (int)m_entry.size());
    const BlockFileEntry& e = m_entry[it->second];
    assert(strcmp(e.name, name)==0);
    return CreateBlockFile(e.start_id, IFile::O_ReadOnly);
}

int BlockFS::GetNextBlockId( int blockid, int beginid )
{
    assert(blockid<=m_pMgr->GetBlocks());
    assert(beginid<=m_pMgr->GetBlocks());
    BlockHeader header;
    LoadBlockHeader(blockid, header);

    if(header.next == beginid)
        return -1;
    else
        return header.next;
}

int BlockFS::AppendOrGetNextBlockId( int blockid, int beginid )
{
    assert(blockid<=m_pMgr->GetBlocks());
    assert(beginid<=m_pMgr->GetBlocks());
    BlockHeader header;
    LoadBlockHeader(blockid, header);
    if(header.next == beginid)
        return AppendBlock(blockid, beginid);
    else
        return header.next;
}

int BlockFS::AppendBlock( int end, int begin )
{
    BlockHeader header = {begin, end};
    int i = AllocBlock(header);

    LoadBlockHeader(end, header);
    assert(header.next == begin);
    header.next = i;
    FlushBlockHeader(end, header);

    LoadBlockHeader(begin, header);
    assert(header.prev == end);
    header.prev = i;
    FlushBlockHeader(begin, header);

    return i;
}
