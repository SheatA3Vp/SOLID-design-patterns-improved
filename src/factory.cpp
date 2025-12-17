#include "factory.hpp"
#include "bear.hpp"
#include "heron.hpp"
#include "desman.hpp"
#include <fstream>
#include <sstream>

std::unique_ptr<NPC> NPCFactory::createNPC(const std::string& type, const std::string& name, double x, double y) {
    if (type == "Bear") {
        return std::make_unique<Bear>(name, x, y);
    } else if (type == "Heron") {
        return std::make_unique<Heron>(name, x, y);
    } else if (type == "Desman") {
        return std::make_unique<Desman>(name, x, y);
    }
    return nullptr;
}

std::vector<std::unique_ptr<NPC>> NPCFactory::loadFromFile(const std::string& filename) {
    std::vector<std::unique_ptr<NPC>> npcs;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type, name;
        double x, y;
        if (iss >> type >> name >> x >> y) {
            auto npc = createNPC(type, name, x, y);
            if (npc) {
                npcs.push_back(std::move(npc));
            }
        }
    }
    return npcs;
}