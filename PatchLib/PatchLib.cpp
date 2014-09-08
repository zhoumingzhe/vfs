#include "PatchLib.h"
#include <set>
#include <map>
#include "../Md5Utils/Md5Utils.h"

void CompareHashSet(std::vector<Md5Digest>& hash_set, std::vector<Md5Digest>& hash_set_final, std::set<size_t> result)
{
    size_t i = 0;
    for (; i < hash_set.size() && i < hash_set_final.size(); ++i)
    {
        if (hash_set[i] != hash_set_final[i])
        {
            result.insert(i);
        }
    }
    while (i++ < hash_set_final.size())
        result.insert(i);
}
void GeneratePatch(const std::vector<std::string>& prev_files, const std::string & final_file, const std::string& patch_name)
{
    unsigned char hash[16];
    std::vector<Md5Digest> hash_set;
    offset_type length = offset_type(0);
    GenerateHash(final_file, hash, hash_set, length);
    std::set<size_t> diff_data;
    for (const auto& name : prev_files)
    {
        unsigned char hash1[16];
        std::vector<Md5Digest> hash_set1;
        offset_type length1 = offset_type(0);
        GenerateHash(name, hash1, hash_set1, length1);
        CompareHashSet(hash_set1, hash_set, diff_data);
    }
    IFile* pFile = OpenDiskFile(patch_name.c_str(), IFile::O_Truncate);
    //final file size, offset_type
    pFile->Write(&length, offset_type(sizeof(length)));

    //final file hash set int
    int size = (int)hash_set.size();
    pFile->Write(&size, offset_type(sizeof(size)));
    pFile->Write(&hash_set[0], offset_type(hash_set.size()*sizeof(hash_set[0])));

    //diff hash size int
    int diff_size = (int)diff_data.size();
    pFile->Write(&diff_size, offset_type(sizeof(diff_size)));
    IFile* pFinal_file = OpenDiskFile(final_file.c_str(), IFile::O_ReadOnly);
    for (auto iter : diff_data)
    {
        //diff position int
        int i = (int)iter;
        pFile->Write(&i, offset_type(sizeof(i)));

        //diff data
        std::vector<unsigned char> buffer(block_size.offset);
        pFinal_file->Seek(offset_type(iter*block_size.offset), IFile::S_Begin);
        offset_type towrite = pFinal_file->Read(&buffer[0], block_size);
        pFile->Write(&buffer[0], towrite);
    }
    delete pFile;
    delete pFinal_file;
}
void ApplyPatch(const std::string& file_name, const std::string& patch_name)
{
    unsigned char hash[16];
    std::vector<Md5Digest> hash_set;
    offset_type length;

    GenerateHash(file_name, hash, hash_set, length);

    std::vector<Md5Digest>hash_set_patch;
    IFile* pFile = OpenDiskFile(patch_name.c_str(), IFile::O_ReadOnly);
    //filesize
    pFile->Read(&length, offset_type(sizeof(length)));

    //hashset
    int hash_set_size;
    pFile->Read(&hash_set_size, offset_type(sizeof(hash_set_size)));
    hash_set_patch.resize(hash_set_size);
    pFile->Read(&hash_set_patch[0], offset_type(hash_set_patch.size()*sizeof(hash_set_patch[0])));

    //diff hash size int
    int diff_size;
    pFile->Read(&diff_size, offset_type(sizeof(diff_size)));
    

    std::map<int, std::vector<unsigned char>> diff_data;
    

    for (int i = 0; i < diff_size; ++i)
    {
        int pos;
        pFile->Read(&pos, offset_type(sizeof(pos)));
        diff_data[i].resize(block_size.offset);
        pFile->Read(&diff_data[i], block_size);
    }
    delete pFile;
    std::set<size_t> diff;
    CompareHashSet(hash_set, hash_set_patch, diff);
    IFile* pFinal_file = OpenDiskFile(file_name.c_str(), IFile::O_Write);
    for (auto i : diff)

    {
        pFinal_file->Seek(offset_type(i*block_size.offset), IFile::S_Begin);
        pFinal_file->Write(&diff_data[i][0], block_size);
    }
    pFinal_file->ReserveSpace(length);
    delete pFinal_file;
}