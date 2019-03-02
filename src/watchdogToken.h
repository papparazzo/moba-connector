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

class WatchdogToken {

    public:
        WatchdogToken() : pingStartTime{1}, pingResponseTime{0} {
        }

        void pingStarted() {
            pingStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            );
        }

        void pingResponsed() {
            pingResponseTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            );
        }

        bool isInTime() {
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(pingResponseTime - pingStartTime).count();

            if(diff < WatchdogToken::IN_TIME) {
                return true;
            }
            return false;
        }

    protected:
        const int IN_TIME = 300;
        std::chrono::milliseconds pingStartTime;
        std::chrono::milliseconds pingResponseTime;
};

using WatchdogTokenPtr = std::shared_ptr<WatchdogToken>;