/**
 **************************************************
 *
 * @file        Led_Matrix_Zones.ino
 * @brief       Program to exercise the Led_Matrix library
 *
 *              Test the library transformation functions with range subsets
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

// We always wait a bit between updates of the display
#define DELAYTIME 300 // in milliseconds

// Define the number of devices we have in the chain and the hardware interface
#define MAX_DEVICES   2 // 2, 4, 6, or 8 work best - see Z array
#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define CS_PIN        10 // or LOAD

// SPI hardware interface
Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
// Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Global variables
uint32_t lastTime = 0;

typedef struct
{
    uint8_t startDev; // Start of zone
    uint8_t endDev;   // End of zone
    uint8_t ch;       // Character to show
    Led_Matrix::transformType_t tt;
} zoneDef_t;

// Define transformations depending on the device number
zoneDef_t Z[] = {
#if MAX_DEVICES == 2
    {0, 0, 26, Led_Matrix::TSR}, {1, 1, 27, Led_Matrix::TSL},
#endif // MAX_DEVICES 2
#if MAX_DEVICES == 4
    {0, 0, 26, Led_Matrix::TSR}, {1, 1, 25, Led_Matrix::TSD}, {2, 2, 24, Led_Matrix::TSU}, {3, 3, 27, Led_Matrix::TSL},
#endif // MAX_DEVICES 4
#if MAX_DEVICES == 6
    {0, 1, 26, Led_Matrix::TSR}, {2, 2, 24, Led_Matrix::TSU}, {3, 3, 25, Led_Matrix::TSD}, {4, 5, 27, Led_Matrix::TSL},
#endif // MAX_DEVICES 6
#if MAX_DEVICES == 8
    {0, 1, 26, Led_Matrix::TSR}, {2, 2, 24, Led_Matrix::TSU}, {3, 3, 25, Led_Matrix::TSD}, {4, 4, 24, Led_Matrix::TSU},
    {5, 5, 25, Led_Matrix::TSD}, {6, 7, 27, Led_Matrix::TSL},
#endif // MAX_DEVICES 8
};

#define ARRAY_SIZE(A) (sizeof(A) / sizeof(A[0]))

// Run transformation through each matrix
void runTransformation(void)
{
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    for (uint8_t i = 0; i < ARRAY_SIZE(Z); i++)
        mx.transform(Z[i].startDev, Z[i].endDev, Z[i].tt);

    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
}

void setup()
{
    // Init serial communication
    Serial.begin(115200);
    Serial.println("[Zone Transform Test]");

    // Init matrix
    mx.begin();
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::ON);

    // Set up the display characters
    for (uint8_t i = 0; i < ARRAY_SIZE(Z); i++)
    {
        mx.clear(Z[i].startDev, Z[i].endDev);
        for (uint8_t j = Z[i].startDev; j <= Z[i].endDev; j++)
            mx.setChar(((j + 1) * COL_SIZE) - 2, Z[i].ch);
    }
    lastTime = millis();

    // Enable the display
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
}

void loop()
{
    // Run transformation every 300 ms
    if (millis() - lastTime >= DELAYTIME)
    {
        runTransformation();
        lastTime = millis();
    }
}
