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

#include <thread>
#include <utility>

#include "moba/systemmessages.h"
#include "moba/interfacemessages.h"
#include "moba/cs2utils.h"
#include "moba/configloklistreader.h"
#include "moba/configreader.h"

JsonWriter::JsonWriter(CS2ReaderPtr cs2reader, CS2WriterPtr cs2writer, EndpointPtr endpoint, WatchdogTokenPtr watchdogToken, SharedDataPtr sharedData):
cs2reader{std::move(cs2reader)}, cs2writer{std::move(cs2writer)}, endpoint{std::move(endpoint)}, watchdogToken{std::move(watchdogToken)}, sharedData{std::move(sharedData)} {
}

void JsonWriter::operator()() {
    // TODO: Sollte die Liste nach einem Reset neu eingelesen werden?
    //       Bzw. was, wenn neue Lok aufgegleist wurde?
    readFunctionList();
    
    while(true) {
        try {
            CS2CanCommand data = cs2reader->read();

            if(data.header[1] & 0x01 && data.header[1] == static_cast<uint8_t>(CanCommand::CMD_PING | 0x01)) {
                watchdogToken->pingResponded();
                continue;
            }
            if(s88report(data)) {
                continue;
            }
            if(systemCommands(data)) {
                continue;
            }
            if(controlLocoCommands(data)) {
                continue;
            }

            controlSwitch(data);
        } catch(const std::exception &e) {
            std::cerr << "exception occurred! <" << e.what() << ">" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds{500});
        }
    }
}

bool JsonWriter::s88report(const CS2CanCommand &cmd) {
    if(cmd.header[1] != static_cast<uint8_t>(CanCommand::CMD_S88_EVENT | 0x01)) {
        return false;
    }

    auto module = cmd.getWordAt0();
    auto contact = cmd.getWordAt2();

    auto time = cmd.getWordAt6();

    bool active = static_cast<bool>(cmd.data[4]);

    std::cerr << "Feedback module <" << module << "> contact <" << contact << ">" << std::endl;

    auto locId = sharedData->brakeVector.trigger({module, contact});
    if(locId != BrakeVector::IGNORE_CONTACT) {
        cs2writer->send(::setLocSpeed(locId, 0));
        endpoint->sendMsg(InterfaceSetLocoSpeed{static_cast<std::uint32_t>(locId), 0});
    }

    endpoint->sendMsg(InterfaceContactTriggered{ContactTriggerData{module, contact, active, time}});
    return true;
}

bool JsonWriter::systemCommands(const CS2CanCommand &cmd) const {
    if(static_cast<CanCommand>(cmd.header[1]) != CanCommand::CMD_SYSTEM) {
        return false;
    }

    switch(static_cast<CanSystemSubCommand>(cmd.data[4])) {
        case CanSystemSubCommand::SYS_SUB_CMD_SYSTEM_GO:
            endpoint->sendMsg(SystemReleaseEmergencyStop{});
            return true;

        case CanSystemSubCommand::SYS_SUB_CMD_SYSTEM_STOP:
            endpoint->sendMsg(SystemTriggerEmergencyStop{SystemTriggerEmergencyStop::EmergencyTriggerReason::CENTRAL_STATION});
            return true;

        default:
            return false;
    }
}

bool JsonWriter::controlLocoCommands(const CS2CanCommand &cmd) const {
    switch(static_cast<CanCommand>(cmd.header[1])) {
        case CanCommand::CMD_LOCO_DIRECTION:
            endpoint->sendMsg(InterfaceSetLocoDirection{cmd.getUID(), cmd.data[4]});
            return true;

        case CanCommand::CMD_LOCO_SPEED:
            endpoint->sendMsg(InterfaceSetLocoSpeed{cmd.getUID(), cmd.getWordAt4()});
            return true;

        default:
            return false;
    }
}

bool JsonWriter::controlSwitch(const CS2CanCommand &cmd) const {
    switch(static_cast<CanCommand>(cmd.header[1])) {
        case CanCommand::CMD_SET_SWITCH:
            if(sharedData->automatic) {
                cs2writer->send(::setEmergencyStop());
                endpoint->sendMsg(SystemTriggerEmergencyStop{SystemTriggerEmergencyStop::EmergencyTriggerReason::SELF_ACTING_BY_EXTERN_SWITCHING});
            }
            return true;

        default:
            return false;
    }
}

void JsonWriter::readFunctionList() {
    cs2writer->send(::getLokList());
    auto cfgReader = std::make_shared<ConfigLoklistReader>();

    ConfigReader configReader{};
    configReader.addHandler(cfgReader);

    while(
        configReader.handleCanCommand(cs2reader->read()) != 
        ConfigReader::HANDLED_AND_FINISHED
    ); // no op
}
