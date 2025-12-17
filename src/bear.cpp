#include "bear.hpp"
#include "battle_visitor.hpp"

Bear::Bear(const std::string& name, double x, double y)
    : NPC(name, x, y, "Bear", 5.0, 10.0) {}

void Bear::accept(Visitor& visitor) {
    visitor.visitBear(*this);
}