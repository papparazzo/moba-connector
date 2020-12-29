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
#include "moba/cs2utils.h"

#include <moba-common/log.h>
#include <thread>
#include <functional>

JsonReader::JsonReader(CS2WriterPtr cs2writer, EndpointPtr endpoint, WatchdogTokenPtr watchdogToken, BrakeVectorPtr brakeVector) :
closing{false}, cs2writer{cs2writer}, endpoint{endpoint}, brakeVector{brakeVector}, watchdogToken{watchdogToken} {
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

void JsonReader::setBrakeVector(const InterfaceSetBrakeVector &data) {
    for(auto iter : data.items) {
        brakeVector->handleContact({iter.contact.modulAddr, iter.contact.contactNb}, iter.localId);
    }
}

void JsonReader::shutdown() {
    //cs2writer->send(setHalt());
    closing = true;
}

void JsonReader::reset() {
    //cs2writer->send(setHalt());
    brakeVector->reset();
}

void JsonReader::operator()() {
    while(!closing) {
        try {
            endpoint->connect();
            Registry registry;
            registry.registerHandler<SystemHardwareStateChanged>(std::bind(&JsonReader::setHardwareState, this, std::placeholders::_1));
            registry.registerHandler<InterfaceSetBrakeVector>(std::bind(&JsonReader::setBrakeVector, this, std::placeholders::_1));
            registry.registerHandler<InterfaceSetLocoDirection>([this](const InterfaceSetLocoDirection &d){cs2writer->send(setLocDirection(d.localId, static_cast<std::uint8_t>(d.direction)));});
            registry.registerHandler<InterfaceSetLocoSpeed>([this](const InterfaceSetLocoSpeed &d){cs2writer->send(setLocSpeed(d.localId, d.speed));});
            registry.registerHandler<ClientShutdown>([this]{shutdown();});
            registry.registerHandler<ClientReset>([this]{reset();});

            while(true) {
                registry.handleMsg(endpoint->waitForNewMsg());
            }
        } catch(const std::exception &e) {
            watchdogToken->synchronizeStart();
            LOG(moba::common::LogLevel::ERROR) << "exception occured! <" << e.what() << ">" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
    }
}
