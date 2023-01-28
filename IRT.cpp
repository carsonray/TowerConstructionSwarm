/*
  IRT.cpp - Communicates with other robots and central controller
  Created by Carson G. Ray, January 4 2022.
*/

#include <Arduino.h>
#include "TowerRobot.h"

#include <IRremote.hpp>

//Disables send pin macro so it can be set on begin
//#undef IR_SEND_PIN

TowerRobot::IRT::IRT(int id, int sendPin, int recvPin) {
    //Initializes ir send and recieve pins
    this->sendPin = sendPin;
    this->recvPin = recvPin;

    //Sets communication id
    this->id = id;
}

//Initializes transceiver
void TowerRobot::IRT::begin() {
  IrSender.begin(DISABLE_LED_FEEDBACK);
  //IrSender.setSendPin(sendPin);
  IrReceiver.begin(recvPin, DISABLE_LED_FEEDBACK);
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
  int bitArr[3] = {sendKey, command, data};
  
  //Resets signal and bit count
  int currBits = 0;
  sendSignal = 0;

  //Builds signal from right to left
  for (int i = 3; i >=0; i++) {
    //Adds current component at current base
    sendSignal += pow(2, currBits) * bitArr[i];

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
void TowerRobot::IRT::setSendInterrupt(bool interrupted) {
  sendActive = !interrupted;
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
  for (int i = 3; i >=0; i++) {
    //Gets signal at end with modulo of bitMap base
    bitArr[i] = temp % (int) pow(2, bitMap[i]);

    //Removes signal from end
    temp /= pow(2, bitMap[i]);
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

  //Writes components
  *command = recvCommand;
  *data = recvData;

  //Returns true if address matches
  return (recvAddress == id);
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

  //Returns true if address matches and signal is unique
  return (recvAddress == id) && (recvKey != prevRecvKey);
}

void TowerRobot::IRT::setRecieveInterrupt(bool interrupted) {
  if (!recvActive && interrupted) {
    //Clears data if changed to active
    IrReceiver.decode();
    IrReceiver.resume();
  }
  recvActive = !interrupted;
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
    recvExists = true;
    
    recvAddress = IrReceiver.decodedIRData.address;
    unpack(IrReceiver.decodedIRData.command);

    //Auto relays non-directed commands
    if ((recvAddress != id) && autoRelay) {
      IrSender.sendNEC(recvAddress, IrReceiver.decodedIRData.command, 1);
    }
  }

  //Sends data
  if (sendActive) {
    if ((currRepeats <= sendRepeats) && ((millis() - lastSend) >= currInterval)) {
      IrSender.sendNEC(sendAddress, sendSignal, 1);

      //Updates last send time
      lastSend = millis();

      //Calculates new interval
      currInterval = random(minInterval, maxInterval);
    }
  }

  //Resumes recieve cycle
  if (recvActive) {
    IrReceiver.resume();
  }
}