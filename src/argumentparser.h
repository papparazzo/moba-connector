/*
 *  Project:    moba-connector
 *
 *  Copyright (C) 2018 Stefan Paproth <pappi-@gmx.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/agpl.txt>.
 *
 */

#pragma once

#include <moba-common/helper.h>
#include "watchdog.h"

class ArgumentParser {
public:
    ArgumentParser() = default;

    [[nodiscard]]
    Watchdog::PingSettings getPingSettings() const {
        return pingSettings;
    }

    [[nodiscard]]
    bool getDebug() const {
        return debug;
    }

    [[nodiscard]]
    bool getSuppressSound() const {
        return suppressSound;
    }

    [[nodiscard]]
    moba::LogLevel getThresholdLogLevel() const {
        return thresholdLevel;
    }

    bool parseArguments(int argc, char *argv[], moba::AppData &appData, CS2ContactData &cs2ContactData) ;

    static void printHelp(const std::string &appName, const CS2ContactData &cs2ContactData, const Watchdog::PingSettings &pingSettings);

private:
    Watchdog::PingSettings pingSettings;
    bool debug{false};
    bool suppressSound{false};

    moba::LogLevel thresholdLevel{moba::LogLevel::NOTICE};

    static std::chrono::milliseconds parseMilliseconds(const std::string &s);

    [[nodiscard]]
    static moba::LogLevel logLevelFromString(const std::string& s);
};


