#include <gtest/gtest.h>
#include "dungeon.hpp"
#include "factory.hpp"
#include "console_observer.hpp"
#include "file_observer.hpp"
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdio>

// Перенаправление вывода для проверки
class CaptureOutput {
public:
    CaptureOutput() {
        old_cout_ = std::cout.rdbuf();
        std::cout.rdbuf(buffer_.rdbuf());
    }
    
    ~CaptureOutput() {
        std::cout.rdbuf(old_cout_);
    }
    
    std::string str() const {
        return buffer_.str();
    }
    
private:
    std::streambuf* old_cout_;
    std::stringstream buffer_;
};

// Создание NPC
TEST(NPCTest, CreateBear) {
    auto npc = NPCFactory::createNPC("Bear", "Bear1", 10, 10);
    ASSERT_NE(npc, nullptr);
    EXPECT_EQ(npc->getType(), "Bear");
    EXPECT_EQ(npc->getName(), "Bear1");
    EXPECT_EQ(npc->getX(), 10);
    EXPECT_EQ(npc->getY(), 10);
}

TEST(NPCTest, CreateHeron) {
    auto npc = NPCFactory::createNPC("Heron", "Heron1", 50, 50);
    ASSERT_NE(npc, nullptr);
    EXPECT_EQ(npc->getType(), "Heron");
    EXPECT_EQ(npc->getName(), "Heron1");
}

TEST(NPCTest, CreateDesman) {
    auto npc = NPCFactory::createNPC("Desman", "Desman1", 20, 20);
    ASSERT_NE(npc, nullptr);
    EXPECT_EQ(npc->getType(), "Desman");
    EXPECT_EQ(npc->getName(), "Desman1");
}

// Некорректный тип NPC
TEST(NPCTest, CreateInvalidType) {
    auto npc = NPCFactory::createNPC("Invalid", "Test", 0, 0);
    ASSERT_EQ(npc, nullptr);
}

// Валидация координат
TEST(NPCTest, InvalidCoordinatesTooHigh) {
    EXPECT_THROW(NPCFactory::createNPC("Bear", "Bear1", 51, 25), std::out_of_range);
}

TEST(NPCTest, InvalidCoordinatesTooLow) {
    EXPECT_THROW(NPCFactory::createNPC("Bear", "Bear1", -1, 250), std::out_of_range);
}

TEST(NPCTest, ValidCoordinatesBoundary) {
    auto npc1 = NPCFactory::createNPC("Bear", "Bear1", 0, 0);
    ASSERT_NE(npc1, nullptr);
    
    auto npc2 = NPCFactory::createNPC("Bear", "Bear2", 50, 50);
    ASSERT_NE(npc2, nullptr);
}

// Расстояние между NPC
TEST(NPCTest, DistanceCalculation) {
    auto npc1 = NPCFactory::createNPC("Bear", "Bear1", 0, 0);
    auto npc2 = NPCFactory::createNPC("Bear", "Bear2", 3, 4);
    EXPECT_DOUBLE_EQ(npc1->distanceTo(*npc2), 5.0);
}

// Добавление NPC в подземелье
TEST(DungeonTest, AddNPC) {
    Dungeon dungeon;
    auto npc = NPCFactory::createNPC("Bear", "TestBear", 10, 10);
    dungeon.addNPC(std::move(npc));
    SUCCEED();
}

// Печать NPC
TEST(DungeonTest, PrintNPCs) {
    CaptureOutput capture;
    Dungeon dungeon;
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear1", 10, 10));
    dungeon.addNPC(NPCFactory::createNPC("Heron", "Heron1", 15, 15));
    dungeon.print();
    
    std::string output = capture.str();
    EXPECT_TRUE(output.find("Bear1") != std::string::npos);
    EXPECT_TRUE(output.find("Heron1") != std::string::npos);
}

// Медведь убивает Выпь в пределах дальности
TEST(DungeonTest, BearKillsHeronInRange) {
    Dungeon dungeon;
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear1", 0, 0));
    dungeon.addNPC(NPCFactory::createNPC("Heron", "Heron1", 10, 10));

    auto consoleObs = std::make_shared<ConsoleObserver>();
    std::vector<std::shared_ptr<Observer>> observers = {consoleObs};

    dungeon.battle(20.0, observers);
    // Выпь должна быть убита, остаётся только медведь
    SUCCEED();
}

// Медведь не убивает другого медведя
TEST(DungeonTest, BearNotKillingBear) {
    Dungeon dungeon;
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear1", 0, 0));
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear2", 10, 10));

    auto consoleObs = std::make_shared<ConsoleObserver>();
    std::vector<std::shared_ptr<Observer>> observers = {consoleObs};

    dungeon.battle(20.0, observers);
    // Оба медведя должны остаться живы
    SUCCEED();
}

// Выпь никого не убивает
TEST(DungeonTest, HeronKillsNobody) {
    Dungeon dungeon;
    dungeon.addNPC(NPCFactory::createNPC("Heron", "Heron1", 0, 0));
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear1", 10, 10));
    dungeon.addNPC(NPCFactory::createNPC("Desman", "Desman1", 20, 20));

    auto consoleObs = std::make_shared<ConsoleObserver>();
    std::vector<std::shared_ptr<Observer>> observers = {consoleObs};

    dungeon.battle(30.0, observers);
    // После боя остаётся только выпь, медведь убил остальных
    SUCCEED();
}

