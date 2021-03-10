#include "BOSS.h"
#include <chrono>
#include <thread>

#include <SFML/Graphics.hpp>
#include <sstream>

using namespace BOSS;

int windowWidth = 1440;
int windowHeight = 960;
sf::Font font;
sf::Text text;

std::string getBuildOrderString(std::vector<std::string>& bo, size_t index)
{
    std::stringstream ss;

    ss << "\nBuild Order\n";
    ss << "-------------\n";
    for (size_t i = 0; i < bo.size(); i++)
    {
        ss << (i != index ? "  " : "> ");
        ss << bo[i] << "\n";
    }

    return ss.str();
}

GameState GetProtossStartState()
{
    GameState state;
    state.addUnit(ActionType("Nexus"));
    state.addUnit(ActionType("Probe"));
    state.addUnit(ActionType("Probe"));
    state.addUnit(ActionType("Probe"));
    state.addUnit(ActionType("Probe"));
    state.setMinerals(50);
    return state;
}

GameState GetTerranStartState()
{
    GameState state;
    state.addUnit(ActionType("CommandCenter"));
    state.addUnit(ActionType("SCV"));
    state.addUnit(ActionType("SCV"));
    state.addUnit(ActionType("SCV"));
    state.addUnit(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SupplyDepot"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("Barracks"));
    state.doAction(ActionType("Refinery"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("SCV"));
    state.doAction(ActionType("Factory"));
    state.doAction(ActionType("Factory"));
    state.doAction(ActionType("MachineShop"));
    state.doAction(ActionType("MachineShop"));
    state.setMinerals(300);
    state.setGas(300);
    return state;
}

void test()
{
    font.loadFromFile("fonts/cour.ttf");
    text.setFillColor(sf::Color(255, 255, 255));
    text.setFont(font);
    text.setCharacterSize(16);

    GameState state = GetTerranStartState();

    std::vector<std::string> bos =
    { "Probe",  "Pylon", "Probe", "Probe", "Gateway", "Probe",
    "Assimilator", "Probe", "Probe", "CyberneticsCore", "Probe", "Pylon", "Probe", "Gateway",
    "Dragoon", "Gateway", "Dragoon", "Dragoon", "Probe", "Gateway", "Pylon", "Probe", "Dragoon", "Dragoon", "Dragoon" };

    bos = { "SiegeTank", "SiegeTank" };

    std::vector<ActionType> buildOrder;

    for (auto & str : bos)
    {
        buildOrder.push_back(ActionType(str));
    }

    size_t buildOrderIndex = 0;
    bool progress = true;

    std::vector<GameState> states;
    std::vector<size_t> buildOrderIndices;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "BOSS Visualization");

    // main loop - continues for each frame while window is open
    while (window.isOpen())
    {
        
        if (progress || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            state.fastForward(state.getCurrentFrame() + 1);
            states.push_back(state);
            buildOrderIndices.push_back(buildOrderIndex);
        }
        else if (!progress && sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            if (states.size() > 1)
            {
                state = states.back();
                states.pop_back();

                buildOrderIndex = buildOrderIndices.back();
                buildOrderIndices.pop_back();
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

        window.clear();

        text.setPosition(sf::Vector2f(10, 10));
        text.setString(state.toStringAllUnits()); 
        window.draw(text);

        text.setPosition(sf::Vector2f(720, 10));
        text.setString(state.toStringResources());
        window.draw(text);

        text.setPosition(sf::Vector2f(720, 300));
        text.setString(state.toStringCompleted());
        window.draw(text);

        text.setPosition(sf::Vector2f(720, 400));
        text.setString(state.toStringInProgress());
        window.draw(text);

        text.setPosition(sf::Vector2f(720, 500));
        text.setString(state.toStringLegalActions());
        window.draw(text);

        text.setPosition(sf::Vector2f(1150, 10));
        text.setString(getBuildOrderString(bos, buildOrderIndex));
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

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

int main(int argc, char *argv[])
{
    // Initialize all the BOSS internal data
    BOSS::Init("config/BWData.json");

    for (auto t : ActionTypes::GetAllActionTypes())
    {
        std::cout << t.getID() << " " << t.getName() << " " << t.whatBuildsAddon().getName() << "\n";
    }

    test();
    
    return 0;
}
