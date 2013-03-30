#ifndef MD5UTLS_H
#define MD5UTLS_H

#include <string>
#include <vector>
#include "../VFS/IFile.h"
struct Md5Digest
{
    unsigned char hash[16];
};

const offset_type block_size(4*1024);
const offset_type max_hs_size(4*1024);

void HashToString(const unsigned char hash[16], char str[16*2+1]);
void StringToHash(const char str[32], unsigned char hash[16]);

void GenerateHash(const std::string & name, unsigned char hash[16],
                  std::vector<Md5Digest> &hash_set, offset_type & length);
#endif