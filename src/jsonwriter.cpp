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

#include "jsonwriter.h"
#include <moba/log.h>
#include "moba/systemhandler.h"
#include "moba/interfacehandler.h"

JsonWriter::JsonWriter(
    ConcurrentCanQueuePtr dataToAppServer, EndpointPtr endpoint
) : dataToAppServer{dataToAppServer}, endpoint{endpoint} {
}

JsonWriter::~JsonWriter() {
}

void JsonWriter::operator()() const {
    try {
        while(true) {
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
        }
    } catch(const std::exception &e) {
        LOG(moba::ERROR) << "exception occured! <" << e.what() << ">" << std::endl;
    }
}

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
