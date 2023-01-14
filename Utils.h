/*
  Utils.h - Utilities
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef Utils_h
#define Utils_h

#include "Arduino.h"

class Utils {
    public:
        static float loga(float base, float arg);
        static int sign(double val);
};

#endif