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

#include <thread>
#include <functional>
#include <utility>
#include <moba-common/loggerprefix.h>

#include "actionabstract.h"
#include "actiondelay.h"
#include "actionlocdirection.h"
#include "actionlocfunction.h"
#include "actionlocspeed.h"
#include "actionlocstop.h"
#include "actionsendblockreleased.h"
#include "actionsendroutereleased.h"
#include "actionsendrouteswitched.h"
#include "actionsendswitchroute.h"
#include "actionswitching.h"
#include "actionvoid.h"
#include "moba/enumactiontype.h"
#include "moba/enumfunction.h"

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

void JsonReader::shutdown() {
    // TODO: Vor dem Shutdown sollten schon alle Züge angehalten haben.
    //cs2writer->send(::setHalt());
    closing = true;
}

void JsonReader::reset() const {
    cs2writer->send(setHalt());
}

void JsonReader::setActionList(const nlohmann::json &d) const {
    const int id = d["id"].get<int>();

    // TODO: Prüfen ob id schon hinterlegt ist

    std::vector<ActionAbstractPtr> actionList;

    std::uint32_t localId = 0;

    for(auto &iter: d["actionList"]) {
        switch(auto action = iter["action"].get<std::string>(); stringToActionTypeEnum(action)) {
            case ActionType::VOID:
                actionList.emplace_back(std::make_shared<ActionVoid>());
                continue;

            case ActionType::DELAY:
                actionList.emplace_back(std::make_shared<ActionDelay>(iter["data"].get<int>));
                continue;

            case ActionType::LOCO_HALT:
                actionList.emplace_back(std::make_shared<ActionLocStop>(cs2writer, localId));
                continue;

            case ActionType::LOCO_SPEED:
                actionList.emplace_back(std::make_shared<ActionLocSpeed>(cs2writer, localId, iter["data"].get<int>));
                continue;

            case ActionType::LOCO_DIRECTION_BACKWARD:
                actionList.emplace_back(std::make_shared<ActionLocDirection>(cs2writer, localId, moba::DrivingDirection::BACKWARD));
                continue;

            case ActionType::LOCO_DIRECTION_FORWARD:
                actionList.emplace_back(std::make_shared<ActionLocDirection>(cs2writer, localId, moba::DrivingDirection::FORWARD));
                continue;

            case ActionType::LOCO_FUNCTION_ON:
                actionList.emplace_back(getFunctionAction(localId, iter["data"].get<std::string>, true));
                continue;

            case ActionType::LOCO_FUNCTION_OFF:
                actionList.emplace_back(getFunctionAction(localId, iter["data"].get<std::string>, false));
                continue;

            case ActionType::SWITCHING_RED:
                actionList.emplace_back(std::make_shared<ActionSwitching>(cs2writer, localId, true));
                continue;

            case ActionType::SWITCHING_GREEN:
                actionList.emplace_back(std::make_shared<ActionSwitching>(cs2writer, localId, false));
                continue;

            case ActionType::SEND_SWITCH_ROUTE:
                actionList.emplace_back(std::make_shared<ActionSendSwitchRoute>(iter["data"].get<std::string>, true));
                continue;

            case ActionType::SEND_ROUTE_SWITCHED:
                actionList.emplace_back(std::make_shared<ActionSendRouteSwitched>(iter["data"].get<std::string>, true));
                continue;

            case ActionType::SEND_ROUTE_RELEASED:
                actionList.emplace_back(std::make_shared<ActionSendRouteReleased>(iter["data"].get<std::string>, true));
                continue;

            case ActionType::SEND_BLOCK_RELEASED:
                actionList.emplace_back(std::make_shared<ActionSendBlockReleased>(iter["data"].get<std::string>, true));
                continue;

            default:
                monitor->appendAction(moba::LogLevel::WARNING, "unknown action type <" + action + "> for id <" + std::to_string(id) + ">");
                // TODO Send Error to moba-server
                continue;
        }

        sharedData->actionListHandler.insertActionList();
       // sharedData->actionListHandler.trigger(ActionData{id, actionList});

    }

/*
    Integer	id		1	1	Eindeutige Id
    ContactTrigger	trigger		0	1	
    ActionData	actionList		1	-1	

    SwitchingOutputsHandler soh{endpoint, cs2writer, std::move(data.switchingOutputs)};
    std::thread jsonwriterThread{std::move(soh)};
    jsonwriterThread.detach();

    explicit InterfaceHandleActionList(const nlohmann::json &d) {
        if (!d["trigger"].is_null()) {
            contactTrigger = ContactTrigger{d["trigger"]};
        }

    }
    */
}

ActionAbstractPtr JsonReader::getFunctionAction(std::uint32_t localId, const std::string &function, bool active) const {

    const auto iter = locomotives->find(localId);
    if(iter == locomotives->end()) {
        monitor->appendAction(moba::LogLevel::NOTICE, "given localId <" + std::to_string(localId) + "> does not exist");
        return std::make_shared<ActionVoid>();
    }

    auto funcEnum = stringToControllableFunctionEnum(function);

    auto &func = iter->second->functions;

    // TODO: Try alternative functions...
    const auto funcIter = func.find(static_cast<int>(funcEnum));

    if(funcIter == func.end()) {
        monitor->appendAction(moba::LogLevel::WARNING, "no function found for localId <" + std::to_string(localId) + ">");
        return std::make_shared<ActionVoid>();
    }
    monitor->appendAction(
        moba::LogLevel::NOTICE,
        "set function " + function + " for localId <" +
        std::to_string(localId) +  "> " + (active ? "on" : "off")
    );

    return std::make_shared<ActionLocFunction>(cs2writer, localId, funcIter->second, active);
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
