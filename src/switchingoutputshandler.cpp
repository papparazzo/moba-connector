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

#include "switchingoutputshandler.h"
#include "moba/cs2utils.h"

#include <thread>
#include <utility>

SwitchingOutputsHandler::SwitchingOutputsHandler(EndpointPtr endpoint, CS2WriterPtr cs2writer, SwitchingOutputs && switchVector):
endpoint{std::move(endpoint)}, cs2writer{std::move(cs2writer)}, switchVector{std::move(switchVector)} {
}

void SwitchingOutputsHandler::operator()() {
    for(auto const& v: switchVector) {
        setSwitch(v.localId, v.differ);
    }
    // TODO: Hier noch eine Response-Id mit übergeben...
    //endpoint->sendMsg(InterfaceSwitchAccessoryDecodersComplete);
}

void SwitchingOutputsHandler::setSwitch(const std::uint8_t addr, const bool r) const {
    using namespace std::chrono_literals;
    
    cs2writer->send(::setSwitch(convertMMToLocId(addr), r, true));
    std::this_thread::sleep_for(50ms);
    cs2writer->send(::setSwitch(convertMMToLocId(addr), r, false));
    std::this_thread::sleep_for(250ms);
}
