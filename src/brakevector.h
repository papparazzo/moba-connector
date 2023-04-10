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

#include <boost/noncopyable.hpp>

#include <map>
#include <exception>
#include <mutex>
#include <memory>

class BrakeVector {
public:
    BrakeVector() = default;
    
    BrakeVector(const BrakeVector&) = delete;
    BrakeVector& operator=(const BrakeVector&) = delete;
    
    ~BrakeVector() = default;
    
    static const int IGNORE_CONTACT = 0;

    using Contact = std::pair<int, int>;
    using Vector = std::map<Contact, int>;

    int trigger(Contact contactId);

    void handleContact(Contact contactId, int locId);
    void reset();

protected:
    Vector vector;
    std::mutex mutex;
};

using BrakeVectorPtr = std::shared_ptr<BrakeVector>;