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
#include "actionabstract.h"

#include <utility>
#include <chrono>
#include <memory>

#include "moba/cs2utils.h"

using namespace std::literals::chrono_literals;

struct ActionLocFunction final: ActionAbstract {

    ActionLocFunction(CS2WriterPtr cs2writer, const std::uint32_t localId, const std::uint8_t function, const bool on):
    cs2writer{std::move(cs2writer)}, localId{localId}, function{function}, on{on} {
    }

    void operator()() override {
        cs2writer->send(::setLocFunction(localId, function, on));
    }

private:
    CS2WriterPtr  cs2writer;
    std::uint32_t localId;
    std::uint8_t  function;
    bool          on;
};
