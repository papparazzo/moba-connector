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

#include <iostream>
#include <memory>

#include <moba/helper.h>
#include <moba/msgendpoint.h>
#include <moba/log.h>

#include "config.h"
#include "msgloop.h"
#include "cs2connector.h"

namespace {
    moba::AppData appData = {
        PACKAGE_NAME,
        moba::Version(PACKAGE_VERSION),
        __DATE__,
        __TIME__,
        "localhost",
        7000
    };
}

int main(int argc, char *argv[]) {
    moba::setCoreFileSizeToULimit();

    std::shared_ptr<CS2Connector> connector(new CS2Connector());
    connector->connect("192.168.178.38");
    std::shared_ptr<Bridge> bridge(new Bridge(connector));
    moba::MsgEndpointPtr endpoint(new moba::MsgEndpoint(appData.host, appData.port));

    while(true) {
        try {
            MessageLoop loop(appData.appName, appData.version, endpoint, bridge);
            loop.connect();
            loop.run();
            return EXIT_SUCCESS;
        } catch(moba::MsgEndpointException &e) {
            LOG(moba::WARNING) << e.what() << std::endl;
            sleep(4);
        }
    }
}
