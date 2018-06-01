
#define _CRT_NO_VA_START_VALIDATION

#include "BOSS.h"
#include "GameState.h"
#include "BOSSExperiments.h"
#include <chrono>
#include <thread>
#include "CImg/CImg.h"

using namespace BOSS;
using namespace cimg_library;

#ifdef WIN32

#include <Windows.h>

bool GetKey(char key)
{
    return GetKeyState(key) & 0x8000;
}

void cls( HANDLE hConsole )
{
   COORD coordScreen = { 0, 0 };    // home for the cursor 
   DWORD cCharsWritten;
   CONSOLE_SCREEN_BUFFER_INFO csbi; 
   DWORD dwConSize;

    // Get the number of character cells in the current buffer. 

   if( !GetConsoleScreenBufferInfo( hConsole, &csbi ))
      return;
   dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

   // Fill the entire screen with blanks.

   if( !FillConsoleOutputCharacter( hConsole, (TCHAR) ' ',
      dwConSize, coordScreen, &cCharsWritten ))
      return;

   // Get the current text attribute.

   if( !GetConsoleScreenBufferInfo( hConsole, &csbi ))
      return;

   // Set the buffer's attributes accordingly.

   if( !FillConsoleOutputAttribute( hConsole, csbi.wAttributes,
      dwConSize, coordScreen, &cCharsWritten ))
      return;

   // Put the cursor at its home coordinates.

   SetConsoleCursorPosition( hConsole, coordScreen );
}

void testBuildOrder()
{
    HANDLE hStdout;
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    GameState state;
    state.addUnit(ActionTypes::GetActionType("Nexus"));
    state.addUnit(ActionTypes::GetActionType("Probe"));
    state.addUnit(ActionTypes::GetActionType("Probe"));
    state.addUnit(ActionTypes::GetActionType("Probe"));
    state.addUnit(ActionTypes::GetActionType("Probe"));
    state.setMinerals(50.0f);

    std::vector<std::string> bos = 
        {"Probe", "Probe", "Probe", "Probe", "Pylon", "Pylon", "Pylon", "Probe", "Probe", "Gateway", "Probe",
        "Assimilator", "Probe", "Probe", "CyberneticsCore", "Probe", "Pylon", "Probe", "Gateway", 
        "Dragoon", "Gateway", "Dragoon", "Dragoon", "Probe", "Gateway", "Pylon", "Probe", "Dragoon", "Dragoon", "Dragoon"};

    std::vector<ActionType> buildOrder;

    for (auto & str : bos)
    {
        buildOrder.push_back(ActionTypes::GetActionType(str));
    }

    size_t buildOrderIndex = 0;
    bool progress = true;

    CImg<> image;
    CImgList<> font_full = CImgList<>::font(17, false);
    //font_full.remove(0,255);
    unsigned char color[3] = {222, 222, 222};
    //image.draw_text(0, 0, state.toString().c_str(), &color, 0, 1, font_full); 
    CImgDisplay main_disp(image,"Click a point");

    std::vector<GameState> states;

    while(true)
    {
        if (progress || GetKey('D')) 
        { 
            state.fastForward(state.getCurrentFrame() + 4); 
            states.push_back(state);
        }
        else if (!progress && GetKey('A'))
        {
            if (states.size() > 1)
            {
                state = states.back();
                states.pop_back();
            }
        }



        CImg<unsigned char> image2;
        image2.draw_text(0, 0, state.toString().c_str(), color, 0, 1, font_full); 
        main_disp.display(image2);
        main_disp.resize(image2);
        
        if (GetKey('S') && progress) { progress = false; }
        if (GetKey('W') && !progress) { progress = true; }

        if (buildOrderIndex < buildOrder.size())
        {
            if (state.canBuildNow(buildOrder[buildOrderIndex]))
            {
                state.doAction(buildOrder[buildOrderIndex]);
                buildOrderIndex++;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }

}

#else
void testBuildOrder() {}
#endif

#include "json/json.hpp"

void testjson()
{

}

int main(int argc, char *argv[])
{
    std::vector<size_t> test = { 1, 2, 3, 4, 5, 6 };
    size_t result = std::accumulate(test.begin(), test.end(), 0,
        [](size_t lhs, size_t rhs) { return lhs * 2 + rhs; });
    std::cout << result << std::endl;

    // Initialize all the BOSS internal data
    BOSS::Init("BWData.json");

    // Read in the config parameters that will be used for experiments
    BOSS::BOSSConfig::Instance().ParseParameters("BOSS_Config.txt");

    //BOSS::Experiments::RunExperiments("BOSS_Config.txt");

    testBuildOrder();
    
    std::cout << "Action Types: " << ActionTypes::GetAllActionTypes().size() << "\n";
    std::cout << "BOSSInitComplete\n";

    return 0;
}
