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

CS2Reader::CS2Reader(CS2WriterPtr cs2writer, BrakeVectorPtr brakeVector, EndpointPtr endpoint) :
cs2writer{cs2writer}, brakeVector{brakeVector}, endpoint{endpoint}, fd_read{-1} {
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

CS2CanCommand CS2Reader::read() const {
    CS2CanCommand data;
    memset((void*)&data, '\0', sizeof(data));

    struct sockaddr_in s_addr_other;
    socklen_t slen = sizeof(s_addr_other);

    if(::recvfrom(fd_read, (void*)&data, sizeof(data), 0, (struct sockaddr *) &s_addr_other, &slen) == -1) {
        throw CS2ConnectorException("::recvfrom returned -1");
    }
    return std::move(data);
}

void CS2Reader::operator()() {
    try {
        while(true) {
            CS2CanCommand data = read();

            if(data.header[1] & 0x01 && data.header[1] == static_cast<uint8_t>(CanCommand::CMD_PING | 0x01)) {
               // return RES_PING;
            }
            if(data.header[1] == static_cast<uint8_t>(CanCommand::CMD_S88_EVENT | 0x01)) {
                s88report(data);
            }

/*
            auto data = dataToAppServer->pop();
            switch(static_cast<CanCommand>(data.header[1])) {
                case CanCommand::CMD_SYSTEM:
                    convertSystemCommand(data);
                    break;

                case CanCommand::CMD_PING:
                    //if(pingSend) {
                    //    pingSend = false;
                        endpoint->sendMsg(InterfaceConnectivityStateChanged{
                            InterfaceConnectivityStateChanged::Connectivity::CONNECTED
                        });
                    //}
                    break;

                default:
                    break;
            }

*/



//            dataToAppServer->push(data);
        }
    } catch(const std::exception &e) {
        LOG(moba::LogLevel::ERROR) << "exception occured! <" << e.what() << ">" << std::endl;
    }
}

void CS2Reader::s88report(const CS2CanCommand &data) {
    //std::uint16_t time = (data.data[2] << 8) | data.data[3];

    std::uint16_t addr = (data.uid[0] << 8) | data.uid[1];
    std::uint16_t contact = (data.uid[2] << 8) | data.uid[3];

    //bool active = static_cast<bool>(data.data[1]);

    //LOG(moba::LogLevel::DEBUG) << "addr " << addr << " contact " << contact << " active " << active << " time " << time << std::endl;
    auto locId = brakeVector->trigger({addr, contact});
    if(locId == BrakeVector::IGNORE_CONTACT) {
        return;
    }
    if(locId != BrakeVector::CONTACT_UNSET) {
//        auto data = setLocSpeed(locId, 0);
        //dataToCS2->push(data);
//        dataToAppServer->push(data);
    }
}
/*
void JsonWriter::convertSystemCommand(const CS2CanCommand &cmd) const {
    switch(static_cast<CanSystemSubCommand>(cmd.data[0])) {
        case CanSystemSubCommand::SYS_SUB_CMD_SYSTEM_GO:
            return endpoint->sendMsg(SystemSetEmergencyStop{false});

        case CanSystemSubCommand::SYS_SUB_CMD_SYSTEM_HALT:
            return;

        case CanSystemSubCommand::SYS_SUB_CMD_SYSTEM_STOP:
            return endpoint->sendMsg(SystemSetEmergencyStop{true});

        default:
            break;

    }
}
*/