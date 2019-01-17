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

#include "msgreceiver.h"

#include <moba/log.h>

MsgReceiver::MsgReceiver(CS2WritePtr cs2writer) : cs2writer{cs2writer} {

}

MsgReceiver::~MsgReceiver() {

}

void MsgReceiver::operator()() {
    bool keepRunning = true;

    while(keepRunning) {


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

void MsgReceiver::printNotice(moba::JsonItemPtr ptr) {
    moba::JsonObjectPtr o = boost::dynamic_pointer_cast<moba::JsonObject>(ptr);
    LOG(moba::INFO) <<
        moba::castToString(o->at("type")) << ": [" <<
        moba::castToString(o->at("caption")) << "] " <<
        moba::castToString(o->at("text")) << std::endl;
}

void MsgReceiver::printError(moba::JsonItemPtr ptr) {
    moba::JsonObjectPtr o = boost::dynamic_pointer_cast<moba::JsonObject>(ptr);
    moba::JsonStringPtr i = boost::dynamic_pointer_cast<moba::JsonString>(o->at("errorId"));
    moba::JsonStringPtr s = boost::dynamic_pointer_cast<moba::JsonString>(o->at("additonalMsg"));
    LOG(moba::WARNING) << "ErrorId <" << *i << "> " << *s << std::endl;
}

void MsgReceiver::setHardwareState(moba::JsonItemPtr data) {
    std::string status = moba::castToString(data);

    if(status == "ERROR") {
        return;
    }
    if(status == "EMERGENCY_STOP" || status == "STANDBY") {
//        bridge->setEmergencyStop();
    } else {
//        bridge->setEmergencyStopClearing();
    }
}
