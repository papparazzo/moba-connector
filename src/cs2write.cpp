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

#include "cs2write.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

CS2Write::CS2Write(const std::string &host, int port) : host{host}, port{port}, fd_write{-1} {
}

CS2Write::~CS2Write() {
    if(fd_write != -1) {
        ::close(fd_write);
    }
}

/*
void CS2Write::setLocSpeed(uint32_t locId, uint16_t speed) {
    uint8_t low  = speed & 0xFF;
    uint8_t high = (speed >> 8) & 0xFF;
    send(CanCommand::CMD_LOCO_SPEED, 6, locId, low, high);
}

void CS2Write::setEmergencyStop() {
    //send(CanCommand::CMD_SYSTEM, 5, 0x00000000, CanSystemSubCommand::SYS_SUB_CMD_SYSTEM_STOP);
}

void CS2Write::setEmergencyStopClearing() {
    //send(CanCommand::CMD_SYSTEM, 5, 0x00000000, CanSystemSubCommand::SYS_SUB_CMD_SYSTEM_GO);
}

void CS2Write::ping() {
    send(CanCommand::CMD_PING);
}
*/
void CS2Write::send(CanCommand cmd, uint8_t length, uint32_t uid, uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3) {
    CS2CanRawData raw;
    memset((void*)&raw, '\0', sizeof(raw));

    raw.header[0] = 0x00;
    raw.header[1] = static_cast<uint8_t>(cmd);

    raw.hash[0] = 0x03;
    raw.hash[1] = 0x00;

    raw.length = length;

/*
    raw.uid[0]
    raw.uid[1]
    raw.uid[2]
    raw.uid[3]
*/
    raw.data[0] = data0;
    raw.data[1] = data1;
    raw.data[2] = data2;
    raw.data[3] = data3;

    write(raw);
}

void CS2Write::connect() {

    if((fd_write = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        throw CS2ConnectorException("socket-creation for writing failed");
    }

    ::memset((char *) &s_addr_write, 0, sizeof(s_addr_write));
    s_addr_write.sin_family = AF_INET;
    s_addr_write.sin_port = htons(port);

    if(::inet_aton(host.c_str() , &s_addr_write.sin_addr) == 0) {
        throw CS2ConnectorException("inet_aton failed");
    }
}

void CS2Write::write(const CS2CanRawData &data) {
    if(::sendto(fd_write, (void*)&data, sizeof(data), 0, (struct sockaddr *)&s_addr_write, sizeof(s_addr_write)) == -1) {
        throw CS2ConnectorException("sending failed");
    }
}


