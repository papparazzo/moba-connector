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

#include "watchdog.h"
#include "moba/interfacemessages.h"
#include <thread>
#include "moba/cs2utils.h"
#include "moba/messagingmessages.h"
#include <moba-common/loggerprefix.h>

Watchdog::Watchdog(
    WatchdogTokenPtr watchdogToken, CS2WriterPtr cs2writer, EndpointPtr endpoint
): watchdogToken{std::move(watchdogToken)}, cs2writer{std::move(cs2writer)}, endpoint{std::move(endpoint)}, lastState{ConnectState::ERROR} {
}

void Watchdog::operator()() {
    while(true) {
        try {
            cs2writer->send(ping());
            watchdogToken->pingStarted();

            std::this_thread::sleep_for(std::chrono::milliseconds{30});

            if(const auto tokenState = watchdogToken->getTokenState(); tokenState == WatchdogToken::TokenState::SYNCHRONIZE) {
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
            endpoint->sendMsg(MessagingNotifyIncident{IncidentData{
                IncidentLevel::ERROR,
                IncidentType::EXCEPTION,
                "Watchdog Exception",
                e.what(),
                "Watchdog::operator()()"
            }});
            std::cerr << moba::LogLevel::CRITICAL << "Watchdog exception: <" << e.what() << ">" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
    }
}
