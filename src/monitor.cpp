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
#include <mutex>
#include <string>

#include "moba/cs2cancommand.h"
#include "moba/cs2utils.h"

Monitor::Monitor(
    const moba::LogLevel thresholdLevel, const CS2ContactData &cs2ContactData
): thresholdLevel{thresholdLevel}, cs2ContactData{cs2ContactData} {
    std::cerr << moba::LogLevel::NOTICE << "Starting monitor... " << std::endl;
    std::cerr << moba::LogLevel::NOTICE << cs2ContactData << std::endl;
}

void Monitor::printAction(const std::string &action, const std::string &message) {
    std::scoped_lock l{m};

    print(moba::LogLevel::NOTICE, std::format("[{}] {}", action, message));
}

void Monitor::printMessage(const moba::LogLevel level, const std::string &message) {
    std::scoped_lock l{m};

    print(level, message);
}

void Monitor::printException(const std::string &where, const std::string &what) {
    std::scoped_lock l{m};

    print(moba::LogLevel::CRITICAL, std::format("{} {}", where, what));
}

void Monitor::printCS2CanCommand(const CS2CanCommand &data) {
    std::scoped_lock l{m};

    print(
        moba::LogLevel::TRACE,
        std::format("{} [{}]", getCommandName(data.getCanCommand()), data.getAsString())
    );
}

void Monitor::feedbackContactTriggered(
    const std::uint16_t module, const std::uint16_t contact, const std::uint16_t time, const bool active
    ) {
    std::scoped_lock l{m};

    print(
        moba::LogLevel::NOTICE,
        std::format(
            "Feedback module [{:04}:{:04}] time {:5} ms {}",
            module,
            contact,
            time,
            active ? "[ on]" : "[off]"
        )
    );
}

void Monitor::locCommandsTriggered(const std::string& cmd, const std::uint32_t addr, const int value) {
    std::scoped_lock l{m};

    print(moba::LogLevel::DEBUG, std::format("{} [{:04}:{:04}] ", cmd, addr, value));
}

void Monitor::printStatus(const std::string &status) {
    std::scoped_lock l{m};

    print(moba::LogLevel::NOTICE, std::format("Status switched to <{}>", status));
}

void Monitor::print(const moba::LogLevel level, const std::string &message) const {
    if (level <= thresholdLevel) {
        std::cerr << level << message << std::endl;
    }
}

