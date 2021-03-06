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
#include "moba/interfacemessages.h"
#include <moba-common/log.h>
#include <thread>
#include "moba/cs2utils.h"

Watchdog::Watchdog(
    WatchdogTokenPtr watchdogToken, CS2WriterPtr cs2writer, EndpointPtr endpoint
) : watchdogToken{watchdogToken}, cs2writer{cs2writer}, endpoint{endpoint}, lastState{ConnectState::ERROR} {
}

Watchdog::~Watchdog() {
}

void Watchdog::operator()() {
    while(true) {
        try {
            cs2writer->send(ping());
            watchdogToken->pingStarted();

            std::this_thread::sleep_for(std::chrono::milliseconds{30});

            auto tokenState = watchdogToken->getTokenState();
            if(tokenState == WatchdogToken::TokenState::SYNCHRONIZE) {
                endpoint->sendMsg(InterfaceConnectivityStateChanged{lastState});
                watchdogToken->synchronizeFinish();
            } else if(tokenState == WatchdogToken::TokenState::CONNECTED && lastState == ConnectState::ERROR) {
                endpoint->sendMsg(InterfaceConnectivityStateChanged{ConnectState::CONNECTED});
                lastState = ConnectState::CONNECTED;
            } else if(tokenState == WatchdogToken::TokenState::ERROR && lastState == ConnectState::CONNECTED) {
                endpoint->sendMsg(InterfaceConnectivityStateChanged{ConnectState::ERROR});
                lastState = ConnectState::ERROR;
            }
        } catch(const std::exception &e) {
            LOG(moba::common::LogLevel::ERROR) << "exception occured! <" << e.what() << ">" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
    }
}
