/*
  IRT.cpp - Communicates with other robots and central controller
  Created by Carson G. Ray, January 4 2022.
*/

#include <Arduino.h>
#include "TowerRobot.h"

#define USE_IRREMOTE_HPP_AS_PLAIN_INCLUDE
#include <IRremote.hpp>

TowerRobot::IRT::IRT(int id, int sendPin, int recvPin) {
    //Initializes ir send and recieve
    send.setSendPin(sendPin);
    recv = IRrecv(recvPin);

    //Sets communication id
    this->id = id;
}