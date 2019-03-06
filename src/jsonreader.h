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

#include "cs2cancommand.h"

#include <boost/noncopyable.hpp>
#include <string>
#include <memory>

#include "moba/endpoint.h"
#include "moba/systemhandler.h"
#include "moba/interfacehandler.h"
#include "brakevector.h"
#include "cs2writer.h"

class JsonReader : private boost::noncopyable {
    public:
        JsonReader(CS2WriterPtr cs2writer, EndpointPtr endpoint, BrakeVectorPtr brakeVector);
        virtual ~JsonReader() noexcept;

        void operator()();

    protected:
        CS2WriterPtr cs2writer;
        EndpointPtr endpoint;
        BrakeVectorPtr brakeVector;

        void setHardwareState(const SystemHardwareStateChanged &data);
        void setBrakeVector(const InterfaceSetBrakeVector &data);

};
