#include "FileTools.h"
#include <iostream>
#include <filesystem>

using namespace BOSS;

void FileTools::MakeDirectory(const std::string & dir)
{
    std::filesystem::create_directory(dir);
}