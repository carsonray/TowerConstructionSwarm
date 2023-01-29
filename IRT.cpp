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
  //Increments unique key code
  sendKey = (sendKey + 1) % 4;

  //Sets address
  sendAddress = address;

  //Compiles signal components in array
  unsigned int bitArr[3] = {sendKey, command, data};
  
  //Resets signal and bit count
  unsigned int currBits = 0;
  sendSignal = 0;
  
  //Builds signal from right to left
  for (int i = 2; i >=0; i--) {
    //Adds current component shifted by current base
    sendSignal |= (long) bitArr[i]<<currBits;

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

void TowerRobot::IRT::unpack(unsigned long signal) {
  //Signal component array
  unsigned int bitArr[3];

  //Gets temporary copy of signal
  unsigned long temp = signal;

  //Builds signal from right to left
  for (int i = 2; i >=0; i--) {
    //Gets signal at end
    bitArr[i] = temp & (((long) 1<<bitMap[i]) - 1);

    //Removes signal from end
    temp = temp>>bitMap[i];
  }

  //Updates recieve key
  prevRecvKey = recvKey;

  //Writes components
  recvKey = bitArr[0];
  recvCommand = bitArr[1];
  recvData = bitArr[2];
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
//Receives any unique signal data
bool TowerRobot::IRT::receiveOnce(unsigned int*command, unsigned int*data) {
  //Ensures signal is availiable
  if (!recvExists) {
    return false;
  } else if (recvClear) {
    recvExists = false;
  }
  
  //Writes components
  *command = recvCommand;
  *data = recvData;

  //Returns true if signal is unique
  return (recvKey != prevRecvKey);
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
    Serial.println(IrReceiver.decodedIRData.command, BIN);
    //Gets address
    recvAddress = IrReceiver.decodedIRData.address;

    //Unpacks main signal
    unpack(IrReceiver.decodedIRData.command);

    //Determines whether signal is addressed or has master address
    recvExists = (recvAddress == address) || (recvAddress == MASTER_ADDRESS);
    
    //Auto relays non-directed commands
    if (!recvExists && autoRelay) {
      IrSender.sendNEC(recvAddress, IrReceiver.decodedIRData.command, 1);
    }

    IrReceiver.resume();
  }

  //Sends data
  if (sendActive) {
    //Checks to see if their are still repeats left and interval is reached
    if (((currRepeats < sendRepeats) || (sendRepeats == -1)) && ((millis() - lastSend) >= currInterval)) {
      IrSender.sendNEC(sendAddress, sendSignal, 0);

      //Updates last send time
      lastSend = millis();

      //Calculates new interval
      currInterval = random(minInterval, maxInterval);

      //Increments repeats
      currRepeats++;
    }
  }
}