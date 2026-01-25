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
#include <string>

#include "moba/cs2cancommand.h"
#include "moba/cs2utils.h"

void Monitor::appendAction(const std::string &action, const std::string &message) {
    std::lock_guard l{m};

    std::cerr << moba::LogLevel::NOTICE << "[" << action << "] " << message << std::endl;
}

void Monitor::appendAction(const moba::LogLevel level, const std::string &action) {
    std::lock_guard l{m};

    std::cerr << level << action << std::endl;
}

void Monitor::printException(const std::string &where, const std::string &what) {
    std::lock_guard l{m};
    std::cerr << moba::LogLevel::CRITICAL << where << " " << what << std::endl;
}

void Monitor::printCS2CanCommand(const CS2CanCommand &data) {
    std::lock_guard l{m};
    std::cerr << moba::LogLevel::NOTICE << getCommandName(data.getCanCommand()) << " [" << data << "]" << std::endl;
}

void Monitor::feedbackContactTriggered(const std::uint16_t module, const std::uint16_t contact, const std::uint16_t time, const bool active) {
    std::lock_guard l{m};

    std::cerr <<
        moba::LogLevel::NOTICE <<
        "Feedback module [" <<
        std::setw(4) << std::setfill('0') << module << ":" <<
        std::setw(4) << std::setfill('0') << contact << "] time " <<
        std::setw(5) << std::setfill(' ') << time << " ms " <<
        (active ? "[ on]" : "[off]")  << std::endl;

}

void Monitor::locCommandsTriggered(const std::string& cmd, const std::uint32_t addr, const int value) {
    std::lock_guard l{m};

    std::cerr <<
        moba::LogLevel::NOTICE <<
        cmd << " [" <<
        std::setw(4) << std::setfill('0') << addr << ":" <<
        std::setw(4) << std::setfill('0') << value << "]" << std::endl;
}

void Monitor::printStatus(const std::string &status) {
    std::cerr << moba::LogLevel::NOTICE << "Status switched to <" << status << ">" << std::endl;
}
