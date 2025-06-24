/*
*  Project:    moba-connector
 *
 *  Copyright (C) 2025 Stefan Paproth <pappi-@gmx.de>
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

#include "actionlistcollection.h"

#include <thread>

void ActionListCollection::push(ActionList &&actionList) {
    queue.push(std::move(actionList));
}

void ActionListCollection::execute() {
    if(queue.empty()) {
        // Should never happen!
        throw std::runtime_error("list-collection is empty");
    }
    std::thread jsonwriterThread{std::move(queue.front())};
    jsonwriterThread.detach();
    queue.pop();
}


