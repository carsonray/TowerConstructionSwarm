/*
  Utils.h - Utilities
  Created by Carson G. Ray, January 4 2022.
*/

#ifndef Utils_h
#define Utils_h

#include "Arduino.h"

class Utils {
    public:
        static int sign(double val);
        static int modulo(int dividend, int divisor);
        static double modulo(double dividend, double divisor);
};

#endif