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
 *  along with this program. If not, see <http://www.gnu.org/licenses/agpl.txt>.
 *
 */

#include <iostream>
#include <functional>
#include <memory>
#include <thread>

#include <moba-common/helper.h>

#include "config.h"
#include "jsonreader.h"
#include "watchdog.h"
#include "watchdogToken.h"

#include "moba/cs2reader.h"
#include "moba/cs2writer.h"
#include "moba/cs2cancommand.h"

#include "moba/socket.h"
#include "moba/endpoint.h"
#include "jsonwriter.h"
#include "sharedData.h"

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

int main(int argc, char *argv[]) {
    auto socket = std::make_shared<Socket>(appData.host, appData.port);
    auto endpoint = EndpointPtr{new Endpoint{socket, appData.appName, appData.version, {Message::INTERFACE, Message::SYSTEM, Message::TIMER}}};

    auto cs2Writer = std::make_shared<CS2Writer>();
    cs2Writer->connect("192.168.178.38");

    auto cs2Reader = std::make_shared<CS2Reader>();
    cs2Reader->connect();

    auto watchdogToken = std::make_shared<WatchdogToken>();
    auto sharedData = std::make_shared<SharedData>();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    JsonWriter jsonwriter{cs2Reader, cs2Writer, endpoint, watchdogToken, sharedData};
    std::thread jsonwriterThread{std::move(jsonwriter)};
    jsonwriterThread.detach();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    std::thread watchDogThread{Watchdog{watchdogToken, cs2Writer, endpoint}};
    watchDogThread.detach();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    JsonReader jsonReader{cs2Writer, endpoint, watchdogToken, sharedData};
    std::thread jsonReaderThread{std::move(jsonReader)};
    jsonReaderThread.join();

    return EXIT_SUCCESS;
}
