#include "dungeon.hpp"
#include "npc.hpp"
#include "factory.hpp"
#include "battle_visitor.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include <chrono>

namespace {
double randomDelta(double maxStep, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(-maxStep, maxStep);
    return dist(rng);
}
}

Dungeon::Dungeon() : rng_(std::random_device{}()) {}

void Dungeon::addNPC(std::unique_ptr<NPC> npc) {
    std::lock_guard<std::shared_mutex> lock(npcsMutex_);
    npcs_.push_back(std::move(npc));
}

void Dungeon::spawnRandomNPCs(std::size_t count) {
    std::vector<std::string> types = {"Bear", "Heron", "Desman"};
    std::uniform_int_distribution<int> typeDist(0, static_cast<int>(types.size()) - 1);
    std::uniform_real_distribution<double> posDist(NPC::MAP_MIN, NPC::MAP_MAX);

    for (std::size_t i = 0; i < count; ++i) {
        std::string type = types[typeDist(rng_)];
        std::string name = type + std::to_string(i + 1);
        auto npc = NPCFactory::createNPC(type, name, posDist(rng_), posDist(rng_));
        if (npc) {
            addNPC(std::move(npc));
        }
    }
}

void Dungeon::saveToFile(const std::string& filename) const {
    std::shared_lock<std::shared_mutex> lock(npcsMutex_);
    std::ofstream file(filename);
    for (const auto& npc : npcs_) {
        file << npc->getType() << " " << npc->getName() << " " << npc->getX() << " " << npc->getY() << std::endl;
    }
}

std::size_t Dungeon::loadFromFile(const std::string& filename) {
    auto loaded = NPCFactory::loadFromFile(filename);
    std::lock_guard<std::shared_mutex> lock(npcsMutex_);
    npcs_ = std::move(loaded);
    return npcs_.size();
}

void Dungeon::print() const {
    std::shared_lock<std::shared_mutex> lock(npcsMutex_);
    for (const auto& npc : npcs_) {
        std::cout << npc->getType() << " " << npc->getName() << " at (" << npc->getX() << ", " << npc->getY() << ")" << std::endl;
    }
}

void Dungeon::printMap() const {
    std::shared_lock<std::shared_mutex> dataLock(npcsMutex_);
    std::lock_guard<std::mutex> outLock(coutMutex_);

    constexpr int GRID = 50;
    constexpr double cellSize = (NPC::MAP_MAX - NPC::MAP_MIN) / GRID;

    std::vector<std::vector<char>> grid(GRID, std::vector<char>(GRID, ' '));

    for (const auto& npc : npcs_) {
        if (!npc->isAlive()) continue;
        int gx = static_cast<int>((npc->getX() - NPC::MAP_MIN) / cellSize);
        int gy = static_cast<int>((npc->getY() - NPC::MAP_MIN) / cellSize);
        gx = std::clamp(gx, 0, GRID - 1);
        gy = std::clamp(gy, 0, GRID - 1);

        char mark = npc->getType().empty() ? '?' : static_cast<char>(std::toupper(npc->getType().front()));
        if (grid[gy][gx] != ' ' && grid[gy][gx] != mark) {
            grid[gy][gx] = '*';
        } else {
            grid[gy][gx] = mark;
        }
    }

    std::cout << "=== Map 50x50 ===" << std::endl;
    for (int y = 0; y < GRID; ++y) {
        for (int x = 0; x < GRID; ++x) {
            char mark = grid[y][x];
            if (mark == ' ') mark = ' ';
            std::cout << '[' << mark << ']';
        }
        std::cout << '\n';
    }
    std::cout << "================" << std::endl;
}

void Dungeon::battle(double range, std::vector<std::shared_ptr<Observer>>& observers) {
    std::unordered_set<std::string> killed;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dice(1, 6);

    std::lock_guard<std::shared_mutex> lock(npcsMutex_);
    for (size_t i = 0; i < npcs_.size(); ++i) {
        if (!npcs_[i]->isAlive()) continue;
        for (size_t j = i + 1; j < npcs_.size(); ++j) {
            if (!npcs_[j]->isAlive()) continue;
            if (npcs_[i]->distanceTo(*npcs_[j]) <= range) {
                BattleVisitor visitorAB(*npcs_[j], observers, killed, dice(rng), dice(rng));
                npcs_[i]->accept(visitorAB);
                if (visitorAB.didKill()) {
                    npcs_[j]->kill();
                }

                BattleVisitor visitorBA(*npcs_[i], observers, killed, dice(rng), dice(rng));
                npcs_[j]->accept(visitorBA);
                if (visitorBA.didKill()) {
                    npcs_[i]->kill();
                }
            }
        }
    }
}

