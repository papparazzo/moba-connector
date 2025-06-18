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

    ActionLocSpeed(CS2WriterPtr cs2writer, const std::uint16_t speed):
    cs2writer{std::move(cs2writer)}, speed {speed} {
    }

    void operator()(const std::uint32_t localId) override {
        localId > 0 && cs2writer->send(::setLocSpeed(localId, speed));
    }

private:
    CS2WriterPtr  cs2writer;
    std::uint16_t speed;
};
