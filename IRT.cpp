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
  randomSeed(analogRead(A0));
  IrSender.begin(sendPin, DISABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
  IrReceiver.begin(recvPin, DISABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
}

//Gets address
int TowerRobot::IRT::getAddress() {
  return address;
}

//Turns sending on and off
void TowerRobot::IRT::setSendActive(bool active) {
  sendActive = active;
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

//Sets interval between repeats
void TowerRobot::IRT::setSendInterval(int interval) {
  setSendInterval(interval, interval);
}
//Sets random interval between bounds
void TowerRobot::IRT::setSendInterval(int minTime, int maxTime) {
  minInterval = minTime;
  maxInterval = maxTime;
}

//Sets number of repeats
void TowerRobot::IRT::setSendRepeats(int repeats) {
  sendRepeats = repeats;
}

//Resets repeating
void TowerRobot::IRT::resetSendRepeats() {
  currRepeats = 0;
}

//Sets intervals
void TowerRobot::IRT::useInterval() {
  //Updates last send time
  lastSend = millis();

  //Calculates new interval
  currInterval = random(minInterval, maxInterval);
}

//Whether transceiver is currently sending
bool TowerRobot::IRT::isSending() {
  return (currRepeats < sendRepeats);
}

//Waits until done sending
void TowerRobot::IRT::waitSend() {
  while (isSending()) {
    update();
  }
}

//Turns receiving on or off
void TowerRobot::IRT::setReceiveActive(bool active) {
  recvActive = active;
}

//Unpacks received signal
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
  if (recvExists) {
    //Reads components
    *command = recvCommand;
    *data = recvData;

    //Turns off receive
    recvExists = false;
    return true;
  } else {
    return false;
  }
}
bool TowerRobot::IRT::receive(unsigned int*address, unsigned int*command, unsigned int*data) {
  //Ensures signal is availiable
  if (recvExists) {
    //Reads components
    *address = recvAddress;
    *command = recvCommand;
    *data = recvData;

    //Turns off receive
    recvExists = false;
    return true;
  } else {
    return false;
  }
}

//Waits until something is received
void TowerRobot::IRT::waitReceive() {
  while (!recvExists) {
    update();
  }
}

//Waits until received or timeout
bool TowerRobot::IRT::waitReceive(int timeout) {
  unsigned long timeoutStart = millis();
  while ((!recvExists) && ((millis() - timeoutStart) < timeout)) {
    update();
  }

  return recvExists;
}

//Sets whether non-directed signals are relayed
void TowerRobot::IRT::setAutoRelay(bool active) {
  autoRelay = active;
}

//Updates sending and receiving actions
void TowerRobot::IRT::update() {
  //Updates recieved signal
  if (recvActive && IrReceiver.decode()) {
    //Ensures protocol is correct and is not interfering with sending
    if ((IrReceiver.decodedIRData.protocol == NEC) && (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW)) && ((millis() - lastSend) >= sheildTime)) {
      //Unpacks signal
      unpack(IrReceiver.decodedIRData.address, IrReceiver.decodedIRData.command);

      IrReceiver.resume();
      
      //Determines whether signal is addressed or has master address
      recvExists = (recvAddress == address) || (recvAddress == MASTER_ADDRESS);

      //Auto relays non-directed commands
      if (!recvExists && autoRelay) {
        send(recvAddress, recvCommand, recvData);
        waitSend();
      }
    } else {
      IrReceiver.resume();
    }
  }

  //Sends data
  if (sendActive) {
    //Checks to see if their are still repeats left and interval is reached
    if (((currRepeats < sendRepeats) || (sendRepeats == -1)) && ((millis() - lastSend) >= currInterval)) {
      IrSender.sendNEC(sendAddress, sendCommand, 0);

      useInterval();

      //Increments repeats
      currRepeats++;
    }
  }
}

//Synchronizes robots
void TowerRobot::IRT::synchronize(int num, int interval) {
  //Sends ready signal
  send(MASTER_ADDRESS, DONE, address);
  setSendInterval(100);
  setSendRepeats(10);
  waitSend();
}