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

#include "parseConfig.h"

ParseConfig::ParseConfig() {
}

ParseConfig::ParseConfig(const ParseConfig& orig) {
}

ParseConfig::~ParseConfig() {
}

std::uint16_t updateCRC(std::uint16_t CRC_acc, std::uint8_t in) {

    // Create the CRC "dividend" for polynomial arithmetic (binary arithmetic with no carries)
    CRC_acc = CRC_acc ^ (in << 8);
    // "Divide" the poly into the dividend using CRC XOR subtraction CRC_acc holds the
    // "remainder" of each divide. Only complete this division for 8 bits since input is 1 byte
    for (int i = 0; i < 8; i++) {
        // Check if the MSB is set (if MSB is 1, then the POLY can "divide" into the "dividend")
        if ((CRC_acc & 0x8000) == 0x8000) {
            // if so, shift the CRC value, and XOR "subtract" the poly
            CRC_acc = CRC_acc << 1;
            CRC_acc ^= 0x1021;
        } else {
            // if not, just shift the CRC value
            CRC_acc = CRC_acc << 1;
        }
    }
    // Return the final remainder (CRC value)
    return CRC_acc;
}





