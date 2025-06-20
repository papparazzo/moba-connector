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
#include "moba/configloklistreader.h"

#include "actionabstract.h"

#include <utility>
#include <chrono>
#include <memory>

using namespace std::literals::chrono_literals;

struct ActionLocFunction final: ActionAbstract {

    ActionLocFunction(MonitorPtr monitor, CS2WriterPtr cs2writer, LocomotivesPtr locomotives, std::string function, const bool active):
    ActionAbstract{std::move(monitor)}, cs2writer{std::move(cs2writer)}, locomotives{std::move(locomotives)}, function{std::move(function)}, active{active} {
    }

    void operator()(const std::uint32_t localId) override {
        localId > 0 && cs2writer->send(::setLocFunction(localId, function, on));
    }

private:
    CS2WriterPtr   cs2writer;
    LocomotivesPtr locomotives;
    std::string    function;
    bool           active;
};
