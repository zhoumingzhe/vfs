#pragma once
#include "PatchLib.h"
#include <vector>
#include <string>
void GeneratePatch(const std::vector<std::string>& prev_files, const std::string & final_files, const std::string& patch_name);
void ApplyPatch(const std::string& file_name, const std::string& patch_name);