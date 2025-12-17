#pragma once
#include <atomic>
#include <string>
#include <stdexcept>

#include <algorithm>
#include <shared_mutex>

class Visitor;

class NPC {
public:
    static constexpr double MAP_MIN = 0.0;
    static constexpr double MAP_MAX = 50.0;

    NPC(const std::string& name, double x, double y, const std::string& type, double moveDistance, double killDistance);
    virtual ~NPC() = default;

    virtual void accept(Visitor& visitor) = 0;

    const std::string& getName() const;
    double getX() const;
    double getY() const;
    const std::string& getType() const;

    double getMoveDistance() const;
    double getKillDistance() const;

    bool isAlive() const;
    void kill();

    void setPosition(double x, double y);
    void moveBy(double dx, double dy);

    double distanceTo(const NPC& other) const;

private:
    std::string name_;
    double x_, y_;
    std::string type_;
    double moveDistance_;
    double killDistance_;
    std::atomic<bool> alive_{true};
    
    void validateCoordinates(double x, double y) const;
};