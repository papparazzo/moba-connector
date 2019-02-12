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

class CS2Read : private boost::noncopyable {
    public:
        static const int DEFAULT_PORT_READ = 15730;

        CS2Read(
            ConcurrentCanQueuePtr queue, const std::string &host, int port = CS2Read::DEFAULT_PORT_READ
        ) : queue{queue}, host{host}, port{port}, fd_read{-1}  {
        }

        virtual ~CS2Read() {
            if(fd_read != -1) {
                ::close(fd_read);
            }
        }

        void connect();
        CS2CanRawData read();

        void recieveCanData();

    protected:
        int fd_read;
        std::string host;
        int port;
        ConcurrentCanQueuePtr queue;
};

using CS2ReadPtr = std::shared_ptr<CS2Read>;