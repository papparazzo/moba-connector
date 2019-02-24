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
#include "cs2writer.h"
#include "jsonreader.h"
#include "jsonwriter.h"
#include "cs2cancommand.h"

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

    auto dataToAppServer = std::make_shared<ConcurrentQueue<DispatchMessage>>();
    auto dataToCS2 = std::make_shared<ConcurrentQueue<DispatchMessage>>();
    auto brakeVector = std::make_shared<BrakeVector>();

    auto groups = std::make_shared<moba::JsonArray>();
    groups->push_back(moba::toJsonStringPtr("SYSTEM"));

    auto socket = std::make_shared<Socket>(appData.host, appData.port);
    auto endpoint = std::make_shared<Endpoint>(socket, appData.appName, appData.version, groups);
    endpoint->connect();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    CS2Reader cs2Reader{dataToCS2, dataToAppServer, brakeVector};
    cs2Reader.connect("192.168.178.38");

    std::thread cs2ReaderThread{[&cs2Reader](){cs2Reader();}};
    cs2ReaderThread.join();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    JsonReader jsonReader{dataToCS2, endpoint, brakeVector};

    std::thread jsonReaderThread{[&jsonReader](){jsonReader();}};
    jsonReaderThread.join();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    CS2Writer cs2Writer{dataToCS2};

    std::thread cs2writerThread{[&cs2Writer](){cs2Writer();}};
    cs2writerThread.join();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    JsonWriter jsonWriter{dataToCS2, endpoint};

    std::thread jsonWriterThread{[&jsonWriter](){jsonWriter();}};
    cs2writerThread.join();

    return EXIT_SUCCESS;
}
