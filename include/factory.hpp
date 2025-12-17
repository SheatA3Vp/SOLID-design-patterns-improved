#pragma once
#include <memory>
#include <string>
#include <vector>

class NPC;

class NPCFactory {
public:
    static std::unique_ptr<NPC> createNPC(const std::string& type, const std::string& name, double x, double y);
    static std::vector<std::unique_ptr<NPC>> loadFromFile(const std::string& filename);
};