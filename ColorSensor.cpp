/*
  ColorSensor.cpp - Detects block colors with Adafruit TCS34725 Color Sensor
  Created by Carson G. Ray, January 4 2022.
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "TowerRobot.h"

TowerRobot::ColorSensor::ColorSensor() {
    tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
}

int TowerRobot::ColorSensor::numColors() {
    return sizeof(blockColors)/sizeof(blockColors[0]);
}

bool TowerRobot::ColorSensor::begin() {
    return tcs.begin();
}

void TowerRobot::ColorSensor::getRaw(bool led, int* r, int*g, int*b, int* c) {
    //Whether LED is on or off
    if (led) {
        tcs.setInterrupt(false);  // turn on LED

        delay(100);  // takes 50ms to read
    
        tcs.getRawData(r, g, b, c);

        tcs.setInterrupt(true); // turn off LED
    } else {
        tcs.setInterrupt(true); // turn off LED

        delay(100); // takes 50ms to read

        tcs.getRawData(r, g, b, c);
    }
}

void TowerRobot::ColorSensor::getReflected(int* r, int* g, int* b, int* c) {
    //Gets ambient light values with LED off
    int ambient[4];
    getRaw(false, ambient, ambient+1, ambient+2, ambient+3);

    //Gets reflected light values with LED on
    int reflected[4];
    getRaw(true, reflected, reflected+1, reflected+2, reflected+3);

    //Subtracts out ambient light
    for (int i = 0; i < 4; i++) {
        reflected[i] -= ambient[i];
    }

    *r = *(reflected);
    *g = *(reflected+1);
    *b = *(reflected+2);
    *c = *(reflected+3);
}

int TowerRobot::ColorSensor::getBlockColor() {
    //Gets raw reading with channels in array
    int c;
    int channels[3];
    getReflected(channels, channels+1, channels+2, &c);

    //Checks empty threshold
    if (c > emptyThres) {
        //Scales rgb values

        //Minimum color difference
        float minDiff = 0;

        //Color with least difference
        float minColor = 0;

        //Checks each color
        for (int col = 0; col < numColors(); col++) {
            //Sums each squared channel difference
            int currDiff = 0;
            for (int ch = 0; ch < 3; ch++) {
                currDiff += abs(blockColors[col][ch] - channels[ch]);
            }

            //Updates closest color
            if ((currDiff < minDiff) || (col == 0)) {
                minDiff = currDiff;
                minColor = col;
            }
        }

        //Returns closest color
        return minColor;
    } else {
        return EMPTY;
    }
}
