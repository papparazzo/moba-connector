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

#include "brakevector.h"

int BrakeVector::trigger(const Contact &contactId) {
    std::lock_guard guard{mutex};
    const auto iter = vector.find(contactId);

    if(iter == vector.end()) {
        return IGNORE_CONTACT;
    }

    const auto tmp = iter->second;

    // Ignore the following contacts afterward (it might be from the same train, E.g., light)
    iter->second = IGNORE_CONTACT;
    return tmp;
}

void BrakeVector::handleContact(const Contact &contactId, const int locId) {
    std::lock_guard guard{mutex};
    vector[contactId] = locId;
}

void BrakeVector::reset() {
    std::lock_guard guard{mutex};
    vector.clear();
}
