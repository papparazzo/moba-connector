/*
 *  Project:    moba-connector
 *
 *  Copyright (C) 2022 Stefan Paproth <pappi-@gmx.de>
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

#include "monitor.h"

#include <iomanip>
#include <chrono>
#include <ctime>

#include <string>

#include <sstream>

namespace {
    // TODO: Put this into moba-lib-common
    std::string getTimeStamp() {
        const auto now = std::chrono::system_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        const auto timer = std::chrono::system_clock::to_time_t(now);

        std::tm bt{};

        localtime_r(&timer, &bt);

        std::stringstream ss;
        ss <<
            std::put_time(&bt, "%Y-%m-%d %H:%M:%S.") << std::right <<
            std::setw(3) << std::setfill('0') << ms.count();
        return ss.str();
    }
}

void Monitor::printBrakeVector(const BrakeVector::Vector &vec) {
    std::lock_guard l{m};

    int y = 5;
    for(auto &[fst, snd]: vec) {
        if(snd == BrakeVector::IGNORE_CONTACT) {
            continue;
        }
        std::stringstream ss;
        ss << "[" << fst.first << ":" << fst.second << "] -> " << snd;
        screen.mvprintw(y++, 80, ss.str());
    }
    while(y < 20) {
        screen.mvprintw(y++, 80, "                                ");
    }

    screen.refresh();
}

void Monitor::appendAction(const moba::LogLevel level, const std::string &action) {
    std::lock_guard l{m};

    std::stringstream ss;
    ss << level << action;

    actions.push_back(ss.str());
    screen.printBuffer(30, 2, actions);
}

void Monitor::printException(const std::string &where, const std::string &what) {
    std::lock_guard l{m};
    screen.printException(where, what);
}

void Monitor::feedbackContactTriggered(const std::uint16_t module, const std::uint16_t contact, const std::uint16_t time, bool active) {
    std::lock_guard l{m};

    std::stringstream ss;
    ss <<
        "Feedback module [" <<
        std::setw(4) << std::setfill('0') << module << ":" <<
        std::setw(4) << std::setfill('0') << contact << "] time " <<
        std::setw(5) << std::setfill(' ') << time << " ms " <<
        (active ? "[ on]" : "[off]");

    appendCanBusAction(ss.str());
}

void Monitor::appendCanBusAction(const std::string &action) {
    std::string msg = getTimeStamp() + " " + action;
    canBusActions.push_back(msg.c_str());
    screen.printBuffer(5, 2, canBusActions);
}

void Monitor::printStatus(SystemHardwareStateChanged::HardwareState status) {
    switch(status) {
        case SystemHardwareStateChanged::HardwareState::ERROR:
            return screen.printStatus("Keine Verbindung zur Hardware", true);

        case SystemHardwareStateChanged::HardwareState::AUTOMATIC:
            return screen.printStatus("automatisch", false);

        case SystemHardwareStateChanged::HardwareState::EMERGENCY_STOP:
            return screen.printStatus("Nothalt ausgelöst", true);

        case SystemHardwareStateChanged::HardwareState::MANUEL:
            return screen.printStatus("manuell", false);

        case SystemHardwareStateChanged::HardwareState::STANDBY:
            return screen.printStatus("Energiesparmodus", false);
    }
}
