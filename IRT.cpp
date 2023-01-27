/*
  IRT.cpp - Communicates with other robots and central controller
  Created by Carson G. Ray, January 4 2022.
*/

#include <Arduino.h>
#include "TowerRobot.h"

#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>

//Disables send pin macro so it can be set in constructor
#undef IR_SEND_PIN

//Commands

#define IR_POLL_STATUS 0x00
#define IR_STATUS_WAITING 0x01
#define IR_STATUS_READY 0x02

#define IR_STOP 0xFF

#define IR_TOWER_UPDATE 0x10

#define IR_CLOSE_GRIP 0x20
#define IR_OPEN_GRIP 0x21
#define IR_TURRET_LEFT 0x22
#define IR_TURRET_RIGHT 0x23
#define IR_SLIDE_DOWN 0x24
#define IR_SLIDE_UP 0x25

TowerRobot::IRT::IRT(int id, int sendPin, int recvPin) {
    //Initializes ir send and recieve
    send.setSendPin(sendPin);
    recv = IRrecv(recvPin);

    //Sets communication id
    this->id = id;
}

void TowerRobot::IRT::begin() {
  irrecv.enableIRIn();
}

long pack(int target, int command) {

}
long pack(int target, int command, int data) {

}

void unpack(lon signal, int bits, int* target, int * command, int* data) {
  
}

void TowerRobot::IRT::send(int target, int command) {

}
void TowerRobot::IRT::send(int target, int command, long data) {

}

void TowerRobot::IRT::setSendRepeat(int interval) {

}
void TowerRobot::IRT::setSendRepeat(int minTime, int maxTime) {

}

void TowerRobot::IRT::setSendLimit(int num) {

}
void TowerRobot::IRT::setSendInterrupt(bool interrupted) {

}

bool TowerRobot::IRT::receive(int*command, int*data) {

}
bool TowerRobot::IRT::receiveOnce(int*command, int*data) {

}

void TowerRobot::IRT::resumeReceive() {

}

void TowerRobot::IRT::setAutoRelay(bool active) [

]

void TowerRobot::IRT::update() [

]