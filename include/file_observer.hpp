#pragma once
#include "observer.hpp"
#include <fstream>

class FileObserver : public Observer {
public:
    FileObserver(const std::string& filename);
    void onKill(const std::string& killer, const std::string& victim) override;

private:
    std::ofstream file_;
};