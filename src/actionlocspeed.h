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

#include <utility>
#include <memory>

#include "actionabstract.h"

#include "moba/cs2utils.h"
#include "moba/cs2writer.h"

struct ActionLocSpeed final: ActionAbstract {

    ActionLocSpeed(MonitorPtr monitor, CS2WriterPtr cs2writer, const std::uint32_t localId, const std::uint16_t speed):
    ActionAbstract{std::move(monitor)}, cs2writer{std::move(cs2writer)}, localId{localId}, speed{speed} {
        if (localId == 0) {
            throw std::invalid_argument("given localId is invalid");
        }
    }

    void operator()() override {
        monitor->appendAction("ActionLocSpeed", "set speed for localId <" + std::to_string(localId) +  "> to " + std::to_string(speed));
        cs2writer->send(::setLocSpeed(localId, speed));
    }

private:
    CS2WriterPtr  cs2writer;
    std::uint32_t localId;
    std::uint16_t speed;
};
