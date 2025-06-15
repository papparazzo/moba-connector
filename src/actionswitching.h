/*
 *  Project:    moba-connector
 * 
 *  Copyright (C) 2023 Stefan Paproth <pappi-@gmx.de>
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
#include "moba/endpoint.h"
#include "actionabstract.h"

#include <thread>
#include <utility>

#include "moba/cs2utils.h"

struct ActionSwitching final : ActionAbstract {
	ActionSwitching(CS2WriterPtr cs2writer, const std::uint32_t localId, const bool r): cs2writer{std::move(cs2writer)},
        differ(false), localId{localId}, r{r} {
    }

    void operator()() override {
        using namespace std::chrono_literals;

        cs2writer->send(::setSwitch(localId, r, true));
        std::this_thread::sleep_for(50ms);
        cs2writer->send(::setSwitch(localId, r, false));
        std::this_thread::sleep_for(250ms);
    }

private:
    CS2WriterPtr  cs2writer;
    bool          differ;
    std::uint32_t localId;
    bool          r;
};
