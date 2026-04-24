#include "FileTools.h"
#include <iostream>
#include <filesystem>

using namespace BOSS;

void FileTools::MakeDirectory(const std::string & dir)
{
    if (dir.empty())
    {
        return;
    }

    std::filesystem::create_directories(dir);
}
