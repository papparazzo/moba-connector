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

#pragma once

#include "cs2cancommand.h"
#include "brakevector.h"

#include <boost/noncopyable.hpp>
#include <string>
#include <memory>

class CS2Reader : private boost::noncopyable {
    public:
        static const int DEFAULT_PORT_READ = 15730;

        CS2Reader(
            ConcurrentCanQueuePtr dataToCS2, ConcurrentMsgQueuePtr dataToAppServer, BrakeVectorPtr brakeVector
        );
        virtual ~CS2Reader() noexcept;

        void connect(const std::string &host, int port = CS2Reader::DEFAULT_PORT_READ);
        auto read() const -> CS2CanCommand;

        void operator()();

    protected:
        void s88report(const CS2CanCommand &data);

        ConcurrentCanQueuePtr dataToCS2;
        ConcurrentMsgQueuePtr dataToAppServer;
        BrakeVectorPtr brakeVector;
        int fd_read;
};
