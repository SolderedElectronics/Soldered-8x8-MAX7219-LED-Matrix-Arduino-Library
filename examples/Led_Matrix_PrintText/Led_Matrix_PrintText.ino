/**
 **************************************************
 *
 * @file        Led_Matrix_PrintText.ino
 * 
 * @brief       Use the Led_Matrix library to Print some text on the display
 *
 *              Demonstrates the use of the library to print text.
 *
 *              User can enter text on the serial monitor and this will display as a
 *              message on the display.
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

#define PRINT(s, v)                                                                                                    \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print(v);                                                                                               \
    }

// Define the number of devices we have in the chain and the hardware interface

#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define MAX_DEVICES   3
#define CLK_PIN       13 // or SCK
#define DATA_PIN      11 // or MOSI
#define CS_PIN        10 // or LOAD

// SPI hardware interface
Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Text parameters
#define CHAR_SPACING 1 // Pixels between characters

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE 75
char message[BUF_SIZE] = "Hello!";
bool newMessageAvailable = true;

// Read a message from serial
void readSerial(void)
{
    static uint8_t putIndex = 0;

    while (Serial.available())
    {
        message[putIndex] = (char)Serial.read();
        if ((message[putIndex] == '\n') || (putIndex >= BUF_SIZE - 3)) // End of message character or full buffer
        {
            // Put in a message separator and end the string
            message[putIndex] = '\0';
            // Restart the index for next filling spree and flag we have a message waiting
            putIndex = 0;
            newMessageAvailable = true;
        }
        else
            // Just save the next char in next location
            message[putIndex++];
    }
}

void printText(uint8_t modStart, uint8_t modEnd, char *pMsg)
// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
{
    uint8_t state = 0;
    uint8_t curLen;
    uint16_t showLen;
    uint8_t cBuf[8];
    int16_t col = ((modEnd + 1) * COL_SIZE) - 1;

    mx.control(modStart, modEnd, Led_Matrix::UPDATE, Led_Matrix::OFF);

    do // Finite state machine to print the characters in the space available
    {
        switch (state)
        {
        case 0: // Load the next character from the font table
            // If we reached end of message, reset the message pointer
            if (*pMsg == '\0')
            {
                showLen = col - (modEnd * COL_SIZE); // Padding characters
                state = 2;
                break;
            }

            // Retrieve the next character form the font file
            showLen = mx.getChar(*pMsg++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
            curLen = 0;
            state++;
            // !! Deliberately fall through to next state to start displaying

        case 1: // Display the next part of the character
            mx.setColumn(col--, cBuf[curLen++]);

            // Done with font character, now display the space between chars
            if (curLen == showLen)
            {
                showLen = CHAR_SPACING;
                state = 2;
            }
            break;

        case 2: // Initialize state for displaying empty columns
            curLen = 0;
            state++;
            // Fall through

        case 3: // Display inter-character spacing or end of message padding (blank columns)
            mx.setColumn(col--, 0);
            curLen++;
            if (curLen == showLen)
                state = 0;
            break;

        default:
            col = -1; // This definitely ends the do loop
        }
    } while (col >= (modStart * COL_SIZE));

    mx.control(modStart, modEnd, Led_Matrix::UPDATE, Led_Matrix::ON);
}

void setup()
{
    // Init matrix
    mx.begin();

    // Init serial communication
    Serial.begin(115200);
    Serial.print("\n[Led_Matrix Message Display]\nType a message for the display\nEnd message line with a newline");
}

void loop()
{
    // Read a message from serial
    readSerial();

    // If there is a new message
    if (newMessageAvailable)
    {
        // Print it on the matrices and Serial Monitor
        PRINT("\nProcessing new message: ", message);
        printText(0, MAX_DEVICES - 1, message);
        newMessageAvailable = false;
    }
}
