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
 *  along with this program. If not, see <https://www.gnu.org/licenses/agpl.txt>.
 *
 */

#include "jsonwriter.h"

#include <thread>
#include <utility>

#include "moba/systemmessages.h"
#include "moba/cs2utils.h"
#include "moba/configloklistreader.h"
#include "moba/configreader.h"
#include "moba/messagingmessages.h"

// TODO Consider renaming into CS2Reader instead of JsonWriter
JsonWriter::JsonWriter(
    CS2ReaderPtr cs2reader,
    CS2WriterPtr cs2writer,
    EndpointPtr endpoint,
    WatchdogTokenPtr watchdogToken,
    SharedDataPtr sharedData,
    MonitorPtr monitor
): cs2reader{std::move(cs2reader)}, cs2writer{std::move(cs2writer)}, endpoint{std::move(endpoint)},
watchdogToken{std::move(watchdogToken)}, sharedData{std::move(sharedData)}, monitor{std::move(monitor)} {
}

void JsonWriter::operator()() const {
    // TODO: Sollte die Liste nach einem Reset neu eingelesen werden?
    //       Bzw. was, wenn neue Lok aufgegleist wurde?
    readFunctionList();
    
    while(true) {
        try {
            CS2CanCommand data;
            cs2reader->read(data);

            if(data.isResponse() && data.header[1] == static_cast<uint8_t>(CanCommand::CMD_PING | 0x01)) {
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
            if (controlSwitch(data)) {
                continue;
            }
        } catch(const std::exception &e) {
            monitor->printException("JsonWriter::operator()()", e.what());
            emergencyStop(e.what());
        }
    }
}

bool JsonWriter::s88report(const CS2CanCommand &data) const {
    if(data.header[1] != static_cast<uint8_t>(CanCommand::CMD_S88_EVENT | 0x01)) {
        return false;
    }

    const auto module = data.getWordAt0();
    const auto contact = data.getWordAt2();

    const auto time = data.getWordAt6();

    const bool active = static_cast<bool>(data.data[4]);

    monitor->feedbackContactTriggered(module, contact, time, active);

    // FIXME: Hier eventuell auch noch active == false berücksichtigen...
    if(active) {
        sharedData->actionListHandler.trigger(ContactData{module, contact});
    }
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
            monitor->locCommandsTriggered("Fahrtrichtungswechsel", cmd.getUID(), cmd.data[4]);
            return true;

        case CanCommand::CMD_LOCO_SPEED:
            monitor->locCommandsTriggered("Geschwindigkeitsänderung", cmd.getUID(), cmd.getWordAt4());
            return true;

        default:
            return false;
    }
}

bool JsonWriter::controlSwitch(const CS2CanCommand &cmd) const {
    switch(static_cast<CanCommand>(cmd.header[1])) {
        case CanCommand::CMD_SET_SWITCH:
            if(sharedData->automatic) {
                cs2writer->send(setEmergencyStop());
                endpoint->sendMsg(
                    SystemTriggerEmergencyStop{
                        SystemTriggerEmergencyStop::EmergencyTriggerReason::SELF_ACTING_BY_EXTERN_SWITCHING,
                        "address <" + std::to_string(cmd.getDoubleWordAt0()) + "> (" + std::to_string(cmd.getWordAt4()) + ") triggered"
                    }
                );
            }
            return true;

        default:
            return false;
    }
}

void JsonWriter::readFunctionList() const {
    cs2writer->send(getLokList());
    const auto cfgReader = std::make_shared<ConfigLoklistReader>(sharedData->locomotives);

    ConfigReader configReader{};
    configReader.addHandler(cfgReader);

    CanCommandHandlerInterface::HandlerReturn result = ConfigReader::NOT_HANDLED;

    do {
        CS2CanCommand data;
        cs2reader->read(data);
        result = configReader.handleCanCommand(data);
    } while(result != ConfigReader::HANDLED_AND_FINISHED);
}

void JsonWriter::emergencyStop(const std::string &what) const {
    try {
        cs2writer->send(setEmergencyStop());

        endpoint->sendMsg(
            SystemTriggerEmergencyStop{
                SystemTriggerEmergencyStop::EmergencyTriggerReason::SOFTWARE_ERROR,
                "exception <" + what + "> occurred while reading CAN bus. Emergency stop set."
            }
        );
        endpoint->sendMsg(MessagingSendNotification{NotificationData{
            NotificationLevel::ERROR,
            NotificationType::EXCEPTION,
            "JsonWriter Exception",
            what,
            "JsonWriter::operator()()"
        }});
    } catch(const std::exception &e) {
        monitor->printException("JsonWriter::emergencyStop()", e.what());
    }
}