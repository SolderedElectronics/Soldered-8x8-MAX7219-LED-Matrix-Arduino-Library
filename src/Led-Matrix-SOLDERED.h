/**
 **************************************************
 *
 * @file        Generic-easyC-SOLDERED.h
 * @brief       Header file for sensor specific code.
 *
 *
 * @copyright GNU General Public License v3.0
 * @authors     Zvonimir Haramustek for soldered.com.com
 ***************************************************/

#ifndef __MAX7219_SOLDERED__
#define __MAX7219_SOLDERED__

#include "Arduino.h"
#include "libs/MD_MAX72XX/src/MD_MAX72xx.h"

class Led_Matrix : public MD_MAX72XX
{
  public:
    Led_Matrix(int csPin, int maxDevices) : MD_MAX72XX(GENERIC_HW, csPin, maxDevices)
    {
    }

    Led_Matrix(moduleType_t mod, int csPin, int maxDevices) : MD_MAX72XX(mod, csPin, maxDevices)
    {
    }

    Led_Matrix(int dataPin, int clkPin, int csPin, int maxDevices)
        : MD_MAX72XX(GENERIC_HW, dataPin, clkPin, csPin, maxDevices)
    {
    }
};

#endif
