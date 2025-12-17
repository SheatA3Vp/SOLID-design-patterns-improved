#pragma once
#include "npc.hpp"

class Bear : public NPC {
public:
    Bear(const std::string& name, double x, double y);
    void accept(Visitor& visitor) override;
};