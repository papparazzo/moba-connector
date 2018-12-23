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

#include <map>

class ReportVector {
    public:
        enum HandleSwitchingEdge {
            HANDLE_FALLING_ONLY,
            HANDLE_RISING_ONLY,
            HANDLE_BOTH,
            IGNORE_BOTH
        };

        enum SwitchingEdge {
            FALLING,
            RISING,
        };

        struct Action {
            HandleSwitchingEdge edge;
            int action;
        };

        ReportVector();
        ReportVector(const ReportVector& orig);
        virtual ~ReportVector();

        void trigger(int contactId, SwitchingEdge switchingEdge);

        void checkContact(int contactId, HandleSwitchingEdge switchingEdge, int action);
        void ignoreContact(int contactId);

// ignore
// stop
// error

    protected:
        std::map<int, Action> vector;

    private:
};

