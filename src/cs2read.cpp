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

#include "cs2read.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

CS2Read::CS2Read(
    ConcurrentCanQueuePtr queue, const std::string &host, int port) : queue{queue}, host{host}, port{port}, fd_read{-1}  {
}

CS2Read::~CS2Read() {
    if(fd_read != -1) {
        ::close(fd_read);
    }
}

void CS2Read::connect() {
    struct sockaddr_in s_addr_read;

    if((fd_read = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        throw CS2ConnectorException("socket-creation for reading failed");
    }

    ::memset((char *) &s_addr_read, 0, sizeof(s_addr_read));
    s_addr_read.sin_family = AF_INET;
    s_addr_read.sin_port = htons(port);
    s_addr_read.sin_addr.s_addr = htonl(INADDR_ANY);

    if(::bind(fd_read, (struct sockaddr*)&s_addr_read, sizeof(s_addr_read)) == -1) {
        throw CS2ConnectorException("binding failed");
    }
}

void CS2Read::read() const {
    CS2CanRawData data;
    memset((void*)&data, '\0', sizeof(data));

    struct sockaddr_in s_addr_other;
    socklen_t slen = sizeof(s_addr_other);

    if(::recvfrom(fd_read, (void*)&data, sizeof(data), 0, (struct sockaddr *) &s_addr_other, &slen) == -1) {
        throw CS2ConnectorException("::recvfrom returned -1");
    }
    queue->push(std::move(data));
}

void CS2Read::operator()() const {
    try {
        while(true) {
            read();
        }
    } catch(const std::exception &e) {

    }
}
