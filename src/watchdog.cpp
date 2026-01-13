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

Watchdog::Watchdog(
    WatchdogTokenPtr watchdogToken, CS2WriterPtr cs2writer, EndpointPtr endpoint, MonitorPtr monitor
): watchdogToken{std::move(watchdogToken)}, cs2writer{std::move(cs2writer)},
   endpoint{std::move(endpoint)}, monitor{std::move(monitor)}, lastState{Connectivity::ERROR} {
}

void Watchdog::operator()() {
    while(true) {
        try {
            cs2writer->send(ping());
            watchdogToken->pingStarted();

            std::this_thread::sleep_for(std::chrono::milliseconds{30});

            if(const auto tokenState = watchdogToken->getTokenState(); tokenState == WatchdogToken::TokenState::SYNCHRONIZE) {
                endpoint->sendMsg(InterfaceConnected{true});
                watchdogToken->synchronizeFinish();
            } else if(tokenState == WatchdogToken::TokenState::CONNECTED && lastState == Connectivity::ERROR) {
                endpoint->sendMsg(InterfaceConnected{false});
                lastState = Connectivity::CONNECTED;
            } else if(tokenState == WatchdogToken::TokenState::ERROR && lastState == Connectivity::CONNECTED) {
                endpoint->sendMsg(InterfaceConnectionLost{});
                lastState = Connectivity::ERROR;
            }
        } catch(const std::exception &e) {
            endpoint->sendMsg(MessagingSendNotification{NotificationData{
                NotificationLevel::ERROR,
                NotificationType::EXCEPTION,
                "Watchdog Exception",
                e.what(),
                "Watchdog::operator()()"
            }});
            monitor->printException("Watchdog::operator()()", e.what());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{500});
    }
}
