#pragma once
#include "visitor.hpp"

class BattleVisitor : public Visitor {
public:
    BattleVisitor(NPC& other, std::vector<std::shared_ptr<Observer>>& observers, std::unordered_set<std::string>& killed, int attackRoll, int defenseRoll);

    void visitBear(class Bear& bear) override;
    void visitHeron(class Heron& heron) override;
    void visitDesman(class Desman& desman) override;

    bool didKill() const { return killHappened_; }
};