#pragma once
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

#include "npc.hpp"
#include "observer.hpp"

class Dungeon {
public:
    Dungeon();

    void addNPC(std::unique_ptr<NPC> npc);
    void spawnRandomNPCs(std::size_t count);
    void saveToFile(const std::string& filename) const;
    std::size_t loadFromFile(const std::string& filename);
    void print() const;
    void printMap() const;
    void battle(double range, std::vector<std::shared_ptr<Observer>>& observers);

    std::thread startMovementThread(std::atomic<bool>& stopFlag);
    std::thread startBattleThread(std::atomic<bool>& stopFlag, std::vector<std::shared_ptr<Observer>> observers);
    void notifyBattleThread();

    std::vector<std::string> survivors() const;

private:
    struct FightTask {
        NPC* attacker;
        NPC* defender;
    };

    std::vector<std::unique_ptr<NPC>> npcs_;
    mutable std::shared_mutex npcsMutex_;

    std::queue<FightTask> fights_;
    std::mutex queueMutex_;
    std::condition_variable queueCv_;

    mutable std::mutex coutMutex_;
    std::mt19937 rng_;

    void movementLoop(std::atomic<bool>& stopFlag);
    void battleLoop(std::atomic<bool>& stopFlag, std::vector<std::shared_ptr<Observer>> observers);
    void enqueueFight(NPC* attacker, NPC* defender);
    bool tryPopFight(FightTask& task);
    void randomStep(NPC& npc, std::mt19937& rng);
};