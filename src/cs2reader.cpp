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

#include "cs2reader.h"

#include <moba/log.h>

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

CS2Reader::CS2Reader(ConcurrentCanQueuePtr dataToCS2, ConcurrentCanQueuePtr dataToAppServer, BrakeVectorPtr brakeVector) :
dataToCS2{dataToCS2}, dataToAppServer{dataToAppServer}, brakeVector{brakeVector}, fd_read{-1} {
}

CS2Reader::~CS2Reader() {
    if(fd_read != -1) {
        ::close(fd_read);
    }
}

void CS2Reader::connect(const std::string &host, int port) {
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

void CS2Reader::read() const {
    CS2CanRawData data;
    memset((void*)&data, '\0', sizeof(data));

    struct sockaddr_in s_addr_other;
    socklen_t slen = sizeof(s_addr_other);

    if(::recvfrom(fd_read, (void*)&data, sizeof(data), 0, (struct sockaddr *) &s_addr_other, &slen) == -1) {
        throw CS2ConnectorException("::recvfrom returned -1");
    }
    dataToCS2->push(std::move(data));
}

void CS2Reader::operator()() const {
    try {
        while(true) {
            read();
        }
    } catch(const std::exception &e) {
        LOG(moba::ERROR) << "exception occured! <" e.what() << ">" << std::endl;
    }
}

void CS2Reader::s88report(int addr, int contact, bool active, int time) {
    //LOG(moba::DEBUG) << "addr " << addr << " contact " << contact << " active " << active << " time " << time << std::endl;
    auto locId = brakeVector->trigger({addr, contact});
    if(locId == BrakeVector::IGNORE_CONTACT) {
        return;
    }
    if(locId != BrakeVector::CONTACT_UNSET) {
    //    bridge->setLocSpeed(locId, 0);
//        dataToCS2->push();
    }
 //   dataToAppServer->push();

}





/*
 *
 *     // Ping Response-Nachricht
    if(data.header[1] & 0x01 && data.header[1] == (CanCommand::CMD_PING | 0x01)) {
        return RES_PING;
    }

    if(data.header[1] == CanCommand::CMD_SYSTEM) {
        switch(data.data[0]) {
            case SYS_SUB_CMD_SYSTEM_GO:
                return RES_SYSTEM_GO;

            case SYS_SUB_CMD_SYSTEM_HALT:
                return RES_SYSTEM_HALT;

            case SYS_SUB_CMD_SYSTEM_STOP:
                return RES_SYSTEM_STOP;
        }
    }
    */ /*
    if(data.header[1] == static_cast<uint8_t>(CanCommand::CMD_S88_EVENT | 0x01) / *&& s88callback* /) {
        std::uint16_t time = (data.data[2] << 8) | data.data[3];

        std::uint16_t addr = (data.uid[0] << 8) | data.uid[1];
        std::uint16_t contact = (data.uid[2] << 8) | data.uid[3];

        //s88callback(addr, contact, static_cast<bool>(data.data[1]), time);
    }


 *
    //bridge->ping();
    bool pingSend = true;

// TODO: Alle x Sek. ping an CS2 senden
    while(true) {

            Bridge::ResponseCode rc = bridge->recieveCanData();
            switch(rc) {
                case Bridge::RES_SYSTEM_STOP:
                    LOG(moba::INFO) << "EMERGENCY_STOP" << std::endl;
                    sysHandler.sendSetEmergencyStop(true);
                    break;

                case Bridge::RES_SYSTEM_GO:
                    LOG(moba::INFO) << "EMERGENCY_STOP_CLEARING" << std::endl;
                    sysHandler.sendSetEmergencyStop(false);
                    break;

                case Bridge::RES_PING:
                    LOG(moba::INFO) << "PING_RESPONSE" << std::endl;
                    if(pingSend) {
                        pingSend = false;
                        interfacehandler.sendConnectivity(moba::MsgInterfaceHandler::CO_CONNECTED);
                    }
                    break;
            }


    }
}

        */
