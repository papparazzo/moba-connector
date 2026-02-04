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
#include "moba/cs2utils.h"
#include "moba/messagingmessages.h"

#include <thread>

Watchdog::Watchdog(CS2WriterPtr cs2writer, EndpointPtr endpoint, MonitorPtr monitor, const PingSettings ping_settings):
    cs2writer{std::move(cs2writer)},
    endpoint{std::move(endpoint)},
    monitor{std::move(monitor)},
    ping_settings{ping_settings},
    thread(&Watchdog::operator(), this)
{
}

Watchdog::~Watchdog() {
    thread.request_stop();
    cv.notify_all();
}

void Watchdog::ping_response() {
    std::lock_guard lock(mutex);
    pong_received = true;
    cv.notify_all();
}

void Watchdog::synchronize_start() {
    synchronize = true;
}

void Watchdog::operator()(const std::stop_token &st) {
    std::unique_lock lock(mutex);

    bool connected = false;

    while(!st.stop_requested()) {
        try {
            std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

            cs2writer->send(ping());
            pong_received = false;

            cv.wait_for(lock, ping_settings.timeout, [&]{ return pong_received; });

            auto diff = std::chrono::steady_clock::now() - start_time; // std::chrono::steady_clock::duration
            if(pong_received) {
                std::stringstream ss;
                ss << "Watchdog ping received. Diff: " << std::chrono::duration_cast<std::chrono::milliseconds>(diff);
                monitor->appendAction(moba::LogLevel::NOTICE, ss.str());
            }

            if(synchronize) {
                endpoint->sendMsg(InterfaceConnected{true});
                synchronize = false;
            } else if (!pong_received && connected) {
                monitor->appendAction(moba::LogLevel::CRITICAL, "Watchdog timeout!");
                endpoint->sendMsg(InterfaceConnectionLost{});
                connected = false;
            } else if (pong_received && !connected) {
                endpoint->sendMsg(InterfaceConnected{false});
                connected = true;
            }
        } catch(const std::exception &e) {
            monitor->printException("Watchdog::operator()()", e.what());
        }
        if(connected && !st.stop_requested()) {
            std::this_thread::sleep_for(ping_settings.interval);
        }
    }
}
