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

#include <string>
#include <memory>

#include "moba/clientmessages.h"
#include "moba/configloklistreader.h"
#include "moba/endpoint.h"
#include "moba/systemmessages.h"
#include "moba/interfacemessages.h"
#include "moba/shared.h"
#include "moba/cs2cancommand.h"
#include "moba/cs2writer.h"

#include "brakevector.h"
#include "watchdogToken.h"
#include "sharedData.h"

class JsonReader final {
public:
    JsonReader(CS2WriterPtr cs2writer, EndpointPtr endpoint, WatchdogTokenPtr watchdogToken, SharedDataPtr sharedData);
    virtual ~JsonReader() noexcept = default;

    JsonReader(const JsonReader&) = delete;
    JsonReader(JsonReader&&) = default;
    JsonReader& operator=(const JsonReader&) = delete;
    
    void operator()();

protected:
    void setHardwareState(SystemHardwareStateChanged &&data);
    void setBrakeVector(InterfaceSetBrakeVector &&data);
    void resetBrakeVector(InterfaceResetBrakeVector &&data);
    void setSwitch(InterfaceSwitchAccessoryDecoders &&data);
    void setLocoFunction(InterfaceSetLocoFunction &&data) const;
    
    void shutdown();
    void reset() const;

    bool closing;

    CS2WriterPtr     cs2writer;
    EndpointPtr      endpoint;
    WatchdogTokenPtr watchdogToken;
    SharedDataPtr    sharedData;
    LocomotivesPtr   locomotives;
};
