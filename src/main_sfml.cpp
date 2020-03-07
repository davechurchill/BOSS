#include "BOSS.h"
#include <chrono>
#include <thread>
#include "CImg/CImg.h"

#include <SFML/Graphics.hpp>

using namespace BOSS;

int windowWidth = 800;
int windowHeight = 800;
sf::Font font;
sf::Text text;

void test()
{
    font.loadFromFile("fonts/cour.ttf");
    text.setFillColor(sf::Color(255, 255, 255));
    text.setFont(font);
    text.setCharacterSize(14);

    GameState state;
    state.addUnit(ActionTypes::GetActionType("Nexus"));
    state.addUnit(ActionTypes::GetActionType("Probe"));
    state.addUnit(ActionTypes::GetActionType("Probe"));
    state.addUnit(ActionTypes::GetActionType("Probe"));
    state.addUnit(ActionTypes::GetActionType("Probe"));
    state.setMinerals(50.0f);

    std::vector<std::string> bos =
    { "Probe", "Probe", "Probe", "Probe", "Pylon", "Pylon", "Pylon", "Probe", "Probe", "Gateway", "Probe",
    "Assimilator", "Probe", "Probe", "CyberneticsCore", "Probe", "Pylon", "Probe", "Gateway",
    "Dragoon", "Gateway", "Dragoon", "Dragoon", "Probe", "Gateway", "Pylon", "Probe", "Dragoon", "Dragoon", "Dragoon" };

    std::vector<ActionType> buildOrder;

    for (auto & str : bos)
    {
        buildOrder.push_back(ActionTypes::GetActionType(str));
    }

    size_t buildOrderIndex = 0;
    bool progress = true;

    std::vector<GameState> states;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "BOSS Visualization");

    // main loop - continues for each frame while window is open
    while (window.isOpen())
    {
        if (progress || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            state.fastForward(state.getCurrentFrame() + 1);
            states.push_back(state);
        }
        else if (!progress && sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            if (states.size() > 1)
            {
                state = states.back();
                states.pop_back();
            }
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            // this event triggers when the window is closed
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && progress) { progress = false; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && !progress) { progress = true; }

        text.setString(state.toString()); 
        window.clear();
        window.draw(text);
        window.display();
        
        if (buildOrderIndex < buildOrder.size())
        {
            if (state.canBuildNow(buildOrder[buildOrderIndex]))
            {
                state.doAction(buildOrder[buildOrderIndex]);
                buildOrderIndex++;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(42));
    }
}

int main(int argc, char *argv[])
{
    // Initialize all the BOSS internal data
    BOSS::Init("BWData.json");

    test();
    
    return 0;
}