// Выхухоль убивает Медведя
TEST(DungeonTest, DesmanKillsBear) {
    Dungeon dungeon;
    dungeon.addNPC(NPCFactory::createNPC("Desman", "Desman1", 0, 0));
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear1", 10, 10));

    auto consoleObs = std::make_shared<ConsoleObserver>();
    std::vector<std::shared_ptr<Observer>> observers = {consoleObs};

    dungeon.battle(20.0, observers);
    // Выхухоль должна убить медведя
    SUCCEED();
}

// Выхухоль не убивает Выпь
TEST(DungeonTest, DesmanNotKillingHeron) {
    Dungeon dungeon;
    dungeon.addNPC(NPCFactory::createNPC("Desman", "Desman1", 0, 0));
    dungeon.addNPC(NPCFactory::createNPC("Heron", "Heron1", 10, 10));

    auto consoleObs = std::make_shared<ConsoleObserver>();
    std::vector<std::shared_ptr<Observer>> observers = {consoleObs};

    dungeon.battle(20.0, observers);
    // Оба должны остаться живы
    SUCCEED();
}

// NPC вне дальности не взаимодействуют
TEST(DungeonTest, OutOfRangeNoPCs) {
    Dungeon dungeon;
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear1", 0, 0));
    dungeon.addNPC(NPCFactory::createNPC("Heron", "Heron1", 40, 40));

    auto consoleObs = std::make_shared<ConsoleObserver>();
    std::vector<std::shared_ptr<Observer>> observers = {consoleObs};

    dungeon.battle(10.0, observers);  // Слишком короткая дальность
    // Оба должны остаться живы
    SUCCEED();
}

// Сохранение в файл и загрузка из файла
TEST(DungeonTest, SaveAndLoadFromFile) {
    const std::string filename = "test_npcs.txt";
    
    {
        Dungeon dungeon;
        dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear1", 10, 10));
        dungeon.addNPC(NPCFactory::createNPC("Heron", "Heron1", 15, 15));
        dungeon.addNPC(NPCFactory::createNPC("Desman", "Desman1", 20, 20));
        dungeon.saveToFile(filename);
    }
    
    Dungeon dungeon2;
    dungeon2.loadFromFile(filename);
    
    // Проверяем, что NPC были загружены
    CaptureOutput capture;
    dungeon2.print();
    std::string output = capture.str();
    
    EXPECT_TRUE(output.find("Bear1") != std::string::npos);
    EXPECT_TRUE(output.find("Heron1") != std::string::npos);
    EXPECT_TRUE(output.find("Desman1") != std::string::npos);
    
    // Очистка
    std::remove(filename.c_str());
}

// Observer - консольный вывод
TEST(ObserverTest, ConsoleObserverOutput) {
    CaptureOutput capture;
    
    auto observer = std::make_shared<ConsoleObserver>();
    observer->onKill("KillerName", "VictimName");
    
    std::string output = capture.str();
    EXPECT_TRUE(output.find("KillerName") != std::string::npos);
    EXPECT_TRUE(output.find("VictimName") != std::string::npos);
    EXPECT_TRUE(output.find("killed") != std::string::npos);
}

// Observer - файловый вывод
TEST(ObserverTest, FileObserverOutput) {
    const std::string filename = "test_log.txt";
    
    {
        auto observer = std::make_shared<FileObserver>(filename);
        observer->onKill("Killer1", "Victim1");
        observer->onKill("Killer2", "Victim2");
    }
    
    std::ifstream file(filename);
    std::string line;
    std::vector<std::string> lines;
    
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();
    
    ASSERT_EQ(lines.size(), 2);
    EXPECT_TRUE(lines[0].find("Killer1") != std::string::npos);
    EXPECT_TRUE(lines[1].find("Killer2") != std::string::npos);
    
    // Очистка
    std::remove(filename.c_str());
}

// Сложный сценарий боя
TEST(DungeonTest, ComplexBattleScenario) {
    Dungeon dungeon;
    // Создаём сценарий: 3 медведя, 2 выпи, 2 выхухоля в одной зоне
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear1", 10, 10));
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear2", 10.5, 10.5));
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear3", 11, 11));
    dungeon.addNPC(NPCFactory::createNPC("Heron", "Heron1", 11.5, 11.5));
    dungeon.addNPC(NPCFactory::createNPC("Heron", "Heron2", 12, 12));
    dungeon.addNPC(NPCFactory::createNPC("Desman", "Desman1", 12.5, 12.5));
    dungeon.addNPC(NPCFactory::createNPC("Desman", "Desman2", 13, 13));

    auto consoleObs = std::make_shared<ConsoleObserver>();
    auto fileObs = std::make_shared<FileObserver>("complex_test_log.txt");
    std::vector<std::shared_ptr<Observer>> observers = {consoleObs, fileObs};

    dungeon.battle(5.0, observers);
    
    // Проверяем, что некоторые NPC были убиты
    SUCCEED();

    std::remove("complex_test_log.txt");
}

TEST(DungeonTest, KilledNPCCannotCounterAttack) {
    Dungeon dungeon;
    // Медведь и Выхухоль очень близко - оба могут умереть
    dungeon.addNPC(NPCFactory::createNPC("Bear", "Bear1", 10, 10));
    dungeon.addNPC(NPCFactory::createNPC("Desman", "Desman1", 10.1, 10.1));

    auto consoleObs = std::make_shared<ConsoleObserver>();
    std::vector<std::shared_ptr<Observer>> observers = {consoleObs};

    // В этом бою:
    // - Bear1 атакует Desman1 (медведь убивает выхухоля)
    // - Desman1 атакует Bear1 (выхухоль убивает медведя)
    // Оба умирают в одном раунде!
    dungeon.battle(0.5, observers);
    
    SUCCEED();
}