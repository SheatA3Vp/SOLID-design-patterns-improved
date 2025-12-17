#include "npc.hpp"
#include <cmath>

NPC::NPC(const std::string& name, double x, double y, const std::string& type, double moveDistance, double killDistance)
    : name_(name), x_(x), y_(y), type_(type), moveDistance_(moveDistance), killDistance_(killDistance) {
    validateCoordinates(x, y);
}

void NPC::validateCoordinates(double x, double y) const {
    if (x < MAP_MIN || x > MAP_MAX || y < MAP_MIN || y > MAP_MAX) {
        throw std::out_of_range("Coordinates must be in range [0, 50]");
    }
}

const std::string& NPC::getName() const { return name_; }
double NPC::getX() const { return x_; }
double NPC::getY() const { return y_; }
const std::string& NPC::getType() const { return type_; }
double NPC::getMoveDistance() const { return moveDistance_; }
double NPC::getKillDistance() const { return killDistance_; }

bool NPC::isAlive() const { return alive_.load(); }

void NPC::kill() { alive_.store(false); }

void NPC::setPosition(double x, double y) {
    validateCoordinates(x, y);
    x_ = x;
    y_ = y;
}

void NPC::moveBy(double dx, double dy) {
    double newX = std::clamp(x_ + dx, MAP_MIN, MAP_MAX);
    double newY = std::clamp(y_ + dy, MAP_MIN, MAP_MAX);
    setPosition(newX, newY);
}

double NPC::distanceTo(const NPC& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    return std::sqrt(dx * dx + dy * dy);
}