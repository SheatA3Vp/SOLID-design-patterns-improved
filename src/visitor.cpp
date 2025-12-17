#include "visitor.hpp"

Visitor::Visitor(NPC& other, std::vector<std::shared_ptr<Observer>>& observers, std::unordered_set<std::string>& killed, int attackRoll, int defenseRoll)
    : other_(other), observers_(observers), killed_(killed), attackRoll_(attackRoll), defenseRoll_(defenseRoll) {}