#include "BOSS.h"
#include "ActionType.h"
#include "ActionTypeData.h"

namespace BOSS
{
    void Init(const std::string & filename)
    {
        static bool isInit = false;
        if (!isInit)
        {
            ActionTypeData::Init(filename);
            ActionTypes::Init();
            isInit = true;
        }
    }
}