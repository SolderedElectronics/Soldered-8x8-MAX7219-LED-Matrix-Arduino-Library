/**
 **************************************************
 *
 * @file        Led_Matrix_Shift.ino
 *
 * @brief       Program to exercise the Led_Matrix library
 *
 *              Test the library transformation functions.
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
#include <SPI.h>

// Use a button to transfer between transformations or just do it on a timer basis
#define USE_SWITCH_INPUT 0

// Switch pin if enabled - active LOW
#define SWITCH_PIN 8

// We always wait a bit between updates of the display
#define DELAYTIME 500 // in milliseconds

// Number of times to repeat the transformation animations
#define REPEATS_PRESET 16

// Define the number of devices we have in the chain and the hardware interface
#define MAX_DEVICES     3
#define WRAPAROUND_MODE Led_Matrix::ON
#define HARDWARE_TYPE   Led_Matrix::PAROLA_HW

// Define matrix pins
#define CLK_PIN  13 // or SCK
#define DATA_PIN 11 // or MOSI
#define CS_PIN   10 // or LOAD

// SPI hardware interface
Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Returns 1 if it needs to change the transformation (shifting) type
bool changeState(void)
{
    bool b = false;

#if USE_SWITCH_INPUT
    // Read the state from the switch
    static int8_t lastStatus = HIGH;
    int8_t status = digitalRead(SWITCH_PIN);

    b = (lastStatus == HIGH) && (status == LOW);
    lastStatus = status;
#else
    // Change the state periodically
    static uint32_t lastTime = 0;
    static uint8_t repeatCount = 0;

    if (repeatCount == 0)
        repeatCount = REPEATS_PRESET;

    if (millis() - lastTime >= DELAYTIME)
    {
        lastTime = millis();
        b = (--repeatCount == 0);
    }
#endif

    return (b);
}

// Run certain transformation
void transformDemo(Led_Matrix::transformType_t tt, bool bNew)
{
    static uint32_t lastTime = 0;

    if (bNew)
    {
        // If it's time for a new transformation type, clear the matrix and display the char
        mx.clear();

        for (uint8_t i = 0; i < MAX_DEVICES; i++)
            mx.setChar(((i + 1) * COL_SIZE) - 1, 'o' + i);
        lastTime = millis();
    }

    // Actually do the transformation
    if (millis() - lastTime >= DELAYTIME)
    {
        mx.transform(0, MAX_DEVICES - 1, tt);
        lastTime = millis();
    }
}

void setup()
{
    // Init matrix
    mx.begin();

    // Use wraparound mode
    mx.control(Led_Matrix::WRAPAROUND, WRAPAROUND_MODE);

    // If used switch, set it as input
#if USE_SWITCH_INPUT
    pinMode(SWITCH_PIN, INPUT_PULLUP);
#endif

    // Init serial communication
    Serial.begin(115200);
    Serial.println("[Transform Test]");
}

void loop()
{
    static int8_t tState = -1;
    static bool bNew = true;

    // If it's time to change the transformation type
    if (bNew)
    {
        tState = (tState + 1) % 8;
        Serial.print("State: ");
        Serial.println(tState);
    }

    // Run a transformation depending on the current state
    switch (tState)
    {
    case 0:
        transformDemo(Led_Matrix::TSL, bNew);
        break;
    case 1:
        transformDemo(Led_Matrix::TSR, bNew);
        break;
    case 2:
        transformDemo(Led_Matrix::TSU, bNew);
        break;
    case 3:
        transformDemo(Led_Matrix::TSD, bNew);
        break;
    case 4:
        transformDemo(Led_Matrix::TFUD, bNew);
        break;
    case 5:
        transformDemo(Led_Matrix::TFLR, bNew);
        break;
    case 6:
        transformDemo(Led_Matrix::TRC, bNew);
        break;
    case 7:
        transformDemo(Led_Matrix::TINV, bNew);
        break;
    default:
        tState = 0; // Just in case
    }

    // Check if it's time to change the transformation type
    bNew = changeState();
}
