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

#include "shared.h"

#include <boost/noncopyable.hpp>
#include <string>
#include <memory>

class CS2Writer : private boost::noncopyable {
    public:
        static const int DEFAULT_PORT_WRITE  = 15731;

        CS2Writer(const std::string &host, int port = CS2Writer::DEFAULT_PORT_WRITE);
        virtual ~CS2Writer() noexcept;

        void connect();
        void send(const CS2CanRawData &data);

    protected:
        std::string host;
        int port;

        int fd_write;
        struct sockaddr_in s_addr_write;
};
