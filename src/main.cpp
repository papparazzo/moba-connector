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
#include <getopt.h>

#include <moba-common/helper.h>

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
#include "moba/cs2utils.h"

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

struct CS2ContactData {
    std::string host;
    int portIn;
    int portOut;
};

void printHelp(const std::string &appName, const CS2ContactData &cs2ContactData) {
    std::cout
        << appName << " [host] [port]" << std::endl
        << std::endl
        << "-d, --debug         enables debug output" << std::endl
        << "-h, --help          shows this help" << std::endl
        << "-v, --version       shows version-info" << std::endl
        << "-c, --cs2-host      host of the CentralStation (default: " << cs2ContactData.host << ")" << std::endl
        << "-i, --cs2-port-in   port of the CentralStation for incoming messages (default: " << cs2ContactData.portIn << ")" << std::endl
        << "-o, --cs2-port-out  port of the CentralStation for outgoing messages (default: " << cs2ContactData.portOut << ")" << std::endl;
}

bool parseArguments(const int argc, char *argv[], moba::AppData &appData, CS2ContactData &cs2ContactData) {
    static option longOptions[] = {
        {"cs2-host",     required_argument, nullptr, 'c'},
        {"cs2-port-in",  required_argument, nullptr, 'i'},
        {"cs2-port-out", required_argument, nullptr, 'o'},
        {"help",         no_argument,       nullptr, 'h'},
        {"version",      no_argument,       nullptr, 'v'},
        {nullptr,        0,                 nullptr, 0 }
    };

    int optionIndex = 0;

    while(true) {
        const int c = getopt_long(argc, argv, "hvc:i:o:", longOptions, &optionIndex);
        if(c == -1) {
            break;
        }

        switch(c) {
            case 'h':
                printHelp(basename(argv[0]), cs2ContactData);
                return true;

            case 'v':
                moba::printAppData(appData);
                return true;

            case 'c':
                cs2ContactData.host = optarg;
                break;

            case 'i':
                cs2ContactData.portIn = std::stoi(optarg);
                break;

            case 'o':
                cs2ContactData.portOut = std::stoi(optarg);
                break;

            default:
                std::cerr << "Try '" << basename(argv[0]) << " --help' for more information." << std::endl;
                exit(2);
        }
    }

    switch(argc - optind) {
        case 2:
            appData.port = std::stoi(argv[optind + 1]);

        case 1:
            appData.host = argv[optind];
            break;

        default:
            break;
    }
    return false;
}

int main(const int argc, char *argv[]) {
    CS2ContactData cs2ContactData = {
        "192.168.178.38",
        CS2Writer::DEFAULT_PORT,
        CS2Reader::DEFAULT_PORT
    };

    if(parseArguments(argc, argv, appData, cs2ContactData)) {
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
    const auto cs2ReaderPtr = std::make_shared<CS2Reader>(cs2ContactData.portOut);

    const auto watchdogToken = std::make_shared<WatchdogToken>();
    const auto sharedData = std::make_shared<SharedData>();
    const auto monitor = std::make_shared<Monitor>();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    JsonWriter jsonwriter{cs2ReaderPtr, cs2WriterPtr, endpoint, watchdogToken, sharedData, monitor};
    std::thread jsonwriterThread{std::move(jsonwriter)};
    jsonwriterThread.detach();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    std::thread watchDogThread{Watchdog{watchdogToken, cs2WriterPtr, endpoint, monitor}};
    watchDogThread.detach();

    ///////////////////////////////////////////////////////////////////////////////////
    //
    JsonReader jsonReader{cs2WriterPtr, endpoint, watchdogToken, sharedData, monitor};
    std::thread jsonReaderThread{std::move(jsonReader)};
    jsonReaderThread.join();

    return EXIT_SUCCESS;
}
