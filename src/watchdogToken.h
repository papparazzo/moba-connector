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

#pragma once
#include <chrono>
#include <memory>
#include <mutex>

class WatchdogToken {

    public:
        WatchdogToken() : pingStartTime{1}, pingResponseTime{0} {
        }

        void pingStarted() {
            std::lock_guard<std::mutex> l{m};
            pingStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            );
        }

        void pingResponsed() {
            std::lock_guard<std::mutex> l{m};
            pingResponseTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            );
        }

        bool isInTime() {
            std::lock_guard<std::mutex> l{m};
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(pingResponseTime - pingStartTime).count();
            if(diff < WatchdogToken::IN_TIME && diff > 0) {
                return true;
            }
            return false;
        }

    protected:
        std::mutex m;
        const int IN_TIME = 300;
        std::chrono::milliseconds pingStartTime;
        std::chrono::milliseconds pingResponseTime;
};

using WatchdogTokenPtr = std::shared_ptr<WatchdogToken>;