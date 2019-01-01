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

#include "reportvector.h"

int ReportVector::trigger(Contact contactId) {
    auto iter = vector.find(contactId);

    if(iter == vector.end()) {
        return ReportVector::CONTACT_UNSET;
    }
    auto tmp = iter->second;
    iter->second = ReportVector::IGNORE_CONTACT;
    return tmp;
}

void ReportVector::handleContact(Contact contactId, int locId) {
    vector[contactId] = locId;
}

void ReportVector::ignoreContact(Contact contactId) {
    handleContact(contactId, ReportVector::IGNORE_CONTACT);
}

void ReportVector::setContactUnreachable(Contact contactId) {
    handleContact(contactId, ReportVector::CONTACT_UNREACHABLE);
}