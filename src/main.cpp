
#define _CRT_NO_VA_START_VALIDATION

#include "BOSS.h"
#include "GameState.h"
#include <chrono>
#include <thread>
#include "CImg/CImg.h"

using namespace BOSS;
using namespace cimg_library;

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

int main(int argc, char *argv[])
{
    HANDLE hStdout;
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    // Initialize all the BOSS internal data
    BOSS::Init("BWData.json");

    GameState state;
    state.addInstance(ActionTypes::GetActionType("Nexus"));
    state.addInstance(ActionTypes::GetActionType("Probe"));
    state.addInstance(ActionTypes::GetActionType("Probe"));
    state.addInstance(ActionTypes::GetActionType("Probe"));
    state.addInstance(ActionTypes::GetActionType("Probe"));

    std::vector<ActionType> buildOrder;
    buildOrder.push_back(ActionTypes::GetActionType("Probe"));
    buildOrder.push_back(ActionTypes::GetActionType("Probe"));
    buildOrder.push_back(ActionTypes::GetActionType("Probe"));
    buildOrder.push_back(ActionTypes::GetActionType("Probe"));
    buildOrder.push_back(ActionTypes::GetActionType("Pylon"));
    buildOrder.push_back(ActionTypes::GetActionType("Probe"));
    buildOrder.push_back(ActionTypes::GetActionType("Probe"));
    buildOrder.push_back(ActionTypes::GetActionType("Gateway"));
    buildOrder.push_back(ActionTypes::GetActionType("Gateway"));
    buildOrder.push_back(ActionTypes::GetActionType("Probe"));
    buildOrder.push_back(ActionTypes::GetActionType("Probe"));
    buildOrder.push_back(ActionTypes::GetActionType("Zealot"));
    buildOrder.push_back(ActionTypes::GetActionType("Zealot"));

    size_t buildOrderIndex = 0;
    bool progress = true;

    CImg<> image;
    CImgList<> font_full = CImgList<>::font(23, false);
    //font_full.remove(0,255);
    unsigned char color[3] = {160, 160, 160};
    //image.draw_text(0, 0, state.toString().c_str(), &color, 0, 1, font_full); 
    CImgDisplay main_disp(image,"Click a point");

    while(true)
    {
        if (progress || GetKey('D')) 
        { 
            state.fastForward(state.getCurrentFrame() + 24); 
            
            CImg<unsigned char> image2;
            image2.draw_text(0, 0, state.toString().c_str(), color, 0, 1, font_full); 
            
            main_disp.display(image2);
            main_disp.resize(image2);
        }
        
        if (GetKey('S') && progress) { progress = false; }
        if (GetKey('A') && !progress) { progress = true; }

        if (buildOrderIndex < buildOrder.size())
        {
            if (state.whenCanBuild(buildOrder[buildOrderIndex]) == state.getCurrentFrame())
            {
                state.doAction(buildOrder[buildOrderIndex]);
                buildOrderIndex++;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }

    return 0;
}
