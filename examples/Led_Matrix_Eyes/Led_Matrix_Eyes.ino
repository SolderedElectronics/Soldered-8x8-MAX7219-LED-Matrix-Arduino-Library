/**
 **************************************************
 *
 * @file        Led_Matrix_Eyes.ino
 * 
 * @brief       Uses the graphics functions to animate a pair of eyes on 2 matrix modules.
 *              Eyes are coordinated to work together.
 *              Eyes are created to fill all available modules.
 *
 *              Wiring diagram:
 *              Dasduino   LED Matrix
 *                  |          |
 *                 VCC ------ VCC
 *                 GND ------ GND
 *                 D10 ------ LOAD
 *                 D11 ------ DIN
 *                 D13 ------ CLK
 * 
 *              If you connect more matrices, the first matrix is the one on the right side. 
 * 
 *                                 DP G  F  E  D  C  B  A  
 *                               +------------------------+
 *                               | 7  6  5  4  3  2  1  0 | D7
 *                       CLK <---|                      1 | D6 <--- CLK
 *                      LOAD <---|                      2 | D5 <--- LOAD
 *                      DOUT <---|                      3 | D4 <--- DIN
 *                       GND ----| O                    4 | D3 ---- GND
 *                       VCC ----| O  O                 5 | D2 ---- VCC
 *                               | O  O  O              6 | D1
 *                               | O  O  O  O           7 | D0
 *                               +------------------------+
 *              
 *
 * @authors     Goran Juric, Karlo Leksic for Soldered.com
 *
 *              Modified by Soldered for use on https://solde.red/333062, https://solde.red/333148,
 *              https://solde.red/333149, https://solde.red/333150, https://solde.red/333151 and
 *              https://solde.red/333152
 ***************************************************/

#include "Led-Matrix-SOLDERED.h"
#include "MD_EyePair.h"
#include <SPI.h>

// Define the number of devices we have in the chain and the hardware interface
#define MAX_DEVICES 2
#define HARDWARE_TYPE Led_Matrix::PAROLA_HW

// Define matrix pins
#define CLK_PIN       13 // or SCK
#define DATA_PIN      11 // or MOSI
#define CS_PIN        10 // or LOAD

// SPI hardware interface
Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Define the eyes!
#define MAX_EYE_PAIR (MAX_DEVICES / 2)

MD_EyePair E[MAX_EYE_PAIR];

// Miscellaneous defines
#define DELAYTIME 500 // In milliseconds

void setup()
{
    // Initialize matrix
    mx.begin();

    // Initialize the eye view
    for (uint8_t i = 0; i < MAX_EYE_PAIR; i++)
        E[i].begin(i * 2, &mx, DELAYTIME);
}

void loop()
{
    // Run the eyes animation
    for (uint8_t i = 0; i < MAX_EYE_PAIR; i++)
        E[i].animate();
}
