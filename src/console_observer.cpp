#include "console_observer.hpp"
#include <iostream>

void ConsoleObserver::onKill(const std::string& killer, const std::string& victim) {
    std::cout << killer << " killed " << victim << std::endl;
}