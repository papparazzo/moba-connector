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

#pragma once

#include <boost/circular_buffer.hpp>
#include <moba-common/loggerprefix.h>
#include <moba-common/screen.h>

#include "moba/systemmessages.h"

class Monitor final {
public:
    explicit Monitor(const bool debug): screen{"moba-connector", debug} {}
    ~Monitor() = default;

    void appendAction(moba::LogLevel level, const std::string &action);

    void printException(const std::string &where, const std::string &what);

    void feedbackContactTriggered(std::uint16_t module, std::uint16_t contact, std::uint16_t time, bool active);

    void locCommandsTriggered(const std::string& cmd, std::uint32_t addr, int value);

    void printStatus(SystemHardwareStateChanged::HardwareState status) const;

private:
    std::mutex m;
    boost::circular_buffer<std::string> actions{20};
    boost::circular_buffer<std::string> canBusActions{20};
    moba::Screen screen;

    void appendCanBusAction(const std::string &action);
};

using MonitorPtr = std::shared_ptr<Monitor>;
