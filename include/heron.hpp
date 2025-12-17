#pragma once
#include "npc.hpp"

// Выпь
class Heron : public NPC {
public:
    Heron(const std::string& name, double x, double y);
    void accept(Visitor& visitor) override;
};