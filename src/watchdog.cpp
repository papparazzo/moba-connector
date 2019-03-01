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

#include "watchdog.h"
#include "moba/interfacehandler.h"
#include <moba/log.h>
#include <thread>

Watchdog::Watchdog(
    WatchdogTokenPtr watchdog, CS2WriterPtr cs2writer, EndpointPtr endpoint
) : watchdog{watchdog}, cs2writer{cs2writer}, endpoint{endpoint}, lastState{ConnectState::ERROR} {
}

Watchdog::~Watchdog() {
}

void Watchdog::operator()() const {
    try {
        while(true) {
            cs2writer->send(ping());
            watchdog->pingStarted();

            std::this_thread::sleep_for(std::chrono::milliseconds{30});

            auto inTime = watchdog->isInTime();
            if(inTime && lastState == ConnectState::ERROR) {
                lastState == ConnectState::CONNECTED;
                endpoint->sendMsg(InterfaceConnectivityStateChanged{lastState});
            } else if(!inTime && lastState == ConnectState::CONNECTED) {
                lastState == ConnectState::ERROR;
                endpoint->sendMsg(InterfaceConnectivityStateChanged{lastState});
            }

            if(lastState == ConnectState::ERROR) {
                std::this_thread::sleep_for(std::chrono::milliseconds{25});
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds{2500});
            }
        }
    } catch(const std::exception &e) {
        LOG(moba::LogLevel::ERROR) << "exception occured! <" << e.what() << ">" << std::endl;
    }

}
