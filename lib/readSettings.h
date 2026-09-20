//
// Created by Саша on 19.09.2026.
//

#ifndef UTILS_READSETTINGS_H
#define UTILS_READSETTINGS_H
#include <memory>
#include <stdexcept>
#include <vector>
#include <nlohmann/json.hpp>

namespace TrayUtils {
    struct ProgramsRestart {
        std::string path;
        std::string name;
        std::vector<std::string> processName;
    };

    class ReadSettings {
        private:
        std::vector<ProgramsRestart> restartPrograms;

        public:
            ReadSettings();
            ~ReadSettings();
            std::vector<ProgramsRestart>& getPrograms();
    };
}
#endif //UTILS_READSETTINGS_H
