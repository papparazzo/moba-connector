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

#include "concurrentqueue.h"

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

enum class CanCommand {
    CMD_SYSTEM                                  = 0x00,
    CMD_LOCO_DISCOVERY                          = 0x02,
    CMD_MFX_BIND                                = 0x04,
    CMD_MFX_VERIFY                              = 0x06,
    CMD_LOCO_SPEED                              = 0x08,
    CMD_LOCO_DIRECTION                          = 0x0A,
    CMD_LOCO_FUNCTION                           = 0x0C,
    CMD_READ_CONFIG                             = 0x0E,
    CMD_WRITE_CONFIG                            = 0x10,
    CMD_SET_SWITCH                              = 0x16,
    CMD_ATTACHMENTS_CONFIG                      = 0x18,
    CMD_S88_POLLING                             = 0x20,
    CMD_S88_EVENT                               = 0x22,
    CMD_SX1_EVENT                               = 0x24,
    CMD_PING                                    = 0x30,
    CMD_UPDATE_OFFER                            = 0x32,
    CMD_READ_CONFIG_DATA                        = 0x34,
    CMD_BOOTLOADER_CAN                          = 0x36,
    CMD_BOOTLOADER_TRACK                        = 0x38,
    CMD_STATUS_DATA_CONFIGURATION               = 0x3A,
    CMD_CONFIG_DATA_QUERY                       = 0x40,
    CMD_CONFIG_DATA_STREAM                      = 0x42,
    CMD_60128_CONNECT_6021_DATA_STREAM          = 0x44,
};

inline CanCommand operator |(CanCommand a, CanCommand b) {
    return static_cast<CanCommand>(static_cast<int>(a) | static_cast<int>(b));
}

inline CanCommand operator |(CanCommand a, int b) {
    return static_cast<CanCommand>(static_cast<int>(a) | b);
}

enum class CanSystemSubCommand {
    SYS_SUB_CMD_SYSTEM_STOP = 0x00,
    SYS_SUB_CMD_SYSTEM_GO   = 0x01,
    SYS_SUB_CMD_SYSTEM_HALT = 0x02
};

using ConcurrentCanQueuePtr = std::shared_ptr<ConcurrentQueue<CS2CanRawData>>;