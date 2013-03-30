#include "Md5Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include "../VFS/GUtMd5.h"
#include <assert.h>
void HashToString(const unsigned char hash[16], char str[16*2+1])
{
    for(int i = 0; i < 16; ++i)
        sprintf(&str[i*2], "%02x", (int)hash[i]);
    str[16*2] = '\0';
}
void StringToHash(const char str[32], unsigned char hash[16])
{
    for(int i = 0; i < 16; ++i)
    {
        char tmp[3];
        tmp[0] = str[i*2];
        tmp[1] = str[i*2 + 1];
        tmp[2] = '\0';
        char *st;
        hash[i] = (unsigned char)strtol(tmp, &st, 16);
    }
}

void GenerateHash(const std::string & name, unsigned char hash[16],
                  std::vector<Md5Digest> &hash_set, offset_type & length)
{
    hash_set.clear();
    MD5Context ctx;
    MD5Init(&ctx, 0);

    IFile* pTemp = OpenDiskFile(name.c_str(), IFile::O_ReadOnly);
    length = pTemp->GetSize();
    hash_set.reserve(size_t(length.offset/block_size.offset));
    std::vector<unsigned char> buffer((size_t)block_size.offset);
    for(offset_type i(0); i<length; i = i + block_size)
    {
        offset_type size_read = pTemp->Read(&buffer[0], block_size);
        assert(size_read.offset>0);
        MD5Update(&ctx, &buffer[0], (unsigned int)size_read.offset);

        MD5Context ctx_file;
        MD5Init(&ctx_file, 0);
        MD5Update(&ctx_file, &buffer[0], (unsigned int)size_read.offset);
        Md5Digest result;
        MD5Final(&result.hash[0], &ctx_file);
        hash_set.push_back(result);
    }
    delete pTemp;
    MD5Final(hash, &ctx);
}