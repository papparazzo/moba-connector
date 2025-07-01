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

#pragma once

#include <memory>

#include "monitor.h"
#include "moba/configloklistreader.h"
#include "moba/endpoint.h"
#include "moba/systemmessages.h"
#include "moba/cs2writer.h"

#include "watchdogToken.h"
#include "sharedData.h"
#include "actionlist.h"

// TODO Consider renaming into CS2Writer instead of JsonReader
class JsonReader final {
public:
    JsonReader(
        CS2WriterPtr cs2writer,
        EndpointPtr endpoint,
        WatchdogTokenPtr watchdogToken,
        SharedDataPtr sharedData,
        MonitorPtr monitor
    );

    ~JsonReader() noexcept = default;

    JsonReader(const JsonReader&) = delete;
    JsonReader(JsonReader&&) = default;
    JsonReader& operator=(const JsonReader&) = delete;
    
    void operator()();

private:
    void setHardwareState(SystemHardwareStateChanged &&data) const;
    void setActionList(const nlohmann::json &d, bool replace) const;

    void shutdown();
    void reset() const;

    [[nodiscard]]
    ActionAbstractPtr getFunctionAction(std::uint32_t localId, const std::string &function, bool active) const;

    [[nodiscard]]
    ActionList getActionList(const nlohmann::json &d, std::uint32_t localId) const;

    bool closing;

    CS2WriterPtr     cs2writer;
    EndpointPtr      endpoint;
    WatchdogTokenPtr watchdogToken;
    SharedDataPtr    sharedData;
    MonitorPtr       monitor;
    LocomotivesPtr   locomotives;
};
