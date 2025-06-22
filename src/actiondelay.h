/*
 *  Project:    moba-connector
 *
 *  Copyright (C) 2025 Stefan Paproth <pappi-@gmx.de>
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

#include <chrono>
#include <thread>
#include <utility>

#include "actionabstract.h"

struct ActionDelay final: ActionAbstract {
    explicit ActionDelay(MonitorPtr monitor, const std::chrono::milliseconds duration): ActionAbstract{std::move(monitor)}, duration{duration} {};

    void operator()() override {
        monitor->appendAction("ActionDelay", "delaying for " + std::to_string(duration.count()) + "ms");
        std::this_thread::sleep_for(duration);
    }

private:
    const std::chrono::milliseconds duration;
};
