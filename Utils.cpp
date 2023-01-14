/*
  Utils.cpp - Utilities
  Created by Carson G. Ray, January 4 2022.
*/

#include "Arduino.h"
#include "Utils.h"

//Takes logarithm with base
static float Utils::loga(float base, float arg) {
    return log(arg)/log(base);
}

//Sets sign of value
static int Utils::sign(double val) {
    if (val == 0) {
        return 0;
    } else {
        return val/abs(val);
    }
}