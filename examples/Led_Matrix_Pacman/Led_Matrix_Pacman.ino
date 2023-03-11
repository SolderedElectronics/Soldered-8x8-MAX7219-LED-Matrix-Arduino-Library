/**
 **************************************************
 *
 * @file        Led_Matrix_Pacman.ino
 * 
 * @brief       Use the Led_Matrix library to display a Pacman animation
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

#define DEBUG 0 // Enable or disable (default) debugging output

#if DEBUG
#define PRINT(s, v)                                                                                                    \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print(v);                                                                                               \
    } // Print a string followed by a value (decimal)
#define PRINTX(s, v)                                                                                                   \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print(v, HEX);                                                                                          \
    } // Print a string followed by a value (hex)
#define PRINTB(s, v)                                                                                                   \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print(v, BIN);                                                                                          \
    } // Print a string followed by a value (binary)
#define PRINTC(s, v)                                                                                                   \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print((char)v);                                                                                         \
    } // Print a string followed by a value (char)
#define PRINTS(s)                                                                                                      \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
    } // Print a string
#else
#define PRINT(s, v)  // Print a string followed by a value (decimal)
#define PRINTX(s, v) // Print a string followed by a value (hex)
#define PRINTB(s, v) // Print a string followed by a value (binary)
#define PRINTC(s, v) // Print a string followed by a value (char)
#define PRINTS(s)    // Print a string
#endif

// --------------------
// Led_Matrix hardware definitions and object
// Define the number of devices we have in the chain and the hardware interface
//
#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define MAX_DEVICES   3
#define CLK_PIN       13 // or SCK
#define DATA_PIN      11 // or MOSI
#define CS_PIN        10 // or LOAD

Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES); // SPI hardware interface

// --------------------
// Constant parameters
//
#define ANIMATION_DELAY 75 // Milliseconds
#define MAX_FRAMES      4  // Number of animation frames

// ========== General Variables ===========
//
const uint8_t pacman[MAX_FRAMES][18] = // Ghost pursued by a pacman
    {
        {0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe, 0x00, 0x00, 0x00, 0x3c, 0x7e, 0x7e, 0xff, 0xe7, 0xc3, 0x81, 0x00},
        {0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe, 0x00, 0x00, 0x00, 0x3c, 0x7e, 0xff, 0xff, 0xe7, 0xe7, 0x42, 0x00},
        {0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe, 0x00, 0x00, 0x00, 0x3c, 0x7e, 0xff, 0xff, 0xff, 0xe7, 0x66, 0x24},
        {0xfe, 0x7b, 0xf3, 0x7f, 0xf3, 0x7b, 0xfe, 0x00, 0x00, 0x00, 0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c},
};
const uint8_t DATA_WIDTH = (sizeof(pacman[0]) / sizeof(pacman[0][0]));

uint32_t prevTimeAnim = 0; // Remember the millis() value in animations
int16_t idx;               // Display index (column)
uint8_t frame;             // Current animation frame
uint8_t deltaFrame;        // The animation frame offset for the next frame

// ========== Control routines ===========
//
void resetMatrix(void)
{
    mx.control(Led_Matrix::INTENSITY, MAX_INTENSITY / 2);
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
    mx.clear();
}

void setup()
{
    mx.begin(); // Init matrix
    resetMatrix();
    prevTimeAnim = millis(); // Remember the last animation time

    // Init serial communication if it's needed
#if DEBUG
    Serial.begin(115200);
#endif

    PRINTS("\n[Led_Matrix Pacman]");
}

void loop(void)
{
    static boolean bInit = true; // Initialise the animation

    // Is it time to animate?
    if (millis() - prevTimeAnim < ANIMATION_DELAY)
        return;
    prevTimeAnim = millis(); // Starting point for next time

    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    // Initialize
    if (bInit)
    {
        mx.clear();
        idx = -DATA_WIDTH;
        frame = 0;
        deltaFrame = 1;
        bInit = false;

        // Lay out the dots
        for (uint8_t i = 0; i < MAX_DEVICES; i++)
        {
            mx.setPoint(3, (i * COL_SIZE) + 3, true);
            mx.setPoint(4, (i * COL_SIZE) + 3, true);
            mx.setPoint(3, (i * COL_SIZE) + 4, true);
            mx.setPoint(4, (i * COL_SIZE) + 4, true);
        }
    }

    // Now run the animation
    PRINT("\nINV I:", idx);
    PRINT(" frame ", frame);

    // Clear old graphic
    for (uint8_t i = 0; i < DATA_WIDTH; i++)
        mx.setColumn(idx - DATA_WIDTH + i, 0);
    // Move reference column and draw new graphic
    idx++;
    for (uint8_t i = 0; i < DATA_WIDTH; i++)
        mx.setColumn(idx - DATA_WIDTH + i, pacman[frame][i]);

    // Advance the animation frame
    frame += deltaFrame;
    if (frame == 0 || frame == MAX_FRAMES - 1)
        deltaFrame = -deltaFrame;

    // Check if we are completed and set initialise for next time around
    bInit = (idx == mx.getColumnCount() + DATA_WIDTH);

    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return;
}
