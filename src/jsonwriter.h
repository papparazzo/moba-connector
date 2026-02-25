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
 *  along with this program. If not, see <https://www.gnu.org/licenses/agpl.txt>.
 *
 */

#pragma once

#include "monitor.h"
#include "moba/cs2cancommand.h"
#include "moba/endpoint.h"
#include "moba/cs2writer.h"
#include "moba/cs2reader.h"

#include "watchdog.h"
#include "sharedData.h"

// TODO Consider renaming into CS2Reader instead of JsonWriter
class JsonWriter final {
public:
    JsonWriter(
        CS2ReaderPtr cs2reader,
        CS2WriterPtr cs2writer,
        EndpointPtr endpoint,
        WatchdogPtr watchdog,
        SharedDataPtr sharedData,
        MonitorPtr monitor
    );

    ~JsonWriter() noexcept = default;

    JsonWriter(JsonWriter&&) = default;
    JsonWriter(const JsonWriter&) = delete;
    JsonWriter& operator=(const JsonWriter&) = delete;
    
    void operator()() const;

private:
    [[nodiscard]]
    bool s88report(const CS2CanCommand &data) const;

    [[nodiscard]]
    bool systemCommands(const CS2CanCommand &cmd) const;

    [[nodiscard]]
    bool controlLocoCommands(const CS2CanCommand &cmd) const;

    [[nodiscard]]
    bool controlSwitch(const CS2CanCommand &cmd) const;
    
    void readFunctionList() const;

    CS2ReaderPtr cs2reader;
    CS2WriterPtr cs2writer;
    EndpointPtr endpoint;
    WatchdogTokenPtr watchdogToken;
    void emergencyStop(const std::string &what) const;
    SharedDataPtr sharedData;
    MonitorPtr monitor;
};
