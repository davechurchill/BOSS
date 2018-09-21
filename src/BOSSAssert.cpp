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
        ss << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S");
        return ss.str();
    }

    void ReportFailure(const GameState * state, const char * condition, const char * file, int line, const char * msg, ...)
    {
        std::cerr << "Assertion thrown!\n";

		// get the extra parameters
        char messageBuffer[4096] = "";
		sprintf(messageBuffer, msg);
        if (msg != NULL)
        {
			char* arg;
            va_list args;
            va_start(args, msg);
			while (true) {
				// get the argument. assuming all extra arguments are chars
				arg = va_arg(args, char*);

				// there are no more arguments
				if (arg)
					break;

				// put the extra parameters inside of msg
				sprintf(messageBuffer, msg, arg);
			}
            
			va_end(args);
        }	

        std::stringstream ss;
        ss                                      << std::endl;
        ss << "!Assert:   " << condition        << std::endl;
        ss << "File:      " << file             << std::endl;
        ss << "Message:   " << messageBuffer    << std::endl;
        ss << "Line:      " << line             << std::endl;
        
        #if !defined(EMSCRIPTEN)
            std::cerr << ss.str();  
            throw BOSSException(ss.str());
        #else
            printf("BOSS Exception Thrown:\n %s\n", ss.str().c_str());
            throw BOSSException(ss.str());
        #endif
    }
}
}

