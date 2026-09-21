//
// Created by Саша on 19.09.2026.
//

#ifndef UTILS_READSETTINGS_H
#define UTILS_READSETTINGS_H
#include <memory>
#include <stdexcept>
#include <vector>
#include <nlohmann/json.hpp>

#include "utils.h"

namespace TrayUtils {
    struct ProgramsRestart {
        std::string path;
        std::string name;
        std::vector<std::string> processName;
    };

    ResUpdater thisCanUpgradeAsync();

    class ReadSettings {
        private:
        std::vector<ProgramsRestart> restartPrograms;
        ResUpdater updaterData;
        mutable std::mutex updaterMutex;
        //void startUpdateCheckAsync();
        //callback
        void startUpdateCheckAsync(std::function<void(const ResUpdater&)> onComplete = nullptr);
        void init();

        public:
        ReadSettings();
        ~ReadSettings();
        const std::vector<ProgramsRestart>& getPrograms() const;
        ResUpdater getUpdater() const;
        //callback
        explicit ReadSettings(std::function<void(const ResUpdater&)> onComplete = nullptr);
    };
}
#endif //UTILS_READSETTINGS_H
