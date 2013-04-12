#include "stdafx.h"
#include "../zlib/zlib.h"
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
    if(m_pMgr->GetBlocks().offset>0)
    {
        offset_type id;
        id.offset = 0;
        m_pFirst = CreateBlockFile(id, mode);
    }
    else
    {
        offset_type i = m_pMgr->AllocBlock();
        assert(i.offset==0);
        offset_type id;
        id.offset = 0;
        m_pFirst = CreateBlockFile(id, mode);
    }

    offset_type size = m_pFirst->GetSize();
    assert(size.offset%sizeof(BlockFileEntry)==0);
    offset_type entries;
    entries.offset = size.offset/sizeof(BlockFileEntry);

    m_entry.resize((size_t)entries.offset);
    if(entries.offset>0)
        m_pFirst->Read(&m_entry[0], size);

    for (int i = 0; i < (int)m_entry.size(); ++i)
    {
        const BlockFileEntry& e = m_entry[i];
        if(e.name != "")//empty entry
        {
            std::map<std::string, offset_type>::iterator itname
                = m_nameIndex.find(e.name);
            assert(itname==m_nameIndex.end());
            m_nameIndex[e.name] = offset_type(i);

            m_md5Index[e.index].insert(offset_type(i));
        }

    }
}


BlockFS::~BlockFS(void)
{
    delete m_pFirst;
    assert(m_opened.empty()&&"not all file closed");
}


UnpackedFile* BlockFS::CreateBlockFile( offset_type blockid, IFile::OpenMode mode )
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

bool BlockFS::LoadCache( offset_type id, BlockCache& cache )
{
    LoadBlockHeader(id, cache.header);
    LoadBlockData(id, cache.data);
    return true;
}

bool BlockFS::LoadBlockHeader( offset_type id, BlockHeader &header )
{
    m_pMgr->ReadPartialBlockData(id, &header, offset_type(0), offset_type(sizeof(header)));
    return true;
}

bool BlockFS::LoadBlockData( offset_type id, std::vector<char>& data )
{
    data.resize((size_t)m_pMgr->GetBlockSize().offset - sizeof(BlockHeader));
    m_pMgr->ReadPartialBlockData(id, &data[0], offset_type(sizeof(BlockHeader)), offset_type(data.size()));
    return true;
}

bool BlockFS::FlushBlockHeader( offset_type id, const BlockHeader &header )
{
    m_pMgr->WritePartialBlockData(id, &header, offset_type(0), offset_type(sizeof(header)));
    return true;
}

bool BlockFS::FlushBlockData( offset_type id, const std::vector<char>& data )
{
    assert(data.size() == m_pMgr->GetBlockSize().offset - sizeof(BlockHeader));
    m_pMgr->WritePartialBlockData(id, &data[0], offset_type(sizeof(BlockHeader)), offset_type(data.size()));
    return true;
}

bool BlockFS::FlushCache( offset_type id, BlockCache& cache )
{
    FlushBlockHeader(id, cache.header);
    FlushBlockData(id, cache.data);
    return true;
}

offset_type BlockFS::GetBlockDataSize()
{
    return offset_type(m_pMgr->GetBlockSize().offset - sizeof(BlockHeader));
}

offset_type BlockFS::AllocBlock(const BlockHeader& header)
{
    offset_type i = m_pMgr->AllocBlock();
    FlushBlockHeader(i, header);
    return i;
}

static int CompressData(IFile *pOut, void* data, offset_type length)
{
    if(length.offset <= 0)
        return Z_OK;
    const int chunk = 1024*1024;
    int ret;
    unsigned have;
    z_stream strm;
    std::vector<unsigned char> out;

    out.resize(chunk);

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_BEST_SPEED);
    if (ret != Z_OK)
        return ret;

    strm.avail_in = (unsigned int)length.offset;
    strm.next_in = (unsigned char*)data;

    do {
        strm.avail_out = chunk;
        strm.next_out = &out[0];
        ret = deflate(&strm, Z_FINISH);    /* no bad return value */
        assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
        have = chunk - strm.avail_out;
        pOut->Write(&out[0], offset_type(have));
    } while (strm.avail_out == 0);
    assert(strm.avail_in == 0);     /* all input will be used */

    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    deflateEnd(&strm);
    return Z_OK;
}

