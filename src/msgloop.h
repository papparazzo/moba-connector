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

#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <moba/msgendpoint.h>
#include <moba/msgsystemhandler.h>

#include "bridge.h"
#include "reportvector.h"

class MessageLoop : private boost::noncopyable {
    public:
        MessageLoop(
            const std::string &appName,
            const moba::Version &version,
            moba::MsgEndpointPtr,
            std::shared_ptr<Bridge> bridge
        );

        void run();
        void connect();
        void s88report(int addr, int contact, bool active, int time);

    protected:
        void printError(moba::JsonItemPtr ptr);
        void printNotice(moba::JsonItemPtr ptr);
        void setHardwareState(moba::JsonItemPtr data);

        moba::MsgEndpointPtr msgEndpoint;
        long appId;
        std::string appName;
        moba::Version version;

        std::shared_ptr<Bridge> bridge;
        moba::MsgSystemHandler sysHandler;

        ReportVector reportVector;
};
