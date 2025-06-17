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
 *  along with this program. If not, see <https://www.gnu.org/licenses/agpl.txt>.
 *
 */

#pragma once
#include <mutex>

#include "actionabstract.h"
#include "moba/shared.h"

class ActionListHandler final {
public:
    using ActionList = std::vector<ActionAbstractPtr>;

    ActionListHandler() = default;

    ActionListHandler(const ActionListHandler&) = delete;
    ActionListHandler& operator=(const ActionListHandler&) = delete;

    ~ActionListHandler() = default;

    using Vector = std::map<ContactData, int>;

    const Vector& getVector() {return vector;};


    void insertActionList();
    void removeActionList(int id);

    int trigger(const ContactData &contactId);
    void handleContact(const Contact &contactId, int locId);


private:
    Vector vector;
    ActionList actionList;

    std::mutex mutex;
};