void BlockFS::AddFile( const char* name, char* data, offset_type length, int compression )
{
    size_t name_length = strlen(name);
    assert(name_length > 0);
    assert(name_length < BlockFileEntry::max_length);
    std::map<std::string, offset_type>::iterator itname = m_nameIndex.find(name);
    assert(itname == m_nameIndex.end());
    BlockFileEntry e;
    GetMD5CheckSum((unsigned char*)data, (unsigned)length.offset, e.index.md5);
    e.index.size = length;
    e.compress_method = compression;
    strcpy(e.name, name);

    //if same md5 and size exist, we consider they're the same cotent;
    std::map<MD5Index, std::set<offset_type> >::iterator itindex = m_md5Index.find(e.index);
    if(itindex == m_md5Index.end())
    {
        e.start_id = m_pMgr->AllocBlock();
        BlockHeader header = {e.start_id, e.start_id};
        FlushBlockHeader(e.start_id, header);
        UnpackedFile* pFile = CreateBlockFile(e.start_id, IFile::O_Truncate);
        switch (compression)
        {
        case BlockFileEntry::compress_uncompressed:
            pFile->Write(data, length);
            break;
        case BlockFileEntry::compress_zlib:
            CompressData(pFile, data, length);
            break;
        default:
            assert(0);
            break;
        }
        pFile->SetRefCount(1);
        pFile->FlushHeaderToDisk();
        delete pFile;
    }
    else
    {
        //just add refcount of the unpacked file;
        assert(!itindex->second.empty());
        offset_type blockidx = *(itindex->second.begin());
        e.start_id = m_entry[(size_t)blockidx.offset].start_id;
        assert(blockidx < offset_type(m_entry.size()) && blockidx >= offset_type(0));
        UnpackedFile* pFile = CreateBlockFile(m_entry[(size_t)blockidx.offset].start_id, IFile::O_Write);
        pFile->SetRefCount(pFile->GetRefCount()+1);
        pFile->FlushHeaderToDisk();
        delete pFile;
    }

    //need optimization to find the free entry
    offset_type i = FindFirstUnusedEntry();
    if(i >= offset_type(0))
    {
        assert(i < offset_type(m_entry.size()));
        m_entry[(size_t)i.offset] = e;
    }
    else
    {
        i.offset = m_entry.size();
        m_entry.push_back(e);
    }
    FlushEntry(i);

    m_nameIndex[name] = i;
    m_md5Index[e.index].insert(i);

}

offset_type BlockFS::FindFirstUnusedEntry()
{
    int i = 0;
    for(; i< (int)m_entry.size(); ++i)
    {
        if(m_entry[i].name[0] == 0)
            return offset_type(i);
    }
    return offset_type(-1);
}

IFile* BlockFS::OpenFileInPackage( const char* name )
{
    std::map<std::string, offset_type>::iterator it = m_nameIndex.find(name);
    if(it == m_nameIndex.end())
        return 0;
    assert(it->second < offset_type(m_entry.size()));
    const BlockFileEntry& e = m_entry[(size_t)it->second.offset];
    assert(strcmp(e.name, name)==0);
    switch (e.compress_method)
    {
    case BlockFileEntry::compress_uncompressed:
        return CreateBlockFile(e.start_id, IFile::O_ReadOnly);
        break;
    case BlockFileEntry::compress_zlib:
        {
            UnpackedFile* pFile = CreateBlockFile(e.start_id, IFile::O_ReadOnly);
            PackedFile* pUnpacked = new PackedFile(pFile, e.index.size);
            return pUnpacked;
            break;
        }
    default:
        return 0;
        break;
    }

}

offset_type BlockFS::GetNextBlockId( offset_type blockid, offset_type beginid )
{
    assert(blockid<=m_pMgr->GetBlocks());
    assert(beginid<=m_pMgr->GetBlocks());
    BlockHeader header;
    LoadBlockHeader(blockid, header);

    if(header.next == beginid)
        return offset_type(-1);
    else
        return header.next;
}

offset_type BlockFS::AppendOrGetNextBlockId( offset_type blockid, offset_type beginid )
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

offset_type BlockFS::AppendBlock( offset_type end, offset_type begin )
{
    BlockHeader header = {begin, end};
    offset_type i = AllocBlock(header);

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

void BlockFS::RemoveFile( const char* name )
{
    assert(name[0]&&"file name is null");
    std::map<std::string, offset_type>::iterator it = m_nameIndex.find(name);
    if(it == m_nameIndex.end())
        assert(0&&"file not exist");
    offset_type id = it->second;
    assert(id < offset_type(m_entry.size()));
    BlockFileEntry& entry = m_entry[(size_t)id.offset];
    assert(entry.name[0] && "name index and entry vector inconsistent");
    assert(strcmp(entry.name, name) == 0&&"name index and entry vector inconsistent");
    offset_type blockid = entry.start_id;

    std::map<MD5Index, std::set<offset_type> >::iterator itmd5 = m_md5Index.find(entry.index);
    assert(itmd5 != m_md5Index.end());
    std::set<offset_type>::iterator itset = itmd5->second.find(id);
    assert(itset != itmd5->second.end());


    BlockHeader header;
    LoadBlockHeader(blockid, header);

    offset_type currentid = header.prev;
    while(currentid != blockid)
    {
        LoadBlockHeader(currentid, header);
        assert(header.next >= 0 && "error block id");
        assert(header.prev >= 0 && "error block id");
        m_pMgr->RecycleBlock(currentid);
        currentid = header.prev;
    }
    m_pMgr->RecycleBlock(blockid);

    memset(&entry, 0, sizeof(entry));
    FlushEntry(id);
    //todo: shrink file size 
    m_nameIndex.erase(it);
    itmd5->second.erase(itset);
    if(itmd5->second.empty())
        m_md5Index.erase(itmd5);

}

void BlockFS::FlushEntry( offset_type entry )
{
    m_pFirst->Seek(offset_type(entry.offset*sizeof(m_entry[0])), IFile::S_Begin);
    m_pFirst->Write(&m_entry[(size_t)entry.offset], offset_type(sizeof(m_entry[0])));
}

void BlockFS::ExportFileNames( std::vector<std::string>& names )
{
    names.clear();
    names.reserve(m_nameIndex.size());
    for(std::map<std::string, offset_type>::iterator it = m_nameIndex.begin();
        it != m_nameIndex.end(); ++it)
    {
        names.push_back(it->first);
    }
}

const std::vector<BlockFileEntry> & BlockFS::GetEntries()
{
    return m_entry;
}
