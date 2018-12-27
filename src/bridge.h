/*
 *  Project:    moba-environment
 *
 *  Copyright (C) 2016 Stefan Paproth <pappi-@gmx.de>
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

#include <memory>
#include <boost/noncopyable.hpp>
#include <moba/ipc.h>
#include <functional>

#include "cs2connector.h"
#include "reportvector.h"

class Bridge : private boost::noncopyable {
    public:
        enum ResponseCode {
            RES_EMPTY,
            RES_UNKNOWN,
            RES_SYSTEM_STOP,
            RES_SYSTEM_GO,
            RES_SYSTEM_HALT,
            RES_PING,
        };

        Bridge(std::shared_ptr<CS2Connector> connector) : connector(connector) {
        }
        void setEmergencyStop();
        void setEmergencyStopClearing();
        void ping();

        ResponseCode recieveCanData();

        std::shared_ptr<CS2Connector> connector;

        void addS88CallbackHandler(std::function<void(int, int, bool, bool, int)> s88callback) {
            this->s88callback = s88callback;
        }

    protected:
        enum CanCommand {
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

        enum SystemSubCommand {
            SYS_SUB_CMD_SYSTEM_STOP = 0x00,
            SYS_SUB_CMD_SYSTEM_GO   = 0x01,
            SYS_SUB_CMD_SYSTEM_HALT = 0x02
        };

        void send(
            uint8_t cmd,
            uint8_t length = 0x00,
            uint32_t uid = 0x000000,
            uint8_t data0 = 0x00,
            uint8_t data1 = 0x00,
            uint8_t data2 = 0x00,
            uint8_t data3 = 0x00
        );

        std::function<void(int, int, bool, bool, int)> s88callback;

        ReportVector reportVector;

};