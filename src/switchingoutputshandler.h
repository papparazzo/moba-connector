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

#include <vector>

#include "moba/cs2writer.h"
#include "moba/endpoint.h"
#include "moba/shared.h"
#include "item.h"

class SwitchingOutputsHandler final : public Item {
public:

    SwitchingOutputsHandler(EndpointPtr endpoint, CS2WriterPtr cs2writer, SwitchingOutputs && switchVector);
    SwitchingOutputsHandler(SwitchingOutputsHandler&&) = default;
    
    SwitchingOutputsHandler(const SwitchingOutputsHandler&) = delete;
    SwitchingOutputsHandler& operator=(const SwitchingOutputsHandler&) = delete;
    
    
    ~SwitchingOutputsHandler() noexcept override = default;
    
    void operator()() override;
    
private:
    EndpointPtr endpoint;
    CS2WriterPtr cs2writer;
    SwitchingOutputs switchVector;

    void setSwitch(std::uint8_t addr, bool r) const;
};
