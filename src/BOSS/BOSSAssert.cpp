#include "BOSSAssert.h"
#include "BOSSException.h"
#include <iomanip>
#include <ctime>

using namespace BOSS;

namespace BOSS
{
namespace Assert
{
    const std::string CurrentDateTime() 
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
        return ss.str();
    }

    void ReportFailure(const GameState * state, const char * condition, const char * file, int line, const char * msg, ...)
    {
        std::cerr << "\nAssertion thrown!\n";

        char messageBuffer[1024] = "";
        if (msg != nullptr)
        {
            va_list args;
            va_start(args, msg);
            vsprintf(messageBuffer, msg, args);
            //vsnprintf_s(messageBuffer, 1024, msg, args);
            va_end(args);
        }

        std::stringstream ss;
        ss << std::endl;
        ss << "!Assert:   " << condition << std::endl;
        ss << "File:      " << file << std::endl;
        ss << "Message:   " << messageBuffer << std::endl;
        ss << "Line:      " << line << std::endl;
        ss << "Time:      " << CurrentDateTime() << std::endl;
                
        #if !defined(EMSCRIPTEN)
            std::cerr << ss.str() << "\n"; 
            getchar();
            exit(0);
        #else
            printf("BOSS Exception Thrown:\n %s\n", ss.str().c_str());
            throw BOSSException(ss.str());
        #endif

            
    }
}
}