std::thread Dungeon::startMovementThread(std::atomic<bool>& stopFlag) {
    return std::thread([this, &stopFlag]() { movementLoop(stopFlag); });
}

std::thread Dungeon::startBattleThread(std::atomic<bool>& stopFlag, std::vector<std::shared_ptr<Observer>> observers) {
    return std::thread([this, &stopFlag, observers]() mutable { battleLoop(stopFlag, std::move(observers)); });
}

void Dungeon::notifyBattleThread() {
    queueCv_.notify_all();
}

std::vector<std::string> Dungeon::survivors() const {
    std::vector<std::string> alive;
    std::shared_lock<std::shared_mutex> lock(npcsMutex_);
    for (const auto& npc : npcs_) {
        if (npc->isAlive()) {
            alive.push_back(npc->getName());
        }
    }
    return alive;
}

void Dungeon::movementLoop(std::atomic<bool>& stopFlag) {
    using namespace std::chrono_literals;
    std::mt19937 localRng(std::random_device{}());

    while (!stopFlag.load()) {
        {
            std::unique_lock<std::shared_mutex> lock(npcsMutex_);
            for (auto& npc : npcs_) {
                if (!npc->isAlive()) {
                    continue;
                }
                double dx = randomDelta(npc->getMoveDistance(), localRng);
                double dy = randomDelta(npc->getMoveDistance(), localRng);
                npc->moveBy(dx, dy);
            }

            for (size_t i = 0; i < npcs_.size(); ++i) {
                if (!npcs_[i]->isAlive()) continue;
                for (size_t j = i + 1; j < npcs_.size(); ++j) {
                    if (!npcs_[j]->isAlive()) continue;
                    double distance = npcs_[i]->distanceTo(*npcs_[j]);
                    if (distance <= npcs_[i]->getKillDistance()) {
                        enqueueFight(npcs_[i].get(), npcs_[j].get());
                    }
                    if (distance <= npcs_[j]->getKillDistance()) {
                        enqueueFight(npcs_[j].get(), npcs_[i].get());
                    }
                }
            }
        }

        queueCv_.notify_all();
        std::this_thread::sleep_for(200ms);
    }
    queueCv_.notify_all();
}

void Dungeon::battleLoop(std::atomic<bool>& stopFlag, std::vector<std::shared_ptr<Observer>> observers) {
    std::unordered_set<std::string> killedNames;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dice(1, 6);

    while (true) {
        FightTask task{};
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCv_.wait(lock, [&]() { return !fights_.empty() || stopFlag.load(); });
            if (fights_.empty()) {
                if (stopFlag.load()) {
                    break;
                }
                continue;
            }
            task = fights_.front();
            fights_.pop();
        }

        std::shared_lock<std::shared_mutex> dataLock(npcsMutex_);
        if (task.attacker == nullptr || task.defender == nullptr) {
            continue;
        }

        if (!task.attacker->isAlive() || !task.defender->isAlive()) {
            continue;
        }

        if (task.attacker->distanceTo(*task.defender) > task.attacker->getKillDistance()) {
            continue;
        }

        BattleVisitor visitor(*task.defender, observers, killedNames, dice(rng), dice(rng));
        task.attacker->accept(visitor);
        if (visitor.didKill()) {
            task.defender->kill();
        }
    }
}

void Dungeon::enqueueFight(NPC* attacker, NPC* defender) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    fights_.push(FightTask{attacker, defender});
    queueCv_.notify_one();
}

bool Dungeon::tryPopFight(FightTask& task) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (fights_.empty()) {
        return false;
    }
    task = fights_.front();
    fights_.pop();
    return true;
}

void Dungeon::randomStep(NPC& npc, std::mt19937& rng) {
    double dx = randomDelta(npc.getMoveDistance(), rng);
    double dy = randomDelta(npc.getMoveDistance(), rng);
    npc.moveBy(dx, dy);
}