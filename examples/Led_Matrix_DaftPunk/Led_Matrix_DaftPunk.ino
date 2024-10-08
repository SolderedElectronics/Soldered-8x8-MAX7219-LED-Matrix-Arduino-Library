/**
 **************************************************
 *
 * @file        Led_Matrix_DaftPunk.ino
 * 
 * @brief       If RUN_DEMO is set to zero the display cycles changes triggered by a switch on
 *              the MODE_SWITCH pin. This can be substituted for any trigger as implemented
 *              by the helmet wearer.
 *              If RUN_DEMO is set to 1 the sketch will cycle each element of the display every
 *              DEMO_DELAY seconds, without the need for a switch.
 *
 *              Uses the MD_UISwitch library found at https://github.com/MajicDesigns/MD_UISwitch
 *
 *              Wiring diagram:
 *              Dasduino   LED Matrix
 *                  |          |
 *                 VCC ------ VCC
 *                 GND ------ GND
 *                 D10 ------ LOAD
 *                 D11 ------ DIN
 *                 D13 ------ CLK
 *              Additionally, connect a switch to a pin on the Dasduino (default is digital pin 9)
 *              if you want to use it.
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

#define RUN_DEMO 1

#include "Led-Matrix-SOLDERED.h"
#include <SPI.h>

#if RUN_DEMO
#define DEMO_DELAY 15 // Time to show each demo element in seconds
#else
#include <MD_UISwitch.h>
#endif

#define DEBUG 1 // Enable or disable (default) debugging output at 115200 baud rate

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

// Led_Matrix hardware definitions and object
// Define the number of devices we have in the chain and the hardware interface
#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define MAX_DEVICES   3

// Define matrix pins
#define CLK_PIN       13 // or SCK
#define DATA_PIN      11 // or MOSI
#define CS_PIN        10 // or LOAD

Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES); // SPI hardware interface

#if !RUN_DEMO
// --------------------
// Mode keyswitch parameters and object
//
#define MODE_SWITCH 9 // Digital Pin

MD_UISwitch_Digital ks = MD_UISwitch_Digital(MODE_SWITCH, LOW);
#endif

// --------------------
// Constant parameters
//
// Various delays in milliseconds
#define UNIT_DELAY      25
#define SCROLL_DELAY    (4 * UNIT_DELAY)
#define MIDLINE_DELAY   (6 * UNIT_DELAY)
#define SCANNER_DELAY   (2 * UNIT_DELAY)
#define RANDOM_DELAY    (6 * UNIT_DELAY)
#define FADE_DELAY      (8 * UNIT_DELAY)
#define SPECTRUM_DELAY  (4 * UNIT_DELAY)
#define HEARTBEAT_DELAY (1 * UNIT_DELAY)
#define HEARTS_DELAY    (28 * UNIT_DELAY)
#define EYES_DELAY      (20 * UNIT_DELAY)
#define WIPER_DELAY     (1 * UNIT_DELAY)
#define ARROWS_DELAY    (3 * UNIT_DELAY)
#define ARROWR_DELAY    (8 * UNIT_DELAY)
#define INVADER_DELAY   (6 * UNIT_DELAY)
#define PACMAN_DELAY    (4 * UNIT_DELAY)
#define SINE_DELAY      (2 * UNIT_DELAY)

#define CHAR_SPACING 1  // Pixels between characters
#define BUF_SIZE     75 // Character buffer size

// ========== General Variables ===========
//
uint32_t prevTimeAnim = 0; // Used for remembering the millis() value in animations
#if RUN_DEMO
uint32_t prevTimeDemo = 0;     // Used for remembering the millis() time in demo loop
uint8_t timeDemo = DEMO_DELAY; // Number of seconds left in this demo loop
#endif

// ========== Text routines ===========
//
// Text Message Table
// To change messages simply reorder, add to, or delete from, this table
const char *msgTab[] = {
    "DAFT PUNK", "GET LUCKY", "ONE MORE TIME", "HARDER  BETTER  FASTER  STRONGER", "HUMAN AND ROBOT", "TECHNOLOGIC",
};

// Callback function for data that is required for scrolling into the display
bool scrollText(bool bInit, const char *pmsg)
{
    static char curMessage[BUF_SIZE];
    static char *p = curMessage;
    static uint8_t state = 0;
    static uint8_t curLen, showLen;
    static uint8_t cBuf[8];
    uint8_t colData;

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Initializing ScrollText");
        resetMatrix();
        strcpy(curMessage, pmsg);
        state = 0;
        p = curMessage;
        bInit = false;
    }

    // Is it time to scroll the text?
    if (millis() - prevTimeAnim < SCROLL_DELAY)
        return (bInit);

    // Scroll the display
    mx.transform(Led_Matrix::TSL); // Scroll along
    prevTimeAnim = millis();       // Starting point for next time

    // Now run the finite state machine to control what we do
    PRINT("\nScroll FSM S:", state);
    switch (state)
    {
    case 0: // Load the next character from the font table
        PRINTC("\nLoading ", *p);
        showLen = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state = 1;

        // !! Deliberately fall through to next state to start displaying

    case 1: // Display the next part of the character
        colData = cBuf[curLen++];
        mx.setColumn(0, colData);
        if (curLen == showLen)
        {
            showLen = ((*p != '\0') ? CHAR_SPACING : mx.getColumnCount() - 1);
            curLen = 0;
            state = 2;
        }
        break;

    case 2: // Display inter-character spacing (blank column) or scroll off the display
        mx.setColumn(0, 0);
        if (++curLen == showLen)
        {
            state = 0;
            bInit = (*p == '\0');
        }
        break;

    default:
        state = 0;
    }

    return (bInit);
}

// ========== Graphic routines ===========

// Draw a midline
bool graphicMidline1(bool bInit)
{
    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Midline1 init");
        resetMatrix();
        bInit = false;
    }
    else
    {
        // Set midline on each matrix
        for (uint8_t j = 0; j < MAX_DEVICES; j++)
        {
            mx.setRow(j, 3, 0xff);
            mx.setRow(j, 4, 0xff);
        }
    }

    return (bInit);
}

// Draw animate midline
bool graphicMidline2(bool bInit)
{
    static uint8_t idx = 0;   // Position
    static int8_t idOffs = 1; // Increment direction

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Midline2 init");
        resetMatrix();
        idx = 0;
        idOffs = 1;
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < MIDLINE_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Debug messages if it's enabled
    PRINT("\nML2 R:", idx);
    PRINT(" D:", idOffs);

    // Now run the animation
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    // Turn off the old lines
    for (uint8_t j = 0; j < MAX_DEVICES; j++)
    {
        mx.setRow(j, idx, 0x00);
        mx.setRow(j, ROW_SIZE - 1 - idx, 0x00);
    }

    // Change offset direction when lines went to one of the edges
    idx += idOffs;
    if ((idx == 0) || (idx == ROW_SIZE - 1))
        idOffs = -idOffs;

    // Turn on the new lines
    for (uint8_t j = 0; j < MAX_DEVICES; j++)
    {
        mx.setRow(j, idx, 0xff);
        mx.setRow(j, ROW_SIZE - 1 - idx, 0xff);
    }

    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Animate a scanner effect
bool graphicScanner(bool bInit)
{
    const uint8_t width = 3;  // Scanning bar width
    static uint8_t idx = 0;   // Position
    static int8_t idOffs = 1; // Increment direction

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Scanner init");
        resetMatrix();
        idx = 0;
        idOffs = 1;
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < SCANNER_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    PRINT("\nS R:", idx);
    PRINT(" D:", idOffs);

    // Now run the animation
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    // Turn off the old lines
    for (uint8_t i = 0; i < width; i++)
        mx.setColumn(idx + i, 0);

    // Change offset direction when lines went to one of the edges
    idx += idOffs;
    if ((idx == 0) || (idx + width == mx.getColumnCount()))
        idOffs = -idOffs;

    // Turn on the new lines
    for (uint8_t i = 0; i < width; i++)
        mx.setColumn(idx + i, 0xff);

    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Random turn on and off LEDs
bool graphicRandom(bool bInit)
{
    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Random init");
        resetMatrix();
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < RANDOM_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Now run the animation
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    for (uint8_t i = 0; i < mx.getColumnCount(); i++)
        mx.setColumn(i, (uint8_t)random(255));
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Scroll the strips
bool graphicScroller(bool bInit)
{
    const uint8_t width = 3; // Width of the scroll bar
    const uint8_t offset = mx.getColumnCount() / 3;
    static uint8_t idx = 0; // Counter

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Scroller init");
        resetMatrix();
        idx = 0;
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < SCANNER_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    PRINT("\nS I:", idx);

    // Now run the animation
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    mx.transform(Led_Matrix::TSL);

    mx.setColumn(0, idx >= 0 && idx < width ? 0xff : 0);
    if (++idx == offset)
        idx = 0;

    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Random spectrum on each matrix
bool graphicSpectrum1(bool bInit)
{
    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Spectrum1 init");
        resetMatrix();
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < SPECTRUM_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Now run the animation
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    for (uint8_t i = 0; i < MAX_DEVICES; i++)
    {
        uint8_t r = random(ROW_SIZE); // Get random value
        uint8_t cd = 0;

        // Fill the columns until the random number
        for (uint8_t j = 0; j < r; j++)
            cd |= 1 << j;
        for (uint8_t j = 1; j < COL_SIZE - 1; j++)
            mx.setColumn(i, j, ~cd);
    }
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Random spectrum on each matrix column
bool graphicSpectrum2(bool bInit)
{
    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Spectrum2init");
        resetMatrix();
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < SPECTRUM_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Now run the animation
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    for (uint8_t i = 0; i < mx.getColumnCount(); i++)
    {
        uint8_t r = random(ROW_SIZE); // Get random value
        uint8_t cd = 0;

        // Fill the columns until the random number
        for (uint8_t j = 0; j < r; j++)
            cd |= 1 << j;

        mx.setColumn(i, ~cd);
    }
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Simulation of heartbeat
bool graphicHeartbeat(bool bInit)
{
#define BASELINE_ROW 4

    static uint8_t state;
    static uint8_t r, c;
    static bool bPoint;

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Heartbeat init");
        resetMatrix();
        state = 0;
        r = BASELINE_ROW;
        c = mx.getColumnCount() - 1;
        bPoint = true;
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < HEARTBEAT_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Now run the animation
    PRINT("\nHB S:", state);
    PRINT(" R: ", r);
    PRINT(" C: ", c);
    PRINT(" P: ", bPoint);
    mx.setPoint(r, c, bPoint);

    switch (state)
    {
    case 0: // Straight line from the right side
        if (c == mx.getColumnCount() / 2 + COL_SIZE)
            state = 1;
        c--;
        break;

    case 1: // First stroke
        if (r != 0)
        {
            r--;
            c--;
        }
        else
            state = 2;
        break;

    case 2: // Down stroke
        if (r != ROW_SIZE - 1)
        {
            r++;
            c--;
        }
        else
            state = 3;
        break;

    case 3: // Second up stroke
        if (r != BASELINE_ROW)
        {
            r--;
            c--;
        }
        else
            state = 4;
        break;

    case 4: // Straight line to the left
        if (c == 0)
        {
            c = mx.getColumnCount() - 1;
            bPoint = !bPoint;
            state = 0;
        }
        else
            c--;
        break;

    default:
        state = 0;
    }

    return (bInit);
}

// Increase and decrease matrix brightness
bool graphicFade(bool bInit)
{
    static uint8_t intensity = 0;
    static int8_t iOffs = 1;

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Fade init");
        resetMatrix();
        mx.control(Led_Matrix::INTENSITY, intensity);

        // Set all LEDS on
        mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
        for (uint8_t i = 0; i < mx.getColumnCount(); i++)
            mx.setColumn(i, 0xff);
        mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < FADE_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Now run the animation
    intensity += iOffs;
    PRINT("\nF I:", intensity);
    PRINT(" D:", iOffs);
    if ((intensity == 0) || (intensity == MAX_INTENSITY))
        iOffs = -iOffs;
    mx.control(Led_Matrix::INTENSITY, intensity);

    return (bInit);
}

// Draw empty and filled hearts
bool graphicHearts(bool bInit)
{
#define NUM_HEARTS ((MAX_DEVICES / 2) + 1)
    const uint8_t heartFull[] = {0x1c, 0x3e, 0x7e, 0xfc};
    const uint8_t heartEmpty[] = {0x1c, 0x22, 0x42, 0x84};
    const uint8_t offset = mx.getColumnCount() / (NUM_HEARTS + 1);
    const uint8_t dataSize = (sizeof(heartFull) / sizeof(heartFull[0]));

    static bool bEmpty;

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Hearts init");
        resetMatrix();
        bEmpty = true;
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < HEARTS_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Now run the animation
    PRINT("\nH E:", bEmpty);

    // Draw filled or empty hearts
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    for (uint8_t h = 1; h <= NUM_HEARTS; h++)
    {
        for (uint8_t i = 0; i < dataSize; i++)
        {
            mx.setColumn((h * offset) - dataSize + i, bEmpty ? heartEmpty[i] : heartFull[i]);
            mx.setColumn((h * offset) + dataSize - i - 1, bEmpty ? heartEmpty[i] : heartFull[i]);
        }
    }
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
    bEmpty = !bEmpty;

    return (bInit);
}

// Draw blinking eyes
bool graphicEyes(bool bInit)
{
#define NUM_EYES 2
    const uint8_t eyeOpen[] = {0x18, 0x3c, 0x66, 0x66};
    const uint8_t eyeClose[] = {0x18, 0x3c, 0x3c, 0x3c};
    const uint8_t offset = mx.getColumnCount() / (NUM_EYES + 1);
    const uint8_t dataSize = (sizeof(eyeOpen) / sizeof(eyeOpen[0]));

    bool bOpen;

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Eyes init");
        resetMatrix();
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < EYES_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Now run the animation
    bOpen = (random(1000) > 100);
    PRINT("\nH E:", bOpen);

    // Draw open or closed eyes
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    for (uint8_t e = 1; e <= NUM_EYES; e++)
    {
        for (uint8_t i = 0; i < dataSize; i++)
        {
            mx.setColumn((e * offset) - dataSize + i, bOpen ? eyeOpen[i] : eyeClose[i]);
            mx.setColumn((e * offset) + dataSize - i - 1, bOpen ? eyeOpen[i] : eyeClose[i]);
        }
    }
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Animation of bouncing ball
bool graphicBounceBall(bool bInit)
{
    static uint8_t idx = 0;   // Position
    static int8_t idOffs = 1; // Increment direction

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- BounceBall init");
        resetMatrix();
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < SCANNER_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    PRINT("\nBB R:", idx);
    PRINT(" D:", idOffs);

    // Now run the animation
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    // Turn off the old ball
    mx.setColumn(idx, 0);
    mx.setColumn(idx + 1, 0);

    // Change the direction when the ball goes to the edge
    idx += idOffs;
    if ((idx == 0) || (idx == mx.getColumnCount() - 2))
        idOffs = -idOffs;

    // Turn on the new lines
    mx.setColumn(idx, 0x18);
    mx.setColumn(idx + 1, 0x18);

    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Scroll the arrows over all matrices
bool graphicArrowScroll(bool bInit)
{
    const uint8_t arrow[] = {0x3c, 0x66, 0xc3, 0x99}; // Arrow bitmap
    const uint8_t dataSize = (sizeof(arrow) / sizeof(arrow[0]));

    static uint8_t idx = 0;

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- ArrowScroll init");
        resetMatrix();
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < ARROWS_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    PRINT("\nAR I:", idx);

    // Now run the animation
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    mx.transform(Led_Matrix::TSL);
    mx.setColumn(0, arrow[idx++]);
    if (idx == dataSize)
        idx = 0;
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Animation of wiping over all matrices
bool graphicWiper(bool bInit)
{
    static uint8_t idx = 0;   // Position
    static int8_t idOffs = 1; // Increment direction

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Wiper init");
        resetMatrix();
        bInit = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < WIPER_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    PRINT("\nW R:", idx);
    PRINT(" D:", idOffs);

    // Now run the animation
    mx.setColumn(idx, idOffs == 1 ? 0xff : 0);
    idx += idOffs;
    if ((idx == 0) || (idx == mx.getColumnCount()))
        idOffs = -idOffs;

    return (bInit);
}

// Animation of the invader
bool graphicInvader(bool bInit)
{
    // Bitmap of invader
    const uint8_t invader1[] = {0x0e, 0x98, 0x7d, 0x36, 0x3c};
    const uint8_t invader2[] = {0x70, 0x18, 0x7d, 0xb6, 0x3c};
    const uint8_t dataSize = (sizeof(invader1) / sizeof(invader1[0]));

    static int8_t idx;
    static bool iType;

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Invader init");
        resetMatrix();
        bInit = false;
        idx = -dataSize;
        iType = false;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < INVADER_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Now run the animation
    PRINT("\nINV I:", idx);

    // Draw the invader
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    mx.clear();
    for (uint8_t i = 0; i < dataSize; i++)
    {
        mx.setColumn(idx - dataSize + i, iType ? invader1[i] : invader2[i]);
        mx.setColumn(idx + dataSize - i - 1, iType ? invader1[i] : invader2[i]);
    }
    idx++;
    if (idx == mx.getColumnCount() + (dataSize * 2))
        bInit = true;
    iType = !iType;
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Draw animation of paceman
bool graphicPacman(bool bInit)
{
#define MAX_FRAMES    4 // Number of animation frames
#define PM_DATA_WIDTH 18
    const uint8_t pacman[MAX_FRAMES][PM_DATA_WIDTH] = // Ghost pursued by a pacman
        {
            {0x3c, 0x7e, 0x7e, 0xff, 0xe7, 0xc3, 0x81, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73,
             0xfe},
            {0x3c, 0x7e, 0xff, 0xff, 0xe7, 0xe7, 0x42, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73,
             0xfe},
            {0x3c, 0x7e, 0xff, 0xff, 0xff, 0xe7, 0x66, 0x24, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73,
             0xfe},
            {0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73,
             0xfe},
        };

    static int16_t idx;        // Display index (column)
    static uint8_t frame;      // Current animation frame
    static uint8_t deltaFrame; // The animation frame offset for the next frame

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Pacman init");
        resetMatrix();
        bInit = false;
        idx = -1; // DATA_WIDTH;
        frame = 0;
        deltaFrame = 1;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < PACMAN_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    PRINT("\nPAC I:", idx);

    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    mx.clear();

    // Clear old graphic
    for (uint8_t i = 0; i < PM_DATA_WIDTH; i++)
        mx.setColumn(idx - PM_DATA_WIDTH + i, 0);
    // Move reference column and draw new graphic
    idx++;
    for (uint8_t i = 0; i < PM_DATA_WIDTH; i++)
        mx.setColumn(idx - PM_DATA_WIDTH + i, pacman[frame][i]);

    // Advance the animation frame
    frame += deltaFrame;
    if (frame == 0 || frame == MAX_FRAMES - 1)
        deltaFrame = -deltaFrame;

    // Check if we are completed and set initialize for next time around
    if (idx == mx.getColumnCount() + PM_DATA_WIDTH)
        bInit = true;

    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    return (bInit);
}

// Draw, move, and rotate arrows 
bool graphicArrowRotate(bool bInit)
{
    static uint16_t idx; // Transformation index

    // Arrow bitmap
    uint8_t arrow[COL_SIZE] = {0b00000000, 0b00011000, 0b00111100, 0b01111110,
                               0b00011000, 0b00011000, 0b00011000, 0b00000000};

    // Shift directions
    Led_Matrix::transformType_t t[] = {
        Led_Matrix::TRC, Led_Matrix::TRC, Led_Matrix::TSR, Led_Matrix::TSR, Led_Matrix::TSR, Led_Matrix::TSR,
        Led_Matrix::TSR, Led_Matrix::TSR, Led_Matrix::TSR, Led_Matrix::TSR, Led_Matrix::TRC, Led_Matrix::TRC,
        Led_Matrix::TSL, Led_Matrix::TSL, Led_Matrix::TSL, Led_Matrix::TSL, Led_Matrix::TSL, Led_Matrix::TSL,
        Led_Matrix::TSL, Led_Matrix::TSL, Led_Matrix::TRC,
    };

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- ArrowRotate init");
        resetMatrix();
        bInit = false;
        idx = 0;

        // Use the arrow bitmap
        mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
        for (uint8_t j = 0; j < mx.getDeviceCount(); j++)
            mx.setBuffer(((j + 1) * COL_SIZE) - 1, COL_SIZE, arrow);
        mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < ARROWR_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Shift the arrows in all shift directions
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::ON);
    mx.transform(t[idx++]);
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::OFF);

    // Check if we are completed and set initialize for next time around
    if (idx == (sizeof(t) / sizeof(t[0])))
        bInit = true;

    return (bInit);
}

// Animate sinewave
bool graphicSinewave(bool bInit)
{
    static uint8_t curWave = 0;
    static uint8_t idx;

#define SW_DATA_WIDTH 11 // Valid data count followed by up to 10 data points
    const uint8_t waves[][SW_DATA_WIDTH] = {
        {9, 8, 6, 1, 6, 24, 96, 128, 96, 16, 0},
        {6, 12, 2, 12, 48, 64, 48, 0, 0, 0, 0},
        {10, 12, 2, 1, 2, 12, 48, 64, 128, 64, 48},

    };
    const uint8_t WAVE_COUNT = sizeof(waves) / (SW_DATA_WIDTH * sizeof(uint8_t));

    // Are we initializing?
    if (bInit)
    {
        PRINTS("\n--- Sinewave init");
        resetMatrix();
        bInit = false;
        idx = 1;
    }

    // Is it time to animate?
    if (millis() - prevTimeAnim < SINE_DELAY)
        return (bInit);
    prevTimeAnim = millis(); // Starting point for next time

    // Start animation
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::ON);
    mx.transform(Led_Matrix::TSL);
    mx.setColumn(0, waves[curWave][idx++]);
    if (idx > waves[curWave][0])
    {
        curWave = random(WAVE_COUNT);
        idx = 1;
    }
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::OFF);

    return (bInit);
}

// ========== Control routines ===========
void resetMatrix(void)
{
    mx.control(Led_Matrix::INTENSITY, MAX_INTENSITY / 2);
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
    mx.clear();
    prevTimeAnim = 0;
}

// Schedule the animations, switching to the next one when the
// the mode switch is pressed.
void runMatrixAnimation(void)
{
    static uint8_t state = 0;
    static uint8_t mesg = 0;
    static boolean bRestart = true;
    static boolean bInMessages = false;
    boolean changeState = false;

#if RUN_DEMO
    // Check if one second has passed and then count down the demo timer. Once this
    // gets to zero, change the state.
    if (millis() - prevTimeDemo >= 1000)
    {
        prevTimeDemo = millis();
        if (--timeDemo == 0)
        {
            timeDemo = DEMO_DELAY;
            changeState = true;
        }
    }
#else
    // Check if the switch is pressed and handle that first
    changeState = (ks.read() == MD_UISwitch::KEY_PRESS);
#endif
    if (changeState)
    {
        if (bInMessages) // The message display state
        {
            mesg++;
            if (mesg >= sizeof(msgTab) / sizeof(msgTab[0]))
            {
                mesg = 0;
                bInMessages = false;
                state++;
            }
        }
        else
            state++;

        bRestart = true;
    };

    // Now do whatever we do in the current state
    switch (state)
    {
    case 0:
        bInMessages = true;
        bRestart = scrollText(bRestart, msgTab[mesg]);
        break;
    case 1:
        bRestart = graphicMidline1(bRestart);
        break;
    case 2:
        bRestart = graphicMidline2(bRestart);
        break;
    case 3:
        bRestart = graphicScanner(bRestart);
        break;
    case 4:
        bRestart = graphicRandom(bRestart);
        break;
    case 5:
        bRestart = graphicFade(bRestart);
        break;
    case 6:
        bRestart = graphicSpectrum1(bRestart);
        break;
    case 7:
        bRestart = graphicHeartbeat(bRestart);
        break;
    case 8:
        bRestart = graphicHearts(bRestart);
        break;
    case 9:
        bRestart = graphicEyes(bRestart);
        break;
    case 10:
        bRestart = graphicBounceBall(bRestart);
        break;
    case 11:
        bRestart = graphicArrowScroll(bRestart);
        break;
    case 12:
        bRestart = graphicScroller(bRestart);
        break;
    case 13:
        bRestart = graphicWiper(bRestart);
        break;
    case 14:
        bRestart = graphicInvader(bRestart);
        break;
    case 15:
        bRestart = graphicPacman(bRestart);
        break;
    case 16:
        bRestart = graphicArrowRotate(bRestart);
        break;
    case 17:
        bRestart = graphicSpectrum2(bRestart);
        break;
    case 18:
        bRestart = graphicSinewave(bRestart);
        break;

    default:
        state = 0;
    }
}

void setup()
{
    // Init the matrix
    mx.begin();

    // Time of the previous animation
    prevTimeAnim = millis();
#if RUN_DEMO
    prevTimeDemo = millis();
#else
    // Init button
    ks.begin();
#endif

    // Init serial communication if it's needed
#if DEBUG
    Serial.begin(115200);
#endif
    PRINTS("\n[Led_Matrix DaftPunk]");
}

void loop()
{
    // Start all animations
    runMatrixAnimation();

    // Other code to run the helmet goes here
}
