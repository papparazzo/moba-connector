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

#include <utility>

#include "actionabstract.h"

#include "moba/endpoint.h"
#include "moba/interfacemessages.h"

struct ActionSendSwitchRoute final: ActionAbstract {
    ActionSendSwitchRoute(MonitorPtr monitor, EndpointPtr endpoint, const unsigned long id): ActionAbstract{std::move(monitor)}, id{id}, endpoint{std::move(endpoint)} {
    }

    void operator()() override {
        monitor->appendAction("ActionSendSwitchRoute", "sending switch route for id <" + std::to_string(id) + ">");
        endpoint->sendMsg(InterfaceSwitchRoute{id});
    }

protected:
    unsigned long id;
    EndpointPtr endpoint;
};
