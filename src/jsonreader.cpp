/*
 *  Project:    moba-connector
 *
 *  Copyright (C) 2019 Stefan Paproth <pappi-@gmx.de>
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
 *  along with this program. If not, see <http://www.gnu.org/licenses/agpl.txt>.
 *
 */

#include "jsonreader.h"
#include "moba/registry.h"
#include <moba/log.h>
#include <functional>

JsonReader::JsonReader(CS2WriterPtr cs2writer, EndpointPtr endpoint, BrakeVectorPtr brakeVector) :
cs2writer{cs2writer}, endpoint{endpoint}, brakeVector{brakeVector} {
}

JsonReader::~JsonReader() {
}

void JsonReader::setHardwareState(const SystemHardwareStateChanged &data) {
    switch(data.hardwareState) {
        case SystemHardwareStateChanged::HardwareState::ERROR:
            return;

        case SystemHardwareStateChanged::HardwareState::STANDBY:
        case SystemHardwareStateChanged::HardwareState::EMERGENCY_STOP:
            return cs2writer->send(setEmergencyStop());

        case SystemHardwareStateChanged::HardwareState::MANUEL:
        case SystemHardwareStateChanged::HardwareState::AUTOMATIC:
            return cs2writer->send(setEmergencyStopClearing());
    }
}

void JsonReader::operator()() {
    try {
        Registry registry;
        registry.registerHandler<SystemHardwareStateChanged>(std::bind(&JsonReader::setHardwareState, this, std::placeholders::_1));

        while(true) {
            registry.handleMsg(endpoint->waitForNewMsg());
        }
    } catch(const std::exception &e) {
        LOG(moba::LogLevel::ERROR) << "exception occured! <" << e.what() << ">" << std::endl;
    }
}
