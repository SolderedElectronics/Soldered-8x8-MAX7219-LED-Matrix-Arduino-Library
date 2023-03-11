/**
 **************************************************
 *
 * @file        Led_Matrix_RobotEyes.ino
 *
 * @brief       Program to exercise the Led_Matrix library
 *
 *              Uses a sequence of bitmaps defined as a font to display animations of eyes to convey emotion.
 *
 *              Eyes are coordinated to work together.
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
#include "MD_RobotEyes.h"
#include <SPI.h>

// Define the number of devices we have in the chain and the hardware interface
#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define MAX_DEVICES 2

#define CS_PIN 10 // or LOAD

// SPI hardware interface
Led_Matrix M = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

MD_RobotEyes E;

typedef struct
{
    char name[7];
    MD_RobotEyes::emotion_t e;
    uint16_t timePause; // In milliseconds
} sampleItem_t;

// All of the samples for displaying
const sampleItem_t eSeq[] = {
    {"Nutral", MD_RobotEyes::E_NEUTRAL, 20000}, {"Blink", MD_RobotEyes::E_BLINK, 1000},
    {"Wink", MD_RobotEyes::E_WINK, 1000},       {"Right", MD_RobotEyes::E_LOOK_L, 1000},
    {"Left", MD_RobotEyes::E_LOOK_R, 1000},     {"Up", MD_RobotEyes::E_LOOK_U, 1000},
    {"Down", MD_RobotEyes::E_LOOK_D, 1000},     {"Angry", MD_RobotEyes::E_ANGRY, 1000},
    {"Sad", MD_RobotEyes::E_SAD, 1000},         {"Evil", MD_RobotEyes::E_EVIL, 1000},
    {"Evil2", MD_RobotEyes::E_EVIL2, 1000},     {"Squint", MD_RobotEyes::E_SQUINT, 1000},
    {"Dead", MD_RobotEyes::E_DEAD, 1000},       {"ScanV", MD_RobotEyes::E_SCAN_UD, 1000},
    {"ScanH", MD_RobotEyes::E_SCAN_LR, 1000},
};

void setup()
{
    // Init matrix
    M.begin();

    // Init animation class
    E.begin(&M);
    E.setText("RobotEyes");

    // Wait for the text to finish
    do
    {
        // Nothing . . .
    } while (!E.runAnimation());
}

void loop()
{
    // Go through each sample and display text and effect for it

    static uint32_t timeStartDelay;
    static uint8_t index = ARRAY_SIZE(eSeq);
    static enum
    {
        S_IDLE,
        S_TEXT,
        S_ANIM,
        S_PAUSE
    } state = S_IDLE;

    bool b = E.runAnimation(); // Always run the animation

    switch (state)
    {
    case S_IDLE:
        index++;
        if (index >= ARRAY_SIZE(eSeq))
            index = 0;
        E.setText(eSeq[index].name);
        state = S_TEXT;
        break;

    case S_TEXT: // Wait for the text to finish
        if (b)   // Text animation is finished
        {
            E.setAnimation(eSeq[index].e, true);
            state = S_ANIM;
        }
        break;

    case S_ANIM: // Checking animation is completed
        if (b)   // Animation is finished
        {
            timeStartDelay = millis();
            state = S_PAUSE;
        }
        break;

    case S_PAUSE: // Non blocking waiting for a period between animations
        if (millis() - timeStartDelay >= eSeq[index].timePause)
            state = S_IDLE;
        break;

    default:
        state = S_IDLE;
        break;
    }
}
