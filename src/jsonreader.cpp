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
 *  along with this program. If not, see <https://www.gnu.org/licenses/agpl.txt>.
 *
 */

#include "jsonreader.h"
#include "moba/registry.h"
#include "moba/cs2utils.h"
#include "moba/clientmessages.h"
#include "switchingoutputshandler.h"

#include <thread>
#include <functional>
#include <utility>
#include <moba-common/loggerprefix.h>

// TODO Consider renaming into CS2Writer instead of JsonReader
JsonReader::JsonReader(
    CS2WriterPtr cs2writer,
    EndpointPtr endpoint,
    WatchdogTokenPtr watchdogToken,
    SharedDataPtr sharedData,
    MonitorPtr monitor
) : closing{false}, cs2writer{std::move(cs2writer)}, endpoint{std::move(endpoint)},
watchdogToken{std::move(watchdogToken)}, sharedData{std::move(sharedData)}, monitor{std::move(monitor)} {
}

void JsonReader::setHardwareState(SystemHardwareStateChanged &&data) const {
    monitor->printStatus(data.hardwareState);
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

void JsonReader::setBrakeVector(InterfaceSetBrakeVector &&data) const {
    for(auto iter: data.items) {
        sharedData->brakeVector.handleContact(
            {iter.contact.moduleAddr, iter.contact.contactNb},
            iter.localId
        );
    }
    monitor->printBrakeVector(sharedData->brakeVector.getVector());
}

void JsonReader::resetBrakeVector(InterfaceResetBrakeVector &&data) const {
    for(auto iter: data.items) {
        sharedData->brakeVector.handleContact(
            {iter.contact.moduleAddr, iter.contact.contactNb},
            BrakeVector::IGNORE_CONTACT
        );        
    }
    monitor->printBrakeVector(sharedData->brakeVector.getVector());
}

void JsonReader::shutdown() {
    // TODO: Vor dem Shutdown sollten schon alle Züge angehalten haben.
    //cs2writer->send(::setHalt());
    closing = true;
}

void JsonReader::reset() const {
    //cs2writer->send(::setHalt());
    sharedData->brakeVector.reset();
}

void JsonReader::setSwitch(InterfaceSwitchAccessoryDecoders &&data) const {
    SwitchingOutputsHandler soh{endpoint, cs2writer, std::move(data.switchingOutputs)};
    
    std::thread jsonwriterThread{std::move(soh)};
    jsonwriterThread.detach();
}

void JsonReader::setLocoFunction(InterfaceSetLocoFunction &&data) const {

    const auto iter = locomotives->find(data.localId);
    if(iter == locomotives->end()) {
        monitor->appendAction(
            moba::LogLevel::NOTICE,
            "given localId <" + std::to_string(data.localId) + "> does not exist"
        );
        endpoint->sendMsg(ClientError{ErrorId::INVALID_VALUE_GIVEN, "given localId does not exist"});
        return;
    }
    
    auto &func = iter->second->functions;
    
    // TODO: Try alternative functions...
    auto iterf = func.find(static_cast<std::uint32_t>(data.function));

    if(iterf == func.end()) {
        monitor->appendAction(
            moba::LogLevel::WARNING,
            "no function found for localId <" + std::to_string(data.localId) + ">"
        );
        return;
    }
    monitor->appendAction(
        moba::LogLevel::NOTICE,
        "set function " + controllableFunctionEnumToString(data.function) + " for localid <" +
        std::to_string(data.localId) +  "> " + (data.active ? "on" : "off")
    );

    cs2writer->send(::setLocFunction(
        data.localId,
        static_cast<std::uint8_t>(iterf->second),
        data.active
    ));
}

void JsonReader::operator()() {
    while(!closing) {
        try {
            endpoint->connect();
            Registry registry;
            registry.registerHandler<SystemHardwareStateChanged>(std::bind(&JsonReader::setHardwareState, this, std::placeholders::_1));
            registry.registerHandler<InterfaceSetBrakeVector>(std::bind(&JsonReader::setBrakeVector, this, std::placeholders::_1));
            registry.registerHandler<InterfaceResetBrakeVector>(std::bind(&JsonReader::resetBrakeVector, this, std::placeholders::_1));
            registry.registerHandler<InterfaceSetLocoDirection>([this](InterfaceSetLocoDirection &&d) {cs2writer->send(setLocDirection(d.localId, static_cast<std::uint8_t>(d.direction)));});
            registry.registerHandler<InterfaceSetLocoSpeed>([this](InterfaceSetLocoSpeed &&d) {cs2writer->send(setLocSpeed(d.localId, d.speed));});
            registry.registerHandler<InterfaceSetLocoFunction>(std::bind(&JsonReader::setLocoFunction, this, std::placeholders::_1));            
            registry.registerHandler<InterfaceSwitchAccessoryDecoders>(std::bind(&JsonReader::setSwitch, this, std::placeholders::_1));
            registry.registerHandler<ClientShutdown>([this]{shutdown();});
            registry.registerHandler<ClientReset>([this]{reset();});
            registry.registerHandler<SystemSetAutomaticMode>([this](SystemSetAutomaticMode &&d) {sharedData->automatic = d.automaticActive;});

            while(!closing) {
                registry.handleMsg(endpoint->waitForNewMsg());
            }
        } catch(const std::exception &e) {
            watchdogToken->synchronizeStart();
            monitor->printException("JsonReader::operator()()", e.what());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
    }
}
