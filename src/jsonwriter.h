/*
 *  Project:    moba-connector
 *
 *  Copyright (C) 2020 Stefan Paproth <pappi-@gmx.de>
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

#include <boost/noncopyable.hpp>

#include "moba/cs2cancommand.h"
#include "moba/endpoint.h"
#include "moba/cs2writer.h"
#include "moba/cs2reader.h"

#include "brakevector.h"
#include "watchdogToken.h"

class JsonWriter : private boost::noncopyable {
    public:
        JsonWriter(CS2ReaderPtr cs2reader, CS2WriterPtr cs2writer, EndpointPtr endpoint, WatchdogTokenPtr watchdog, BrakeVectorPtr brakeVector);
        virtual ~JsonWriter() noexcept { }

        void operator()();

    protected:
        void s88report(const CS2CanCommand &data);
        void convertSystemCommand(const CS2CanCommand &cmd) const;
        void controlLoco(const CS2CanCommand &cmd) const;

        CS2ReaderPtr cs2reader;
        CS2WriterPtr cs2writer;
        EndpointPtr endpoint;
        BrakeVectorPtr brakeVector;
        WatchdogTokenPtr watchdog;
};
