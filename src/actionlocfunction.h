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

#include "moba/cs2writer.h"
#include "moba/cs2utils.h"
#include "moba/enumfunction.h"

#include "actionabstract.h"

#include <utility>
#include <chrono>
#include <memory>

using namespace std::literals::chrono_literals;

struct ActionLocFunction final: ActionAbstract {

    ActionLocFunction(MonitorPtr monitor, CS2WriterPtr cs2writer, const std::uint32_t localId, const Function function, const bool active):
    ActionAbstract{std::move(monitor)}, cs2writer{std::move(cs2writer)}, function{function}, localId{localId}, active{active} {
        if (localId == 0) {
            throw std::invalid_argument("given localId is invalid");
        }
    }

    void operator()() override {
        if (function == Function::NONE) {
            return;
        }

        monitor->appendAction(
            "ActionLocFunction",
            "set function " + controllableFunctionEnumToString(function) + " for localId <" + std::to_string(localId) +  "> " + (active ? "on" : "off")
        );
        cs2writer->send(setLocFunction(localId, static_cast<std::uint8_t>(function), active));
    }

private:
    CS2WriterPtr   cs2writer;
    Function       function;
    std::uint32_t  localId;
    bool           active;
};
