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

#include "actionabstract.h"
#include "actiondelay.h"
#include "actionlist.h"
#include "actionlocdirection.h"
#include "actionlocfunction.h"
#include "actionlocspeed.h"
#include "actionlocstop.h"
#include "actionsendblockreleased.h"
#include "actionsendroutereleased.h"
#include "actionsendrouteswitched.h"
#include "actionsendswitchroute.h"
#include "actionswitching.h"
#include "moba/enumactiontype.h"

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

ActionAbstractPtr JsonReader::getFunctionAction(std::uint32_t localId, const std::string &function, bool active) const {

    const auto iter = locomotives->find(localId);
    if(iter == locomotives->end()) {
        throw std::runtime_error("given localId <" + std::to_string(localId) + "> does not exist");
    }

    auto funcEnum = stringToControllableFunctionEnum(function);

    auto &func = iter->second->functions;

    // TODO: Try alternative functions...
    const auto funcIter = func.find(static_cast<int>(funcEnum));

    if(funcIter == func.end()) {
        return std::make_shared<ActionLocFunction>(monitor, cs2writer, localId, Function::NONE, active);
    }

    return std::make_shared<ActionLocFunction>(monitor, cs2writer, localId, static_cast<Function>(funcIter->second), active);
}


ActionList JsonReader::getActionList(const nlohmann::json &d, std::uint32_t localId) const {
    ActionList actionList;

    for(auto &iter: d["actions"]) {
        switch(auto action = iter["action"].get<std::string>(); stringToActionTypeEnum(action)) {
            case ActionType::DELAY:
                actionList.append(std::make_shared<ActionDelay>(monitor, std::chrono::milliseconds(iter["data"].get<int>())));
                continue;

            case ActionType::LOCO_HALT:
                actionList.append(std::make_shared<ActionLocStop>(monitor, cs2writer, localId));
                continue;

            case ActionType::LOCO_SPEED:
                actionList.append(std::make_shared<ActionLocSpeed>(monitor, cs2writer, localId, iter["data"].get<int>()));
                continue;

            case ActionType::LOCO_DIRECTION_BACKWARD:
                actionList.append(std::make_shared<ActionLocDirection>(monitor, cs2writer, localId, moba::DrivingDirection::BACKWARD));
                continue;

            case ActionType::LOCO_DIRECTION_FORWARD:
                actionList.append(std::make_shared<ActionLocDirection>(monitor, cs2writer, localId, moba::DrivingDirection::FORWARD));
                continue;

            case ActionType::LOCO_FUNCTION_ON:
                actionList.append(getFunctionAction(localId, iter["data"].get<std::string>(), true));
                continue;

            case ActionType::LOCO_FUNCTION_OFF:
                actionList.append(getFunctionAction(localId, iter["data"].get<std::string>(), false));
                continue;

            case ActionType::SWITCHING_RED:
                actionList.append(std::make_shared<ActionSwitching>(monitor, cs2writer, iter["data"].get<int>(), true));
                continue;

            case ActionType::SWITCHING_GREEN:
                actionList.append(std::make_shared<ActionSwitching>(monitor, cs2writer, iter["data"].get<int>(), false));
                continue;

            case ActionType::SEND_SWITCH_ROUTE:
                actionList.append(std::make_shared<ActionSendSwitchRoute>(monitor, endpoint, iter["data"].get<int>()));
                continue;

            case ActionType::SEND_ROUTE_SWITCHED:
                actionList.append(std::make_shared<ActionSendRouteSwitched>(monitor, endpoint, iter["data"].get<int>()));
                continue;

            case ActionType::SEND_ROUTE_RELEASED:
                actionList.append(std::make_shared<ActionSendRouteReleased>(monitor, endpoint, iter["data"].get<int>()));
                continue;

            case ActionType::SEND_BLOCK_RELEASED:
                actionList.append(std::make_shared<ActionSendBlockReleased>(monitor, endpoint, iter["data"].get<int>()));
                continue;

            default:
                throw std::runtime_error("unknown action type <" + action + ">");
        }
    }
    return actionList;
}

void JsonReader::setActionList(const nlohmann::json &d, bool replace) const {
    const std::uint32_t localId = d["localId"].get<std::uint32_t>();

    if(d["trigger"].is_null()) {
        for(auto &iter: d["actionLists"]) {
            auto actionList = getActionList(d, localId);
            std::thread jsonwriterThread{std::move(actionList)};
            jsonwriterThread.detach();
        }
        return;
    }

    const auto triggerContact = ContactData{d["trigger"]};
    const auto listCollection = std::make_shared<ActionListCollection>();

    for(auto &iter: d["actionLists"]) {
        auto actionList = getActionList(d, localId);
        listCollection->push(std::move(actionList));
    }

    if(replace) {
        sharedData->actionListHandler.replaceActionList(triggerContact, listCollection);
    } else {
        sharedData->actionListHandler.insertActionList(triggerContact, listCollection);
    }
}

void JsonReader::operator()() {
    while(!closing) {
        try {
            endpoint->connect();
            Registry registry;
            registry.registerHandler<SystemHardwareStateChanged>(std::bind(&JsonReader::setHardwareState, this, std::placeholders::_1));
            registry.registerHandler(InterfaceMessage::GROUP_ID, InterfaceMessage::REPLACE_ACTION_LIST, std::bind(&JsonReader::setActionList, this, std::placeholders::_1, true));
            registry.registerHandler(InterfaceMessage::GROUP_ID, InterfaceMessage::SET_ACTION_LIST, std::bind(&JsonReader::setActionList, this, std::placeholders::_1, false));
            registry.registerHandler<ClientShutdown>([this]{shutdown();});
            registry.registerHandler<ClientReset>([this]{reset();});
            registry.registerHandler<SystemSetAutomaticMode>([this](SystemSetAutomaticMode &&d) {sharedData->automatic = d.automaticActive;});

            while(!closing) {
                registry.handleMsg(endpoint->waitForNewMsg());
            }
        } catch(const std::exception &e) {
            watchdogToken->synchronizeStart();
            monitor->printException("JsonReader::operator()()", e.what());
            endpoint->sendMsg(SystemTriggerEmergencyStop{SystemTriggerEmergencyStop::EmergencyTriggerReason::SOFTWARE_ERROR});
            cs2writer->send(setEmergencyStop());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
    }
}
