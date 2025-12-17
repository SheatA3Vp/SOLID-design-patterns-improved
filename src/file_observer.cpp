#include "file_observer.hpp"

FileObserver::FileObserver(const std::string& filename) : file_(filename) {}

void FileObserver::onKill(const std::string& killer, const std::string& victim) {
    if (file_.is_open()) {
        file_ << killer << " killed " << victim << std::endl;
    }
}