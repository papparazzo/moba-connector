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


#include "argumentparser.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <getopt.h>

bool ArgumentParser::parseArguments(
    const int argc, char *argv[], moba::AppData &appData, CS2ContactData &cs2ContactData, Watchdog::PingSettings &pingSettings, bool &debug
) const {
    static option longOptions[] = {
        {"cs2-host",      required_argument, nullptr, 'c'},
        {"debug",         no_argument,       nullptr, 'd'},
        {"cs2-port-in",   required_argument, nullptr, 'i'},
        {"cs2-port-out",  required_argument, nullptr, 'o'},
        {"ping-timeout",  required_argument, nullptr, 't'},
        {"ping-interval", required_argument, nullptr, 'n'},
        {"help",          no_argument,       nullptr, 'h'},
        {"version",       no_argument,       nullptr, 'v'},
        {nullptr,         0,                 nullptr, 0 }
    };

    int optionIndex = 0;

    while(true) {
        const int c = getopt_long(argc, argv, "hvc:i:o:dt:n:", longOptions, &optionIndex);
        if(c == -1) {
            break;
        }

        switch(c) {
            case 'd':
                debug = true;
                break;

            case 'h':
                printHelp(basename(argv[0]), cs2ContactData, pingSettings);
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

            case 't':
                pingSettings.timeout = parseMilliseconds(optarg);
                break;

            case 'n':
                pingSettings.interval = parseMilliseconds(optarg);
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

std::chrono::milliseconds ArgumentParser::parseMilliseconds(const std::string &s) {
    long long v = 0;
    try {
        v = std::stoll(std::string{s});
    } catch (...) {
        throw std::invalid_argument(std::string{"invalid milliseconds value: '"} + s + "'");
    }
    if (v < 0) {
        throw std::invalid_argument(std::string{"milliseconds must be >= 0: '"} + s + "'");
    }
    return std::chrono::milliseconds{v};
}

void ArgumentParser::printHelp(const std::string &appName, const CS2ContactData &cs2ContactData, const Watchdog::PingSettings &pingSettings) {
    std::cout
        << appName << " [host] [port]" << std::endl
        << std::endl
        << "-d, --debug         enables debug output" << std::endl
        << "-h, --help          shows this help" << std::endl
        << "-v, --version       shows version-info" << std::endl
        << "-c, --cs2-host      host of the CentralStation (default: " << cs2ContactData.host << ")" << std::endl
        << "-i, --cs2-port-in   port of the CentralStation for incoming messages (default: " << cs2ContactData.portIn << ")" << std::endl
        << "-o, --cs2-port-out  port of the CentralStation for outgoing messages (default: " << cs2ContactData.portOut << ")" << std::endl
        << "-t, --ping-timeout  watchdog ping timeout in ms (default: " << pingSettings.timeout << ")" << std::endl
        << "-n, --ping-interval watchdog ping interval in ms (default: " << pingSettings.interval << ")" << std::endl;
}
