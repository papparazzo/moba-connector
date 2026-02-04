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

#pragma once

#include <condition_variable>
#include <stop_token>
#include <thread>

#include "monitor.h"
#include "moba/cs2writer.h"
#include "moba/endpoint.h"

class Watchdog final {
public:
    struct PingSettings {
        std::chrono::milliseconds timeout{30};
        std::chrono::milliseconds interval{500};
    };

    Watchdog(CS2WriterPtr cs2writer, EndpointPtr endpoint, MonitorPtr monitor, PingSettings ping_settings);

    Watchdog(const Watchdog&) = delete;
    Watchdog(Watchdog&&) = delete;
    Watchdog& operator=(const Watchdog&) = delete;

    ~Watchdog();

    void ping_response();

    void synchronize_start();

private:
    void operator()(const std::stop_token &st);

    CS2WriterPtr     cs2writer;
    EndpointPtr      endpoint;
    MonitorPtr       monitor;
    PingSettings     ping_settings;

    std::jthread thread;
    std::mutex mutex;
    std::condition_variable cv;

    volatile bool pong_received{false};
    std::atomic_bool synchronize;
};

using WatchdogPtr = std::shared_ptr<Watchdog>;
