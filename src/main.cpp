/*
 *  Project:    moba-connector
 *
 *  Copyright (C) 2018 Stefan Paproth <pappi-@gmx.de>
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

#include <memory>
#include <thread>

#include <moba-common/helper.h>

#include "argumentparser.h"
#include "config.h"
#include "jsonreader.h"
#include "watchdog.h"
#include "watchdogToken.h"

#include "moba/cs2reader.h"
#include "moba/cs2writer.h"

#include "moba/socket.h"
#include "moba/endpoint.h"
#include "jsonwriter.h"
#include "monitor.h"
#include "sharedData.h"
#include "moba/cs2contactdata.h"

namespace {
    moba::AppData appData = {
        PACKAGE_NAME,
        moba::Version(PACKAGE_VERSION),
        __DATE__,
        __TIME__,
        "::1",
        7000
    };
}

int main(const int argc, char *argv[]) {
    CS2ContactData cs2ContactData = {
        "192.168.178.38",
        CS2Writer::DEFAULT_PORT,
        CS2Reader::DEFAULT_PORT
    };
    bool debug = false;

    if(ArgumentParser::parseArguments(argc, argv, appData, cs2ContactData, pingSettings, debug)) {
        return EXIT_SUCCESS;
    }

    const auto socket = std::make_shared<Socket>(appData.host, appData.port);
    const auto endpoint = EndpointPtr{
        new Endpoint{
            socket,
            appData.appName,
            "connector",
            appData.version,
            {Message::INTERFACE, Message::SYSTEM}
        }
    };

    const auto cs2WriterPtr = std::make_shared<CS2Writer>(cs2ContactData.host, cs2ContactData.portIn);
    const auto cs2ReaderPtr = std::make_shared<CS2Reader>(cs2ContactData.host, cs2ContactData.portOut);

    const auto watchdogToken = std::make_shared<WatchdogToken>();
    const auto sharedData = std::make_shared<SharedData>();
    const auto monitor = std::make_shared<Monitor>(debug);

    ///////////////////////////////////////////////////////////////////////////////////
    //
    JsonWriter jsonwriter{cs2ReaderPtr, cs2WriterPtr, endpoint, watchdog, sharedData, monitor};
    std::thread jsonwriterThread{std::move(jsonwriter)};
    jsonwriterThread.detach();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    JsonReader jsonReader{cs2WriterPtr, endpoint, watchdog, sharedData, monitor};
    std::thread jsonReaderThread{std::move(jsonReader)};
    jsonReaderThread.join();

    return EXIT_SUCCESS;
}
