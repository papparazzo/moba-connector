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

#include "cs2connector.h"

#include <boost/noncopyable.hpp>
#include <string>
#include <memory>

class CS2Write : private boost::noncopyable {
    public:
        static const int DEFAULT_PORT_WRITE  = 15731;

        CS2Write();
        virtual ~CS2Write();

        void connect(const std::string &host, int port = CS2Write::DEFAULT_PORT_WRITE);
        void write(const CS2CanRawData &data);

        void setLocSpeed(uint32_t locId, uint16_t speed);
        void setEmergencyStop();
        void setEmergencyStopClearing();
        void ping();

    protected:
        int fd_write;
        struct sockaddr_in s_addr_write;
        void send(
            CanCommand cmd,
            uint8_t length = 0x00,
            uint32_t uid = 0x000000,
            uint8_t data0 = 0x00,
            uint8_t data1 = 0x00,
            uint8_t data2 = 0x00,
            uint8_t data3 = 0x00
        );
};

using CS2WritePtr = std::shared_ptr<CS2Write>;