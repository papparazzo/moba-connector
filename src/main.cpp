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

#include <moba/helper.h>
#include <moba/log.h>

#include "config.h"
#include "concurrentqueue.h"
#include "cs2reader.h"
#include "jsonreader.h"
#include "shared.h"

#include "moba/socket.h"
#include "moba/endpoint.h"

namespace {
    moba::AppData appData = {
        PACKAGE_NAME,
        moba::Version(PACKAGE_VERSION),
        __DATE__,
        __TIME__,
        "localhost",
        7000
    };
}

int main(int argc, char *argv[]) {
    moba::setCoreFileSizeToULimit();

    ConcurrentCanQueuePtr dataToAppServer{new ConcurrentQueue<CS2CanRawData>};
    ConcurrentCanQueuePtr dataToCS2{new ConcurrentQueue<CS2CanRawData>};

    BrakeVectorPtr brakeVector{new BrakeVector{}};

    CS2Reader cs2reader{dataToCS2, dataToAppServer, brakeVector};
    cs2reader.connect("192.168.178.38");

    std::thread cs2ReaderThread{[&cs2reader](){cs2reader();}};
    cs2ReaderThread.join();

    moba::JsonArrayPtr groups{new moba::JsonArray{}};
    groups->push_back(moba::toJsonStringPtr("SYSTEM"));

    SocketPtr socket{new Socket{appData.host, appData.port}};
    EndpointPtr endpoint{new Endpoint{socket}};
    endpoint->connect(appData.appName, appData.version, groups);

    JsonReader jsonReader{dataToCS2, endpoint, brakeVector};

    std::thread jsonReaderThread{[&jsonReader](){jsonReader();}};
    jsonReaderThread.join();



    return EXIT_SUCCESS;
}
