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

#include "actionlisthandler.h"

#include <mutex>

void ActionListHandler::replaceActionList(ContactData &&contact, ActionListCollection &&actionListCollection) {
    removeActionListByContact(std::move(contact));
    insertActionList(std::move(contact), std::move(actionListCollection));
}

void ActionListHandler::insertActionList(ContactData &&contact, ActionListCollection &&actionListCollection) {
    actionListCollections.emplace(std::move(contact), std::move(actionListCollection));
}

void ActionListHandler::removeActionListByContact(ContactData &&contact) {
    if (!actionListCollections.erase(contact)) {
        throw std::runtime_error("could not erase contact <" + static_cast<std::string>(contact) + "> from map.");
    }
}

void ActionListHandler::trigger(const ContactData &contact) {
    const auto iter = actionListCollections.find(contact);
    if (iter == actionListCollections.end()) {
        throw std::runtime_error("could not find contact <" + static_cast<std::string>(contact) + "> in map.");
    }

    iter->second.execute();
}
