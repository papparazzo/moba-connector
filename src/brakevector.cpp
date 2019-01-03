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

#include "brakevector.h"
#include <sstream>

int BrakeVector::trigger(Contact contactId) {
    std::lock_guard<std::mutex> guard(mutex);
    auto iter = vector.find(contactId);

    if(iter == vector.end()) {
        return BrakeVector::CONTACT_UNSET;
    }

    auto tmp = iter->second;

    if(tmp != BrakeVector::CONTACT_UNREACHABLE) {
        iter->second = BrakeVector::IGNORE_CONTACT;
        return tmp;
    }

    // An unreachable Contact was triggered. E.g. train reached contact by a wrong turnout
    std::stringstream ss;
    ss << "contact <" << contactId.first << ", " << contactId.second << "> not set!";
    throw BrakeVectorException(ss.str());
}

void BrakeVector::handleContact(Contact contactId, int locId) {
    std::lock_guard<std::mutex> guard(mutex);
    vector[contactId] = locId;
}

void BrakeVector::ignoreContact(Contact contactId) {
    handleContact(contactId, BrakeVector::IGNORE_CONTACT);
}

void BrakeVector::setContactUnreachable(Contact contactId) {
    handleContact(contactId, BrakeVector::CONTACT_UNREACHABLE);
}