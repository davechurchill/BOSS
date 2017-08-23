#include "BOSS.h"
#include "ActionType.h"
#include "ActionTypeData.h"

namespace BOSS
{
    bool isInit = false;

    void Init(const std::string & filename)
    {
        if (!isInit)
        {
            std::cout << "Initializing BOSS...\n";

            ActionTypeData::Init(filename);
            ActionTypes::Init();
            isInit = true;
        }
    }

}