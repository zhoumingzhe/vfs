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

void GenerateHash(const std::string & name, FILE* hash_file)
{
    MD5Context ctx;
    MD5Init(&ctx, 0);

    std::string hashset_name(name + ".hs");
    FILE* hashset_file = fopen(hashset_name.c_str(), "w");

    IFile* pTemp = OpenDiskFile(name.c_str(), IFile::O_ReadOnly);
    offset_type length = pTemp->GetSize();

    fprintf(hashset_file, "%d\n", length.offset);
    std::vector<unsigned char> buffer((size_t)block_size.offset);
    for(offset_type i(0); i<length; i = i + block_size)
    {
        offset_type size_read = pTemp->Read(&buffer[0], block_size);
        assert(size_read.offset>0);
        MD5Update(&ctx, &buffer[0], (unsigned int)size_read.offset);


        MD5Context ctx_file;
        MD5Init(&ctx_file, 0);
        MD5Update(&ctx_file, &buffer[0], (unsigned int)size_read.offset);
        unsigned char result[16];
        MD5Final(result, &ctx_file);


        char out[33];
        HashToString(result, out);
        fprintf(hashset_file, "%s\n", out);
    }
    long l = ftell(hashset_file);
    fclose(hashset_file);
    delete pTemp;
    unsigned char result[16];
    MD5Final(result, &ctx);

    char out[33];
    HashToString(result, out);

    fprintf(hash_file, "%s %ld %s\n", name.c_str(), l, out);

    if(l>max_hs_size.offset)
    {
        GenerateHash(hashset_name, hash_file);
    }
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
        GenerateHash(*it, hash_file);
    }

    fclose(hash_file);
    return 0;
}