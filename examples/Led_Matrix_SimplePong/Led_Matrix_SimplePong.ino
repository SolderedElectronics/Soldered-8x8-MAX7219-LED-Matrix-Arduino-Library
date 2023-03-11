/**
 **************************************************
 *
 * @file        Led_Matrix_SimplePong.ino
 * 
 * @brief       Use the Led_Matrix library to play Pong
 *
 *              Play pong on just one matrix. Bat is controlled by 2
 *              switches for left and right movement. Optionally use
 *              a pot on analog input to set the speed.
 *
 *              Wiring diagram:
 *              Dasduino   LED Matrix
 *                  |          |
 *                 VCC ------ VCC
 *                 GND ------ GND
 *                 D10 ------ LOAD
 *                 D11 ------ DIN
 *                 D13 ------ CLK
 *              Additionally, connect 2 switches to the pins on the Dasduino (default are digital pins 6 and 8)
 *              and potentiometer (optional) to the pin A5.
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

#define SPEED_FROM_ANALOG 1 // Optional to use analog input for speed control
#define DEBUG             0 // Enable or disable (default) debugging output

#if DEBUG

// Print a string followed by a value (decimal)
#define PRINT(s, v)                                                                                                    \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print(v);                                                                                               \
    } 
    
// Print a string followed by a value (hex)
#define PRINTX(s, v)                                                                                                   \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print(v, HEX);                                                                                          \
    }
    
// Print a string 
#define PRINTS(s)                                                                                                      \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
    } 
#else
#define PRINT(s, v)  // Print a string followed by a value (decimal)
#define PRINTX(s, v) // Print a string followed by a value (hex)
#define PRINTS(s)    // Print a string
#endif

// --------------------
// Led_Matrix hardware definitions and object
// Define the number of devices we have in the chain and the hardware interface
//
#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define MAX_DEVICES   1
#define CS_PIN        10 // or LOAD

Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES); // SPI hardware interface

// --------------------
// Mode switch parameters
//
const uint8_t LEFT_SWITCH = 8;  // Bat move right switch pin
const uint8_t RIGHT_SWITCH = 6; // Bat move right switch pin
#if SPEED_FROM_ANALOG
const uint8_t SPEED_POT = A5;
#endif

// --------------------
// Constant parameters
//
const uint32_t TEXT_MOVE_DELAY = 100; // In milliseconds
const uint32_t BAT_MOVE_DELAY = 50;   // In milliseconds
const uint32_t BALL_MOVE_DELAY = 150; // In milliseconds
const uint32_t END_GAME_DELAY = 2000; // In milliseconds

const uint8_t BAT_SIZE = 3; // In pixels, odd number looks best

char welcome[] = "PONG";
bool messageComplete;

// ========== General Variables ===========
//
uint32_t prevTime = 0;    // Used for remembering the mills() value
uint32_t prevBatTime = 0; // Used for bat timing

// ========== Control routines ===========
//

// Callback function for data that is required for scrolling into the display
uint8_t scrollDataSource(uint8_t dev, Led_Matrix::transformType_t t)
{
    static char *p;
    static enum
    {
        INIT,
        LOAD_CHAR,
        SHOW_CHAR,
        BETWEEN_CHAR
    } state = INIT;
    static uint8_t curLen, showLen;
    static uint8_t cBuf[15];
    uint8_t colData = 0; // Blank column is the default

    // Finite state machine to control what we do on the callback
    switch (state)
    {
    case INIT: // Load the new message
        p = welcome;
        messageComplete = false;
        state = LOAD_CHAR;
        break;

    case LOAD_CHAR: // Load the next character from the font table
        showLen = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state = SHOW_CHAR;

        // !! Deliberately fall through to next state to start displaying

    case SHOW_CHAR: // Display the next part of the character
        colData = cBuf[curLen++];
        if (curLen == showLen)
        {
            if (*p == '\0') // End of message!
            {
                messageComplete = true;
                state = INIT;
            }
            else // More to come
            {
                showLen = 1;
                curLen = 0;
                state = BETWEEN_CHAR;
            }
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

// Shift a text to the left
void scrollText(void)
{
    // Is it time to scroll the text?
    if (millis() - prevTime >= TEXT_MOVE_DELAY)
    {
        mx.transform(Led_Matrix::TSL); // Scroll along - the callback will load all the data
        prevTime = millis();           // Starting point for next time
    }
}

// Clear the matrix and set the default intensity
void resetDisplay(void)
{
    mx.control(Led_Matrix::INTENSITY, MAX_INTENSITY / 2);
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
    mx.clear();
}

// Read the state of the left button
inline bool swL(void)
{
    return (digitalRead(LEFT_SWITCH) == LOW);
}

// Read the state of the right button
inline bool swR(void)
{
    return (digitalRead(RIGHT_SWITCH) == LOW);
}

#if SPEED_FROM_ANALOG
// Read the speed value from the pot
inline uint32_t speed(void)
{
    return (30 + analogRead(SPEED_POT) / 4);
}
#else
// Use constant speed value
inline uint32_t speed(void)
{
    return (BALL_MOVE_DELAY);
}
#endif

// Draw a bat
void drawBat(int8_t x, int8_t y, bool bOn = true)
{
    for (uint8_t i = 0; i < BAT_SIZE; i++)
        mx.setPoint(y, x + i, bOn);
}

// Draw a ball
void drawBall(int8_t x, int8_t y, bool bOn = true)
{
    mx.setPoint(y, x, bOn);
}

void setup(void)
{
    // Init matrix
    mx.begin();

    // Define input pins
    pinMode(LEFT_SWITCH, INPUT_PULLUP);
    pinMode(RIGHT_SWITCH, INPUT_PULLUP);

    
#if SPEED_FROM_ANALOG
    // Define analog input if it's used
    pinMode(SPEED_POT, INPUT);
#endif

#if DEBUG
    // Init serial communication if it's used
    Serial.begin(115200);
#endif

    PRINTS("\n[Led_Matrix Simple Pong]");
}

void loop(void)
{
    // Define all possible game states
    static enum : uint8_t
    {
        INIT,
        WELCOME,
        PLAY_INIT,
        WAIT_START,
        PLAY,
        END
    } state = INIT;

    // Variables for positions
    static int8_t ballX, ballY;
    static int8_t batX;
    const int8_t batY = ROW_SIZE - 1;

    // Initialisesd in FSM
    static int8_t deltaX, deltaY; 

    // Execute parts of the game depending on the current state
    switch (state)
    {
    case INIT:
        PRINTS("\n>>INIT");
        resetDisplay();
        mx.setShiftDataInCallback(scrollDataSource);
        prevTime = 0;
        state = WELCOME;
        break;

    case WELCOME:
        PRINTS("\n>>WELCOME");
        scrollText();
        if (messageComplete)
            state = PLAY_INIT;
        break;

    case PLAY_INIT:
        PRINTS("\n>>PLAY_INIT");
        mx.setShiftDataInCallback(nullptr);
        state = WAIT_START;
        mx.clear();
        batX = (COL_SIZE - BAT_SIZE) / 2;
        ballX = batX + (BAT_SIZE / 2);
        ballY = batY - 1;
        deltaY = -1; // Always heading up at the start
        deltaX = 0;  // Initialized in the direction of first bat movement
        drawBat(batX, batY);
        drawBall(ballX, ballY);
        break;

    case WAIT_START:
        // PRINTS("\n>>WAIT_START");
        if (swL())
            deltaX = 1;
        if (swR())
            deltaX = -1;
        if (deltaX != 0)
        {
            prevTime = prevBatTime = millis();
            state = PLAY;
        }
        break;

    case PLAY:
        // === Move the bat if time has expired
        if (millis() - prevBatTime >= BAT_MOVE_DELAY)
        {
            if (swL()) // Left switch move
            {
                PRINTS("\n>>PLAY - move bat L");
                drawBat(batX, batY, false);
                batX++;
                if (batX + BAT_SIZE >= COL_SIZE)
                    batX = COL_SIZE - BAT_SIZE;
                drawBat(batX, batY);
            }

            if (swR()) // Right switch move
            {
                PRINTS("\n>>PLAY - move bat R");
                drawBat(batX, batY, false);
                batX--;
                if (batX < 0)
                    batX = 0;
                drawBat(batX, batY);
            }

            prevBatTime = millis(); // Set up for next time;
        }

        // === Move the ball if its time to do so
        if (millis() - prevTime >= speed())
        {
            PRINTS("\n>>PLAY - ");

            drawBall(ballX, ballY, false);

            // New ball positions
            ballX += deltaX;
            ballY += deltaY;

            // Check for edge collisions
            if (ballX >= COL_SIZE - 1 || ballX <= 0) // Side bounce
            {
                PRINTS("side bounce");
                deltaX *= -1;
            }
            if (ballY <= 0)
            {
                PRINTS("top bounce");
                deltaY *= -1; // Sop bounce
            }

            //=== Check for side bounce/bat collision
            if (ballY == batY - 1 && deltaY == 1) // Just above the bat and travelling towards it
            {
                PRINT("check bat x=", batX);
                PRINTS(" - ");
                if ((ballX >= batX) && (ballX <= batX + BAT_SIZE - 1)) // Over the bat - just bounce vertically
                {
                    deltaY = -1;
                    PRINT("bounce off dy=", deltaY);
                }
                else if ((ballX == batX - 1) || ballX == batX + BAT_SIZE) // Hit corner of bat - also bounce horizontal
                {
                    deltaY = -1;
                    if (ballX != COL_SIZE - 1 && ballX != 0) // Edge effects elimination
                        deltaX *= -1;
                    PRINT("hit corner dx=", deltaX);
                    PRINT(" dy=", deltaY);
                }
            }

            drawBall(ballX, ballY);

            // Check if end of game
            if (ballY == batY)
            {
                PRINTS("\n>>PLAY - past bat! -> end of game");
                state = END;
            }

            prevTime = millis();
        }
        break;

    case END:
        if (millis() - prevTime >= END_GAME_DELAY)
        {
            PRINTS("\n>>END");
            state = PLAY_INIT;
        }
        break;

    default: // Just in case
        PRINT("\n>>UNHANDLED !!! ", state);
        state = INIT;
        break;
    }
}
