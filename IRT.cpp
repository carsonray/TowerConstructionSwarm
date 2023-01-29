/*
  IRT.cpp - Communicates with other robots and central controller
  Created by Carson G. Ray, January 4 2022.
*/

#include <Arduino.h>
#include "TowerRobot.h"

#include <IRremote.hpp>

using namespace IRcommands;

TowerRobot::IRT::IRT(int address, int sendPin, int recvPin) {
    //Initializes ir send and recieve pins
    this->sendPin = sendPin;
    this->recvPin = recvPin;

    //Sets communication id
    this->address = address;
}

//Initializes transceiver
void TowerRobot::IRT::begin() {
  IrSender.begin(sendPin);
  IrReceiver.begin(recvPin);
}

//Sends command without data
void TowerRobot::IRT::send(unsigned int address, unsigned int command) {
  send(address, command, 0);
}
//Sends command with data
void TowerRobot::IRT::send(unsigned int address, unsigned int command, unsigned int data) {
  //Sets command data
  sendCommand = data;

  //Compiles address components in array
  unsigned int bitArr[2] = {address, command};
  
  //Resets signal and bit count
  unsigned int currBits = 0;
  sendAddress = 0;
  
  //Builds signal from right to left
  for (int i = 1; i >=0; i--) {
    //Adds current component shifted by current base
    sendAddress |= bitArr[i]<<currBits;

    //Increments base by bits taken up by component
    currBits += bitMap[i];
  }
  
  //Sets defaults for sending
  currInterval = 0;
  minInterval = 0;
  maxInterval = 0;

  currRepeats = 0;
  sendRepeats = 1;

  sendActive = true;
}

void TowerRobot::IRT::setSendInterval(int interval) {
  setSendInterval(interval, interval);
}
void TowerRobot::IRT::setSendInterval(int minTime, int maxTime) {
  minInterval = minTime;
  maxInterval = maxTime;
}

void TowerRobot::IRT::setSendRepeats(int repeats) {
  sendRepeats = repeats;
}
void TowerRobot::IRT::setSendActive(bool active) {
  sendActive = active;
}
void TowerRobot::IRT::resetSendRepeat() {
  currRepeats = 0;
}

void TowerRobot::IRT::unpack(unsigned int address, unsigned int command) {
  //Address component array
  unsigned int bitArr[2];

  //Gets data
  recvData = command;

  //Gets temporary copy of address
  unsigned long temp = address;

  //Unpacks components from right to left
  for (int i = 1; i >=0; i--) {
    //Gets component at end
    bitArr[i] = temp & (((unsigned int) 1<<bitMap[i]) - 1);

    //Removes component from end
    temp = temp>>bitMap[i];
  }

  //Sets components
  recvAddress = bitArr[0];
  recvCommand = bitArr[1];
}

//Receives any signal data
bool TowerRobot::IRT::receive(unsigned int*command, unsigned int*data) {
  //Ensures signal is availiable
  if (!recvExists) {
    return false;
  } else if (recvClear) {
    //Disables access after read
    recvExists = false;
  }

  //Reads components
  *command = recvCommand;
  *data = recvData;

  return true;
}

void TowerRobot::IRT::setRecieveActive(bool active) {
  if (recvActive != active) {
    if (active) {
      IrReceiver.start();
    } else {
      IrReceiver.stop();
    }
  }
  recvActive = active;
}

void TowerRobot::IRT::setAutoClear(bool clear) {
  recvClear = clear;
}

//Sets whether non-directed signals are relayed
void TowerRobot::IRT::setAutoRelay(bool active) {
  autoRelay = active;
}

//Updates sending and receiving actions
void TowerRobot::IRT::update() {
  //Updates recieved signal
  if (recvActive && IrReceiver.decode()) {
    //Unpacks signal
    unpack(IrReceiver.decodedIRData.address, IrReceiver.decodedIRData.command);

    //Determines whether signal is addressed or has master address
    recvExists = (recvAddress == address) || (recvAddress == MASTER_ADDRESS);
    
    //Auto relays non-directed commands
    if (!recvExists && autoRelay) {
      IrSender.sendNEC(IrReceiver.decodedIRData.address, IrReceiver.decodedIRData.command, 0);
    }

    IrReceiver.resume();
  }

  //Sends data
  if (sendActive) {
    //Checks to see if their are still repeats left and interval is reached
    if (((currRepeats < sendRepeats) || (sendRepeats == -1)) && ((millis() - lastSend) >= currInterval)) {
      IrSender.sendNEC(sendAddress, sendCommand, 0);

      //Updates last send time
      lastSend = millis();

      //Calculates new interval
      currInterval = random(minInterval, maxInterval);

      //Increments repeats
      currRepeats++;
    }
  }
}