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

#pragma once

#include "watchdogToken.h"
#include "cs2cancommand.h"
#include "cs2writer.h"
#include "moba/endpoint.h"
#include "moba/interfacehandler.h"

class Watchdog {
    public:
        Watchdog(WatchdogTokenPtr watchdog, CS2WriterPtr cs2writer, EndpointPtr endpoint);

        virtual ~Watchdog() noexcept;

        void operator()();

    protected:
        using ConnectState = InterfaceConnectivityStateChanged::Connectivity;

        WatchdogTokenPtr watchdog;
        CS2WriterPtr cs2writer;
        EndpointPtr endpoint;

        ConnectState lastState;
};

