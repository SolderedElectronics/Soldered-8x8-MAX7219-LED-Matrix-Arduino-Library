/**
 **************************************************
 *
 * @file        Led_Matrix_PushWheel.ino
 *
 * @brief       Use the Led_Matrix library to create an mechanical pushwheel type display
 *
 *              When numbers change they are scrolled up or down as if on a cylinder
 *
 *              'Speed' displayed is read from pot on SPEED_IN analog in.
 *
 *              Wiring diagram:
 *              Dasduino   LED Matrix
 *                  |          |
 *                 VCC ------ VCC
 *                 GND ------ GND
 *                 D10 ------ LOAD
 *                 D11 ------ DIN
 *                 D13 ------ CLK
 *              Additionally, connect a potentiometer to an analog pin on the Dasduino (default is digital pin A5)
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

#include "Font_Data.h"
#include "Led-Matrix-SOLDERED.h"
#include <SPI.h>

#define DEBUG 0

#if DEBUG
#define PRINT(s, v)                                                                                                    \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print(v);                                                                                               \
    }
#define PRINTX(s, v)                                                                                                   \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print(v, HEX);                                                                                          \
    }
#define PRINTS(s) Serial.print(F(s));
#else
#define PRINT(s, v)
#define PRINTS(s)
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

// Define the number of devices we have in the chain and the hardware interface
#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define MAX_DEVICES   3

// Define matrix pins
#define CLK_PIN       13 // or SCK
#define DATA_PIN      11 // or MOSI
#define CS_PIN        10 // or LOAD

// SPI hardware interface
Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Analog input pin for the input value
#ifdef __AVR__
#define SPEED_IN A5
#else
#define SPEED_IN 5
#endif

// Display and animation parameters
#define CHAR_SPACING          1  // Pixels between characters
#define CHAR_COLS             5  // Should match the fixed width character columns
#define ANIMATION_FRAME_DELAY 30 // In milliseconds

// Structure to hold the data for each character to be displayed and animated
// this could be expanded to include other character specific data (eg, column
// where it starts if display is spaced irregularly).
struct digitData
{
    uint8_t oldValue, newValue; // ASCII value for the character
    uint8_t index;              // Animation progression index
    uint32_t timeLastFrame;     // Time the last frame started animating
    uint8_t charCols;           // Number of valid cols in the charMap
    uint8_t charMap[CHAR_COLS]; // Character font bitmap
};

// Do the necessary to display current bitmap buffer to the LED display
void updateDisplay(uint16_t numDigits, struct digitData *d)
{
    uint8_t curCol = 0;

    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    mx.clear();

    for (int8_t i = numDigits - 1; i >= 0; i--)
    {
        for (int8_t j = d[i].charCols - 1; j >= 0; j--)
        {
            mx.setColumn(curCol++, d[i].charMap[j]);
        }
        curCol += CHAR_SPACING;
    }

    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
}

// Display the required value on the LED matrix and return true if an animation is current
// Finite state machine will ignore new values while animations are underway.
// Needs to be called repeatedly to ensure animations are completed smoothly.
boolean displayValue(uint16_t value)
{
    const uint8_t DIGITS_SIZE = 3;
    static struct digitData digit[DIGITS_SIZE];

    const uint8_t ST_INIT = 0, ST_WAIT = 1, ST_ANIM = 2;
    static uint8_t state = ST_INIT;

    // Finite state machine to control what we do
    switch (state)
    {
    case ST_INIT: // Initialize the display - done once only on first call
        PRINTS("\nST_INIT");
        for (int8_t i = DIGITS_SIZE - 1; i >= 0; i--)
        {
            // Separate digits
            digit[i].oldValue = '0' + value % 10;
            value = value / 10;
        }

        // Display the starting number
        for (int8_t i = DIGITS_SIZE - 1; i >= 0; i--)
        {
            digit[i].charCols = mx.getChar(digit[i].oldValue, CHAR_COLS, digit[i].charMap);
        }
        updateDisplay(DIGITS_SIZE, digit);

        // Now we wait for a change
        state = ST_WAIT;
        break;

    case ST_WAIT: // Not animating - save new value digits and check if we need to animate
        PRINTS("\nST_WAIT");
        for (int8_t i = DIGITS_SIZE - 1; i >= 0; i--)
        {
            // Separate digits
            digit[i].newValue = '0' + value % 10;
            value = value / 10;

            if (digit[i].newValue != digit[i].oldValue)
            {
                // A change has been found - we will be animating something
                state = ST_ANIM;
                // Initialize animation parameters for this digit
                digit[i].index = 0;
                digit[i].timeLastFrame = 0;
            }
        }

        if (state == ST_WAIT) // no changes - keep waiting
            break;
        // Else fall through as we need to animate from now

    case ST_ANIM: // currently animating a change
        // work out the new intermediate bitmap for each character
        // 1. Get the 'new' character bitmap into temp buffer
        // 2. Shift this buffer down or up by current index amount
        // 3. Shift the current character by one pixel up or down
        // 4. Combine the new partial character and the existing character to produce a frame
        for (int8_t i = DIGITS_SIZE - 1; i >= 0; i--)
        {
            if ((digit[i].newValue != digit[i].oldValue) &&                   // Values are different
                (millis() - digit[i].timeLastFrame >= ANIMATION_FRAME_DELAY)) // Timer has expired
            {
                uint8_t newChar[CHAR_COLS] = {0};

                PRINT("\nST_ANIM Digit ", i);
                PRINT(" from '", (char)digit[i].oldValue);
                PRINT("' to '", (char)digit[i].newValue);
                PRINT("' index ", digit[i].index);

                mx.getChar(digit[i].newValue, CHAR_COLS, newChar);
                if (((digit[i].newValue > digit[i].oldValue) ||                 // Incrementing
                     (digit[i].oldValue == '9' && digit[i].newValue == '0')) && // Wrapping around on increase
                    !(digit[i].oldValue == '0' && digit[i].newValue == '9'))    // Not wrapping around on decrease
                {
                    // Scroll down
                    for (uint8_t j = 0; j < digit[i].charCols; j++)
                    {
                        newChar[j] = newChar[j] >> (COL_SIZE - 1 - digit[i].index);
                        digit[i].charMap[j] = digit[i].charMap[j] << 1;
                        digit[i].charMap[j] |= newChar[j];
                    }
                }
                else
                {
                    // Scroll up
                    for (uint8_t j = 0; j < digit[i].charCols; j++)
                    {
                        newChar[j] = newChar[j] << (COL_SIZE - 1 - digit[i].index);
                        digit[i].charMap[j] = digit[i].charMap[j] >> 1;
                        digit[i].charMap[j] |= newChar[j];
                    }
                }

                // Set new parameters for next animation and check if we are done
                digit[i].index++;
                digit[i].timeLastFrame = millis();
                if (digit[i].index >= COL_SIZE)
                    digit[i].oldValue = digit[i].newValue; // Done animating
            }
        }

        updateDisplay(DIGITS_SIZE, digit); // Show new display

        // Are we done animating?
        {
            boolean allDone = true;

            for (uint8_t i = 0; allDone && (i < DIGITS_SIZE); i++)
            {
                allDone = allDone && (digit[i].oldValue == digit[i].newValue);
            }

            if (allDone)
                state = ST_WAIT;
        }
        break;

    default:
        state = 0;
    }

    return (state == ST_WAIT); // Animation has ended
}

void setup()
{
#if DEBUG
    Serial.begin(115200);
#endif // DEBUG
    PRINTS("\n[Led_Matrix PushWheel]")

    mx.begin(); // Init matrix
    mx.setFont(numeric7Seg);

    pinMode(SPEED_IN, INPUT);
}

void loop()
{
    // Read velue from analog pina and remove jitters
    int16_t value = analogRead(SPEED_IN) / 10;

    // Display a value on the matrices
    displayValue(value);
}
