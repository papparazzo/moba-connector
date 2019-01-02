/*
 *  Project:    moba-connector
 *
 *  Copyright (C) 2018 Stefan Paproth <pappi-@gmx.de>
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

#include <boost/algorithm/string.hpp>

#include <moba/log.h>
#include <moba/msginterfacehandler.h>

#include "msgloop.h"
#include <functional>

MessageLoop::MessageLoop(
    const std::string &appName, const moba::Version &version,
    moba::MsgEndpointPtr endpoint, std::shared_ptr<Bridge> bridge
) : appName(appName), version(version), msgEndpoint(endpoint), bridge(bridge), sysHandler(endpoint) {
}

void MessageLoop::connect() {
    moba::JsonArrayPtr groups(new moba::JsonArray());
    groups->push_back(moba::toJsonStringPtr("SYSTEM"));

    appId = msgEndpoint->connect(appName, version, groups);
    LOG(moba::NOTICE) << "AppId <" << appId << ">" << std::endl;
}

void MessageLoop::run() {
    moba::MsgInterfaceHandler interfacehandler(msgEndpoint);

    bridge->ping();
    bool pingSend = true;

// TODO: Alle x Sek. ping an CS2 senden

    bridge->addS88CallbackHandler(std::bind(
        &MessageLoop::s88report,
        this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3,
        std::placeholders::_4
    ));

    while(true) {
        try {
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
        } catch(std::exception &e) {
            LOG(moba::ERROR) << e.what() << std::endl;
            bridge->setEmergencyStop();
        }

        moba::MessagePtr msg = msgEndpoint->recieveMsg();
        if(!msg) {
            usleep(50000);
            continue;
        }
        LOG(moba::NOTICE) << "New Message <" << *msg << ">" << std::endl;
        switch(msg->getMsgType()) {
            case moba::Message::MT_ERROR:
                printError(msg->getData());
                break;

            case moba::Message::MT_SYSTEM_NOTICE: {
                printNotice(msg->getData());
                break;
            }

            case moba::Message::MT_CLIENT_SHUTDOWN:
                return;

            case moba::Message::MT_CLIENT_RESET:
                break;

            case moba::Message::MT_HARDWARE_STATE_CHANGED:
                setHardwareState(msg->getData());
                break;

            default:
                break;
        }
    }
}

void MessageLoop::printNotice(moba::JsonItemPtr ptr) {
    moba::JsonObjectPtr o = boost::dynamic_pointer_cast<moba::JsonObject>(ptr);
    LOG(moba::INFO) <<
        moba::castToString(o->at("type")) << ": [" <<
        moba::castToString(o->at("caption")) << "] " <<
        moba::castToString(o->at("text")) << std::endl;
}

void MessageLoop::printError(moba::JsonItemPtr ptr) {
    moba::JsonObjectPtr o = boost::dynamic_pointer_cast<moba::JsonObject>(ptr);

    moba::JsonStringPtr i = boost::dynamic_pointer_cast<moba::JsonString>(o->at("errorId"));
    moba::JsonStringPtr s = boost::dynamic_pointer_cast<moba::JsonString>(o->at("additonalMsg"));
    LOG(moba::WARNING) << "ErrorId <" << *i << "> " << *s << std::endl;
}

void MessageLoop::setHardwareState(moba::JsonItemPtr data) {
    std::string status = moba::castToString(data);

    if(status == "ERROR") {
        return;
    }
    if(status == "EMERGENCY_STOP" || status == "STANDBY") {
        bridge->setEmergencyStop();
    } else {
        bridge->setEmergencyStopClearing();
    }
}

void MessageLoop::s88report(int addr, int contact, bool active, int time) {
    LOG(moba::DEBUG) << "addr " << addr << " contact " << contact << " active " << active << " time " << time << std::endl;
    auto locId = reportVector.trigger({addr, contact});
    if(locId == BrakeVector::IGNORE_CONTACT) {
        return;
    }
    //msgEndpoint->sendMsg();
    if(locId == BrakeVector::CONTACT_UNSET) {
        return;
    }
    bridge->setLocSpeed(locId, 0);
}

