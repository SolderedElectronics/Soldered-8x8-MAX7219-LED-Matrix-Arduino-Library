/**
 **************************************************
 *
 * @file        Led_Matrix_RobotEyes.ino
 * @brief       Program to exercise the Led_Matrix library
 * 
 *              Uses a sequence of bitmaps defined as a font to display animations of eyes to convey emotion.
 * 
 *              Eyes are coordinated to work together.
 *
 * @authors     Goran Juric for Soldered.com
 * 
 *              Modified by Soldered for use on https://solde.red/333062, https://solde.red/333148,
 *              https://solde.red/333149, https://solde.red/333150, https://solde.red/333151 and
 *              https://solde.red/333152
 ***************************************************/

#include "Led-Matrix-SOLDERED.h"
#include "MD_RobotEyes.h"
#include <SPI.h>

#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define MAX_DEVICES 2

#define CS_PIN 10 // or SS

// SPI hardware interface
Led_Matrix M = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

MD_RobotEyes E;

typedef struct
{
    char name[7];
    MD_RobotEyes::emotion_t e;
    uint16_t timePause; // in milliseconds
} sampleItem_t;

const sampleItem_t eSeq[] = {
    {"Nutral", MD_RobotEyes::E_NEUTRAL, 20000}, {"Blink", MD_RobotEyes::E_BLINK, 1000},
    {"Wink", MD_RobotEyes::E_WINK, 1000},       {"Left", MD_RobotEyes::E_LOOK_L, 1000},
    {"Right", MD_RobotEyes::E_LOOK_R, 1000},    {"Up", MD_RobotEyes::E_LOOK_U, 1000},
    {"Down", MD_RobotEyes::E_LOOK_D, 1000},     {"Angry", MD_RobotEyes::E_ANGRY, 1000},
    {"Sad", MD_RobotEyes::E_SAD, 1000},         {"Evil", MD_RobotEyes::E_EVIL, 1000},
    {"Evil2", MD_RobotEyes::E_EVIL2, 1000},     {"Squint", MD_RobotEyes::E_SQUINT, 1000},
    {"Dead", MD_RobotEyes::E_DEAD, 1000},       {"ScanV", MD_RobotEyes::E_SCAN_UD, 1000},
    {"ScanH", MD_RobotEyes::E_SCAN_LR, 1000},
};

void setup()
{
    M.begin();
    E.begin(&M);
    E.setText("RobotEyes");
    do
    {
    } while (!E.runAnimation()); // wait for the text to finish
}

void loop()
{
    static uint32_t timeStartDelay;
    static uint8_t index = ARRAY_SIZE(eSeq);
    static enum
    {
        S_IDLE,
        S_TEXT,
        S_ANIM,
        S_PAUSE
    } state = S_IDLE;

    bool b = E.runAnimation(); // always run the animation

    switch (state)
    {
    case S_IDLE:
        index++;
        if (index >= ARRAY_SIZE(eSeq))
            index = 0;
        E.setText(eSeq[index].name);
        state = S_TEXT;
        break;

    case S_TEXT: // wait for the text to finish
        if (b)   // text animation is finished
        {
            E.setAnimation(eSeq[index].e, true);
            state = S_ANIM;
        }
        break;

    case S_ANIM: // checking animation is completed
        if (b)   // animation is finished
        {
            timeStartDelay = millis();
            state = S_PAUSE;
        }
        break;

    case S_PAUSE: // non blocking waiting for a period between animations
        if (millis() - timeStartDelay >= eSeq[index].timePause)
            state = S_IDLE;
        break;

    default:
        state = S_IDLE;
        break;
    }
}
