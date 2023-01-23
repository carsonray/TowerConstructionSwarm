/*
  ColorSensor.cpp - Detects block colors with Adafruit TCS34725 Color Sensor
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include <Wire.h>
#include "Adafruit_TCS34725.h"

TowerRobot::ColorSensor::ColorSensor() {
    tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
}

int TowerRobot::ColorSensor::numColors() {
    return sizeof(blockColors)/sizeof(blockColors[0]);
}

bool TowerRobot::ColorSensor::begin() {
    return tcs.begin();
}

void TowerRobot::ColorSensor::raw(int* r, int*g, int*b, int* c) {
    tcs.setInterrupt(false);  // turn on LED

    delay(60);  // takes 50ms to read
    
    tcs.getRawData(r, g, b, c);

    tcs.setInterrupt(true);
}

int TowerRobot::ColorSensor::getBlockColor() {
    //Gets raw reading with channels in array
    int c;
    int channels[3];
    raw(channels, channels+1, channels+2, &c);

    //Checks empty threshold
    if (c > emptyThres) {
        //Scales rgb values
        for (int ch = 0; ch < 3; ch++) {
            channels[ch] *= 255/c;
        }

        //Minimum color difference
        float minDiff = 255;

        //Color with least difference
        float minColor = 0;

        //Checks each color
        for (int col = 0; col < numColors();; col++) {
            //Gets sum of block channels
            int blockSum = 0;
            for (int ch = 0; ch < 3; ch++) {
                blockSum += blockColors[col][ch];
            }

            //Sums each scaled channel difference
            int currDiff = 0;
            for (int ch = 0; ch < 3; ch++) {
                currDiff += abs(blockColors[col][ch]/blockSum*255 - channels[ch]);
            }

            //Updates closest color
            if (currDiff < minDiff) {
                minDiff = currDiff;
                minColor = col;
            }
        }

        //Returns closest color
        return minColor;
    } else {
        return -1;
    }
}
