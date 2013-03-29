#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <assert.h>
#include <windows.h>
#include "../VFS/GUtMd5.h"
#include "../VFS/IFile.h"

void ScanFiles(const std::string& dir, std::set<std::string>& names)
{
    WIN32_FIND_DATAA finddata;
    std::string searchstring = dir + "/*";
    HANDLE hfine = FindFirstFileA(searchstring.c_str(), &finddata);
    if (hfine != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (finddata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            {
                if(strcmp(finddata.cFileName, ".svn")!=0&&
                    strcmp(finddata.cFileName, ".")!=0&&
                    strcmp(finddata.cFileName, "..")!=0)
                    ScanFiles(dir + "/" + finddata.cFileName, names);
                continue;
            }
            if(strstr(finddata.cFileName, ".hs")==0)
            {
                std::string name = dir;
                name += "/";
                name += finddata.cFileName;
                names.insert(name);
            }

        }
        while (FindNextFileA(hfine, &finddata));
    }
    FindClose(hfine);
}
struct Md5Digest
{
    unsigned char hash[16];
};

const offset_type block_size(4*1024);
const offset_type max_hs_size(4*1024);

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
int main(int argc, char** argv)
{
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    std::ifstream except_list("exceptionlist.txt");
    std::set<std::string> exception_files;

    std::string filename;
    while(except_list>>filename)
    {
        assert(exception_files.find(filename)==exception_files.end());
        exception_files.insert(filename);
    }

    std::set<std::string> names;
    ScanFiles(".", names);

    for(std::set<std::string>::iterator it = exception_files.begin();
        it != exception_files.end(); ++it)
    {
        std::set<std::string>::iterator it_remove = names.find(*it);
        if(it_remove != names.end())
        {
            names.erase(it_remove);
        }
    }

    FILE* hash_file = fopen("hash.txt", "w");
    for(std::set<std::string>::iterator it = names.begin();
        it != names.end(); ++it)
    {
        unsigned char hash[16];
        std::vector<Md5Digest> hset;
        std::string name = *it;
        do
        {
            offset_type l;
            GenerateHash(name, hash, hset, l);

            char out[33];
            HashToString(hash, out);
            fprintf(hash_file, "%s %lld %s\n", name.c_str(), l.offset, out);

            if(hset.size()>32)
            {
                std::string hashset_name(name + ".hs");
                FILE* hashset_file = fopen(hashset_name.c_str(), "w");
                fprintf(hashset_file, "%lld\n", l.offset);

                for(std::vector<Md5Digest>::iterator iter = hset.begin();
                    iter != hset.end() ; ++iter)
                {
                    char out[33];
                    HashToString(iter->hash, out);
                    fprintf(hashset_file, "%s\n", out);
                }
                fclose(hashset_file);
                name = hashset_name;
            }
        }while(hset.size()>32);
    }

    fclose(hash_file);
    return 0;
}