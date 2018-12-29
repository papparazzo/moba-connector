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

void Bridge::ping() {
    send(CMD_PING);
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

Bridge::ResponseCode Bridge::recieveCanData() {
    CS2Connector::RawData data;
    if(!connector->recieveData(data)) {
        return RES_EMPTY;
    }

    // Ping Response-Nachricht
    if(data.header[1] & 0x01 && data.header[1] == (CMD_PING | 0x01)) {
        return RES_PING;
    }

    if(data.header[1] == CMD_SYSTEM) {
        switch(data.data[0]) {
            case SYS_SUB_CMD_SYSTEM_GO:
                return RES_SYSTEM_GO;

            case SYS_SUB_CMD_SYSTEM_HALT:
                return RES_SYSTEM_HALT;

            case SYS_SUB_CMD_SYSTEM_STOP:
                return RES_SYSTEM_STOP;
        }
    }

    if(data.header[1] == (CMD_S88_EVENT | 0x01) /*&& s88callback*/) {
        std::uint16_t time = (data.data[2] << 8) | data.data[3];

        std::uint16_t addr = (data.uid[0] << 8) | data.uid[1];
        std::uint16_t contact = (data.uid[2] << 8) | data.uid[3];

        s88callback(addr, contact, static_cast<bool>(data.data[1]), time);
    }
}