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
#include "msgloop.h"

MessageLoop::MessageLoop(
    const std::string &appName, const moba::Version &version,
    moba::MsgEndpointPtr endpoint, std::shared_ptr<Bridge> bridge
) : appName(appName), version(version), msgEndpoint(endpoint), bridge(bridge) {
}

void MessageLoop::connect() {
    moba::JsonArrayPtr groups(new moba::JsonArray());
    groups->push_back(moba::toJsonStringPtr("SYSTEM"));

    appId = msgEndpoint->connect(appName, version, groups);
    LOG(moba::NOTICE) << "AppId <" << appId << ">" << std::endl;

    msgEndpoint->sendMsg(moba::Message::MT_GET_EMERGENCY_STOP_STATE);
}

void MessageLoop::run() {
    while(true) {
/*

        Bridge::SwitchState ss = bridge->checkSwitchState();

        switch(ss) {
            case Bridge::SYS_SUB_CMD_SYSTEM_STOP:
                LOG(moba::INFO) << "EMERGENCY_STOP" << std::endl;
                msgEndpoint->sendMsg(moba::Message::MT_EMERGENCY_STOP);
                break;

            case Bridge::SYS_SUB_CMD_SYSTEM_GO:
                LOG(moba::INFO) << "EMERGENCY_STOP_CLEARING" << std::endl;
                msgEndpoint->sendMsg(moba::Message::MT_EMERGENCY_STOP_CLEARING);
                break;
        }
*/
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

            case moba::Message::MT_EMERGENCY_STOP:
                bridge->setEmergencyStop();
                break;

            case moba::Message::MT_EMERGENCY_STOP_CLEARING:
                bridge->setEmergencyStopClearing();
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

    boost::shared_ptr<moba::JsonNumber<long int> > i =
    boost::dynamic_pointer_cast<moba::JsonNumber<long int> >(o->at("errorId"));
    moba::JsonStringPtr s = boost::dynamic_pointer_cast<moba::JsonString>(o->at("additonalMsg"));
    LOG(moba::WARNING) << "ErrorId <" << i << "> " << s << std::endl;
}
