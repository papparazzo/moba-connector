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

class BrakeVectorException : public std::exception {
    public:
        virtual ~BrakeVectorException() noexcept {

        }

        BrakeVectorException(const std::string &what) {
            this->what__ = what;
        }

        virtual const char* what() const noexcept {
            return this->what__.c_str();
        }

    private:
        std::string what__;
};

class BrakeVector : private boost::noncopyable {
    public:
        static const int CONTACT_UNSET       =  0;
        static const int IGNORE_CONTACT      = -1;
        static const int CONTACT_UNREACHABLE = -2;

        using Contact = std::pair<int, int> ;
        using Vector = std::map<Contact, int>;

        int trigger(Contact contactId);

        void handleContact(Contact contactId, int locId);
        void ignoreContact(Contact contactId);
        void setContactUnreachable(Contact contactId);

    protected:
        Vector vector;
        std::mutex mutex;
};

using BrakeVectorPtr = std::shared_ptr<BrakeVector>;