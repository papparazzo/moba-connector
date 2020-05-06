/*
 *  Project:    moba-connector
 *
 *  Copyright (C) 2020 Stefan Paproth <pappi-@gmx.de>
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

#include <moba-common/log.h>

#include "moba/systemmessage.h"
#include "moba/interfacemessage.h"
#include "moba/cs2utils.h"

JsonWriter::JsonWriter(CS2ReaderPtr cs2reader, CS2WriterPtr cs2writer, EndpointPtr endpoint, WatchdogTokenPtr watchdog, BrakeVectorPtr brakeVector) :
cs2reader{cs2reader}, cs2writer{cs2writer}, endpoint{endpoint}, brakeVector{brakeVector}, watchdog{watchdog} {
}

void JsonWriter::operator()() {
    try {
        while(true) {
            CS2CanCommand data = cs2reader->read();

            if(data.header[1] & 0x01 && data.header[1] == static_cast<uint8_t>(CanCommand::CMD_PING | 0x01)) {
                watchdog->pingResponsed();
                continue;
            }
            if(data.header[1] == static_cast<uint8_t>(CanCommand::CMD_S88_EVENT | 0x01)) {
                s88report(data);
                continue;
            }

            switch(static_cast<CanCommand>(data.header[1])) {
                case CanCommand::CMD_SYSTEM:
                    convertSystemCommand(data);
                    continue;

                default:
                    break;
            }
        }
    } catch(const std::exception &e) {
        LOG(moba::common::LogLevel::ERROR) << "exception occured! <" << e.what() << ">" << std::endl;
    }
}

void JsonWriter::s88report(const CS2CanCommand &data) {
    std::uint16_t time = (data.data[2] << 8) | data.data[3];

    std::uint16_t addr = (data.uid[0] << 8) | data.uid[1];
    std::uint16_t contact = (data.uid[2] << 8) | data.uid[3];

    bool active = static_cast<bool>(data.data[1]);

    LOG(moba::common::LogLevel::DEBUG) << "addr " << addr << " contact " << contact << " active " << active << " time " << time << std::endl;
    auto locId = brakeVector->trigger({addr, contact});
    if(locId == BrakeVector::IGNORE_CONTACT) {
        return;
    }
    if(locId == BrakeVector::CONTACT_UNSET) {
        return;
    }
    cs2writer->send(setLocSpeed(locId, 0));
    endpoint->sendMsg(InterfaceContactTriggered{ContactTrigger{addr, contact, active, time}});
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

