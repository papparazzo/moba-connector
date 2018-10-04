/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   bridge.cpp
 * Author: stefan
 *
 * Created on October 3, 2018, 12:04 AM
 */

#include "bridge.h"
#include <cstring>

void Bridge::setEmergencyStop() {
    send(CMD_SYSTEM, 5, 0x00000000, SYS_SUB_CMD_SYSTEM_STOP);
}

void Bridge::setEmergencyStopClearing() {
    send(CMD_SYSTEM, 5, 0x00000000, SYS_SUB_CMD_SYSTEM_GO);
}

void Bridge::send(uint8_t cmd, uint8_t length, uint32_t uid, uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3) {
    CS2Connector::RawData raw;
    memset((void*)&raw, '\0', sizeof(raw));

    raw.header[0] = 0x00;
    raw.header[1] = cmd;

    raw.hash[0] = 0x03;
    raw.hash[1] = 0x00;

    raw.length = length;

/*
    raw.uid[0]
    raw.uid[1]
    raw.uid[2]
    raw.uid[3]
*/
    raw.data[0] = data0;
    raw.data[1] = data1;
    raw.data[2] = data2;
    raw.data[3] = data3;

    connector->sendData(raw);
}