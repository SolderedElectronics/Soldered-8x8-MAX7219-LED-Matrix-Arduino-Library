/**
 **************************************************
 *
 * @file        Led_Matrix_Message_Serial.ino
 *
 * @brief       Demonstrates the use of the callback function to control what
 *              is scrolled on the display text.
 *
 *              User can enter text on the serial monitor at 115200 baud rate and
 *              this will display as a scrolling message on the display.
 *
 *              Speed for the display is controlled by a pot on SPEED_IN analog in.
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

#include "Led-Matrix-SOLDERED.h"
#include <SPI.h>

// User settings
#define IMMEDIATE_NEW   0 // If 1 will immediately display a new message
#define USE_POT_CONTROL 0
#define PRINT_CALLBACK  0

#define PRINT(s, v)                                                                                                    \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print(v);                                                                                               \
    }

// Define the number of devices we have in the chain and the hardware interface
#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define MAX_DEVICES   3

// Define matrix pins
#define CLK_PIN  13 // or SCK
#define DATA_PIN 11 // or MOSI
#define CS_PIN   10 // or LOAD

// SPI hardware interface
Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Scrolling parameters
#if USE_POT_CONTROL
#define SPEED_IN A5
#else
#define SCROLL_DELAY 75 // In milliseconds
#endif                  // USE_POT_CONTROL

#define CHAR_SPACING 1 // Pixels between characters

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE 75
uint8_t curMessage[BUF_SIZE] = {"Hello!  "};
uint8_t newMessage[BUF_SIZE];
bool newMessageAvailable = false;

uint16_t scrollDelay; // In milliseconds

// Read data from serial
void readSerial(void)
{
    static uint8_t putIndex = 0;

    while (Serial.available())
    {
        newMessage[putIndex] = (char)Serial.read();
        if ((newMessage[putIndex] == '\n') || (putIndex >= BUF_SIZE - 3)) // End of message character or full buffer
        {
            // Put in a message separator and end the string
            newMessage[putIndex++] = ' ';
            newMessage[putIndex] = '\0';
            // Restart the index for next filling spree and flag we have a message waiting
            putIndex = 0;
            newMessageAvailable = true;
        }
        else if (newMessage[putIndex] != '\r')
            // Just save the next char in next location
            putIndex++;
    }
}

// Callback function for data that is being scrolled off the display
void scrollDataSink(uint8_t dev, Led_Matrix::transformType_t t, uint8_t col)
{
#if PRINT_CALLBACK
    Serial.print("\n cb ");
    Serial.print(dev);
    Serial.print(' ');
    Serial.print(t);
    Serial.print(' ');
    Serial.println(col);
#endif
}

// Callback function for data that is required for scrolling into the display
uint8_t scrollDataSource(uint8_t dev, Led_Matrix::transformType_t t)
{
    static uint8_t *p = curMessage;
    static enum
    {
        NEW_MESSAGE,
        LOAD_CHAR,
        SHOW_CHAR,
        BETWEEN_CHAR
    } state = LOAD_CHAR;
    static uint8_t curLen, showLen;
    static uint8_t cBuf[15];
    uint8_t colData = 0; // Blank column is the default

#if IMMEDIATE_NEW
    if (newMessageAvailable) // There is a new message waiting
    {
        state = NEW_MESSAGE;
        mx.clear(); // Clear the display
    }
#endif

    // Finite state machine to control what we do on the callback
    switch (state)
    {
    case NEW_MESSAGE:                             // Load the new message
        memcpy(curMessage, newMessage, BUF_SIZE); // Copy it in
        newMessageAvailable = false;              // Used it!
        p = curMessage;
        state = LOAD_CHAR;
        break;

    case LOAD_CHAR: // Load the next character from the font table
        showLen = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state = SHOW_CHAR;

        // If we reached end of message, opportunity to load the next
        if (*p == '\0')
        {
            p = curMessage; // Reset the pointer to start of message
#if !IMMEDIATE_NEW
            if (newMessageAvailable) // There is a new message waiting
            {
                state = NEW_MESSAGE; // We will load it here
                break;
            }
#endif
        }
        // !! Deliberately fall through to next state to start displaying

    case SHOW_CHAR: // Display the next part of the character
        colData = cBuf[curLen++];
        if (curLen == showLen)
        {
            showLen = CHAR_SPACING;
            curLen = 0;
            state = BETWEEN_CHAR;
        }
        break;

    case BETWEEN_CHAR: // Display inter-character spacing (blank columns)
        colData = 0;
        curLen++;
        if (curLen == showLen)
            state = LOAD_CHAR;
        break;

    default:
        state = LOAD_CHAR;
    }

    return (colData);
}

// Shift all on the matrix to the left to get the scrolling effect
void scrollText(void)
{
    static uint32_t prevTime = 0;

    // Is it time to scroll the text?
    if (millis() - prevTime >= scrollDelay)
    {
        mx.transform(Led_Matrix::TSL); // Scroll along - the callback will load all the data
        prevTime = millis();           // Starting point for next time
    }
}

// Read the value from the potentiometer if it's used for
// controlling the delay or return default delay time
uint16_t getScrollDelay(void)
{
#if USE_POT_CONTROL
    uint16_t t;

    t = analogRead(SPEED_IN);
    t = map(t, 0, 1023, 25, 250);

    return (t);
#else
    return (SCROLL_DELAY);
#endif
}

void setup()
{
    // Init matrix
    mx.begin();

    // Set the Shift Data In callback functions
    mx.setShiftDataInCallback(scrollDataSource);
    mx.setShiftDataOutCallback(scrollDataSink);

    // Define potentiometer as input or use constant delay
#if USE_POT_CONTROL
    pinMode(SPEED_IN, INPUT);
#else
    scrollDelay = SCROLL_DELAY;
#endif

    newMessage[0] = '\0';

    // Init serial communication
    Serial.begin(115200);
    Serial.print(
        "\n[Led_Matrix Message Display]\nType a message for the scrolling display\nEnd message line with a newline");
}

void loop()
{
    // Read a message from serial and scroll it with a delay
    scrollDelay = getScrollDelay();
    readSerial();
    scrollText();
}
