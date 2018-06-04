#include "FileTools.h"

#include <sys/types.h>
#include <sys/stat.h> 

#ifdef WIN32   // Windows system specific
    #include <windows.h>
    #include <direct.h>
#else          // Unix based system specific
    #include <sys/time.h>
#endif

using namespace BOSS;

void FileTools::MakeDirectory(const std::string & dir)
{
    int nError = 0;
#ifdef WIN32
    nError = _mkdir(dir.c_str()); // can be used on Windows
#else 
    mode_t nMode = 0733; // UNIX style permissions
    nError = mkdir(sPath.c_str(), nMode); // can be used on non-Windows
#endif
    if (nError != 0) {
        // handle your error here
    }
}