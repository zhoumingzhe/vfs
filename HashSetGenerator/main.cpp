#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <assert.h>
#include <windows.h>
#include "../Md5Utils/Md5Utils.h"

void ScanFiles(const std::string& dir, std::set<std::string>& names)
{
    WIN32_FIND_DATAA finddata;
    std::string searchstring = dir + "*";
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
                    ScanFiles(dir + finddata.cFileName + "/", names);
                continue;
            }
            if(strstr(finddata.cFileName, ".hs")==0)
            {
                std::string name = dir;
                name += finddata.cFileName;
                names.insert(name);
            }

        }
        while (FindNextFileA(hfine, &finddata));
    }
    FindClose(hfine);
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
    ScanFiles("", names);

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