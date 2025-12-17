#include "dungeon.hpp"
#include "factory.hpp"
#include "console_observer.hpp"
#include "file_observer.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

int main(int argc, char** argv) {
    using namespace std::chrono_literals;

    Dungeon dungeon;
    auto consoleObs = std::make_shared<ConsoleObserver>();
    auto fileObs = std::make_shared<FileObserver>("log.txt");
    std::vector<std::shared_ptr<Observer>> observers = {consoleObs, fileObs};

    std::cout << "=== Dungeon Simulation ===" << std::endl;

    const std::string defaultNpcFile = "npcs.txt";
    std::string npcFile;

    if (argc > 1) {
        npcFile = argv[1];
        std::cout << "Loading NPCs from file: " << npcFile << std::endl;
        auto loaded = dungeon.loadFromFile(npcFile);
        if (loaded == 0) {
            std::cerr << "File has no valid NPC entries. Exiting." << std::endl;
            return 1;
        }
    } else {
        npcFile = defaultNpcFile;
        std::cout << "No NPC file provided. Generating random NPCs and saving to: " << npcFile << std::endl;
        dungeon.spawnRandomNPCs(50);
        dungeon.saveToFile(npcFile);
    }

    std::atomic<bool> stopFlag{false};
    std::thread movementThread = dungeon.startMovementThread(stopFlag);
    std::thread battleThread = dungeon.startBattleThread(stopFlag, observers);

    const auto simulationDuration = 30s;
    const auto startTime = std::chrono::steady_clock::now();

    while (std::chrono::steady_clock::now() - startTime < simulationDuration) {
        dungeon.printMap();
        std::this_thread::sleep_for(1s);
    }

    stopFlag.store(true);
    dungeon.notifyBattleThread();

    if (movementThread.joinable()) movementThread.join();
    if (battleThread.joinable()) battleThread.join();

    auto alive = dungeon.survivors();
    std::cout << "\n=== Survivors after 30 seconds ===" << std::endl;
    for (const auto& name : alive) {
        std::cout << name << std::endl;
    }
    std::cout << "Total survivors: " << alive.size() << std::endl;

    return 0;
}
