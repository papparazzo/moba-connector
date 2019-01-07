/*
 *  Project:    moba-connector
 *
 *  Copyright (C) 2018 Stefan Paproth <pappi-@gmx.de>
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

#include <boost/noncopyable.hpp>
#include <string>
#include <exception>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>

class CS2ConnectorException : public std::exception {

    public:
        virtual ~CS2ConnectorException() noexcept {
        }

        CS2ConnectorException(const std::string &what) {
            what__ = what;
        }

        virtual const char* what() const noexcept {
            return what__.c_str();
        }

    private:
        std::string what__;
};

 struct CS2CanRawData {
    uint8_t header[2];
    uint8_t hash[2];
    uint8_t length;
    uint8_t uid[4];
    uint8_t data[4];
};

class CS2Connector : private boost::noncopyable {
    public:
        struct RawData {
            uint8_t header[2];
            uint8_t hash[2];
            uint8_t length;
            uint8_t uid[4];
            uint8_t data[4];
        };

        CS2Connector();
        virtual ~CS2Connector();

        void connect(const std::string &host);
        bool recieveData(RawData &data, time_t timeoutSec = 0);
        void sendData(const RawData &data);

        std::string getCommmandAsString(int cmd);
        std::string getSystemSubCommand(int subCmd);

    protected:
        static const int PORT_READ   = 15730;
        static const int PORT_WRITE  = 15731;

        static const int MSG_HANDLER_TIME_OUT_USEC = 0;

        int fd_read;
        int fd_write;
        struct sockaddr_in s_addr_write;
};
