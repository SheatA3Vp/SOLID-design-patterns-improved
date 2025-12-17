#include "desman.hpp"
#include "battle_visitor.hpp"

Desman::Desman(const std::string& name, double x, double y)
    : NPC(name, x, y, "Desman", 5.0, 20.0) {}

void Desman::accept(Visitor& visitor) {
    visitor.visitDesman(*this);
}