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

CS2Write::CS2Write() : fd_write(-1) {
}

CS2Write::~CS2Write() {
    if(fd_write != -1) {
        ::close(fd_write);
    }
}

void CS2Write::connect(const std::string &host, int port) {

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

