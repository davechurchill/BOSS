#pragma once

#include "Common.h"
#include "ActionSet.h"
#include "ActionType.h"

#include <algorithm>
#include <vector>

namespace BOSS
{

enum class ActionOrderingType
{
    None,
    LowMineralPrice,
    HighMineralPrice,
    LowGasPrice,
    HighGasPrice,
    LowTotalPrice,
    HighTotalPrice,
    LowestTech,
    HighestTech,
    NaiveBuild,
    ShortestBuildTime,
    LongestBuildTime,
    HighestSupplyCost,
    LowestSupplyCost,
    GoalFirst,
    LeastBuilt,
    MostBuilt
};

namespace ActionOrdering
{

inline std::string GetOrderingName(ActionOrderingType ordering)
{
    switch (ordering)
    {
        case ActionOrderingType::LowMineralPrice:   return "LowMineralPrice";
        case ActionOrderingType::HighMineralPrice:  return "HighMineralPrice";
        case ActionOrderingType::LowGasPrice:       return "LowGasPrice";
        case ActionOrderingType::HighGasPrice:      return "HighGasPrice";
        case ActionOrderingType::LowTotalPrice:     return "LowTotalPrice";
        case ActionOrderingType::HighTotalPrice:    return "HighTotalPrice";
        case ActionOrderingType::LowestTech:        return "LowestTech";
        case ActionOrderingType::HighestTech:       return "HighestTech";
        case ActionOrderingType::NaiveBuild:        return "NaiveBuild";
        case ActionOrderingType::ShortestBuildTime: return "ShortestBuildTime";
        case ActionOrderingType::LongestBuildTime:  return "LongestBuildTime";
        case ActionOrderingType::HighestSupplyCost: return "HighestSupplyCost";
        case ActionOrderingType::LowestSupplyCost:  return "LowestSupplyCost";
        case ActionOrderingType::GoalFirst:         return "GoalFirst";
        case ActionOrderingType::LeastBuilt:        return "LeastBuilt";
        case ActionOrderingType::MostBuilt:         return "MostBuilt";
        default:                                    return "None";
    }
}

inline ActionOrderingType GetOrderingType(const std::string & name)
{
    if (name == "None")              return ActionOrderingType::None;
    if (name == "LowMineralPrice")   return ActionOrderingType::LowMineralPrice;
    if (name == "HighMineralPrice")  return ActionOrderingType::HighMineralPrice;
    if (name == "LowGasPrice")       return ActionOrderingType::LowGasPrice;
    if (name == "HighGasPrice")      return ActionOrderingType::HighGasPrice;
    if (name == "LowTotalPrice")     return ActionOrderingType::LowTotalPrice;
    if (name == "HighTotalPrice")    return ActionOrderingType::HighTotalPrice;
    if (name == "LowestTech")        return ActionOrderingType::LowestTech;
    if (name == "HighestTech")       return ActionOrderingType::HighestTech;
    if (name == "NaiveBuild")        return ActionOrderingType::NaiveBuild;
    if (name == "ShortestBuildTime") return ActionOrderingType::ShortestBuildTime;
    if (name == "LongestBuildTime")  return ActionOrderingType::LongestBuildTime;
    if (name == "HighestSupplyCost") return ActionOrderingType::HighestSupplyCost;
    if (name == "LowestSupplyCost")  return ActionOrderingType::LowestSupplyCost;
    if (name == "GoalFirst")         return ActionOrderingType::GoalFirst;
    if (name == "LeastBuilt")        return ActionOrderingType::LeastBuilt;
    if (name == "MostBuilt")         return ActionOrderingType::MostBuilt;
    BOSS_ASSERT(false, "Unknown action ordering type: %s", name.c_str());
    return ActionOrderingType::None;
}

// ranks: precomputed per-ordering-type data (NaiveBuild ranks, GoalFirst ranks, LeastBuilt/MostBuilt counts)
inline void ApplyOrdering(ActionSet & actions, ActionOrderingType ordering, const std::vector<size_t> & ranks = {})
{
    if (ordering == ActionOrderingType::None || actions.size() <= 1)
    {
        return;
    }

    std::vector<ActionType> vec;
    vec.reserve(actions.size());
    for (size_t i = 0; i < actions.size(); ++i)
    {
        vec.push_back(actions[i]);
    }

    switch (ordering)
    {
        case ActionOrderingType::LowMineralPrice:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return a.mineralPrice() < b.mineralPrice();
            });
            break;
        case ActionOrderingType::HighMineralPrice:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return a.mineralPrice() > b.mineralPrice();
            });
            break;
        case ActionOrderingType::LowGasPrice:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return a.gasPrice() < b.gasPrice();
            });
            break;
        case ActionOrderingType::HighGasPrice:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return a.gasPrice() > b.gasPrice();
            });
            break;
        case ActionOrderingType::LowTotalPrice:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return (a.mineralPrice() + a.gasPrice()) < (b.mineralPrice() + b.gasPrice());
            });
            break;
        case ActionOrderingType::HighTotalPrice:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return (a.mineralPrice() + a.gasPrice()) > (b.mineralPrice() + b.gasPrice());
            });
            break;
        case ActionOrderingType::LowestTech:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return a.getRecursivePrerequisiteActionCount().size() < b.getRecursivePrerequisiteActionCount().size();
            });
            break;
        case ActionOrderingType::HighestTech:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return a.getRecursivePrerequisiteActionCount().size() > b.getRecursivePrerequisiteActionCount().size();
            });
            break;
        case ActionOrderingType::ShortestBuildTime:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return a.buildTime() < b.buildTime();
            });
            break;
        case ActionOrderingType::LongestBuildTime:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return a.buildTime() > b.buildTime();
            });
            break;
        case ActionOrderingType::HighestSupplyCost:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return a.supplyCost() > b.supplyCost();
            });
            break;
        case ActionOrderingType::LowestSupplyCost:
            std::stable_sort(vec.begin(), vec.end(), [](const ActionType & a, const ActionType & b) {
                return a.supplyCost() < b.supplyCost();
            });
            break;
        case ActionOrderingType::NaiveBuild:
        case ActionOrderingType::GoalFirst:
            std::stable_sort(vec.begin(), vec.end(), [&ranks](const ActionType & a, const ActionType & b) {
                const size_t rankA = (a.getID() < ranks.size()) ? ranks[a.getID()] : SIZE_MAX;
                const size_t rankB = (b.getID() < ranks.size()) ? ranks[b.getID()] : SIZE_MAX;
                return rankA < rankB;
            });
            break;
        case ActionOrderingType::LeastBuilt:
            std::stable_sort(vec.begin(), vec.end(), [&ranks](const ActionType & a, const ActionType & b) {
                const size_t countA = (a.getID() < ranks.size()) ? ranks[a.getID()] : 0;
                const size_t countB = (b.getID() < ranks.size()) ? ranks[b.getID()] : 0;
                return countA < countB;
            });
            break;
        case ActionOrderingType::MostBuilt:
            std::stable_sort(vec.begin(), vec.end(), [&ranks](const ActionType & a, const ActionType & b) {
                const size_t countA = (a.getID() < ranks.size()) ? ranks[a.getID()] : 0;
                const size_t countB = (b.getID() < ranks.size()) ? ranks[b.getID()] : 0;
                return countA > countB;
            });
            break;
        default:
            break;
    }

    actions.clear();
    for (const ActionType & action : vec)
    {
        actions.add(action);
    }
}

} // namespace ActionOrdering
} // namespace BOSS
