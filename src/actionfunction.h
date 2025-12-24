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

#include "actionabstract.h"

#include <utility>
#include <memory>

#include "moba/endpoint.h"
#include "moba/environmentmessages.h"

using namespace std::literals::chrono_literals;

struct ActionFunction final: ActionAbstract {

    ActionFunction(MonitorPtr monitor, EndpointPtr endpoint, const FunctionStateData function):
    ActionAbstract{std::move(monitor)}, endpoint{std::move(endpoint)}, function{function} {
    }

    void operator()() override {
        monitor->appendAction(
            "ActionFunction",
            "set function <" + std::to_string(function.address.deviceId) + "/" + std::to_string(function.address.address.controller) + ":" +
            std::to_string(function.address.address.port) + "> to <" + functionStateEnumToString(function.functionState)  +  "> ");
        endpoint->sendMsg(EnvironmentSetFunctions{function});
    }

private:
    EndpointPtr       endpoint;
    FunctionStateData function;
};
