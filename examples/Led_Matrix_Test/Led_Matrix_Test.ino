/**
 **************************************************
 *
 * @file        Led_Matrix_Test.ino
 *
 * @brief       Program to exercise the Led_Matrix library
 *
 *              Uses most of the functions in the library
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

// Turn on debug statements to the serial output
#define DEBUG 1

// Define debug functions
#if DEBUG
#define PRINT(s, x)                                                                                                    \
    {                                                                                                                  \
        Serial.print(F(s));                                                                                            \
        Serial.print(x);                                                                                               \
    }
#define PRINTS(x) Serial.print(F(x))
#define PRINTD(x) Serial.println(x, DEC)

#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTD(x)
#endif

// Define the number of devices we have in the chain and the hardware interface
#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define MAX_DEVICES   3

// Define load pin
#define LOAD_PIN 10

// SPI hardware interface
Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, LOAD_PIN, MAX_DEVICES);

// We always wait a bit between updates of the display
#define DELAYTIME 100 // In milliseconds

// Function for scrolling text on the matrix
void scrollText(const char *p)
{
    uint8_t charWidth;
    uint8_t cBuf[8]; // This should be ok for all built-in fonts

    PRINTS("\nScrolling text");
    mx.clear();

    // Go through the string until the end
    while (*p != '\0')
    {
        charWidth = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);

        for (uint8_t i = 0; i <= charWidth; i++) // Allow space between characters
        {
            mx.transform(Led_Matrix::TSL);
            if (i < charWidth)
                mx.setColumn(0, cBuf[i]);
            delay(DELAYTIME);
        }
    }
}

// Demonstrates the use of setPoint and show where the zero point is in the display
void zeroPointSet()
{
    PRINTS("\nZero point highlight");
    mx.clear();

    // Draw 0
    if (MAX_DEVICES > 1)
        mx.setChar((2 * COL_SIZE) - 1, '0');

    // Animate arrow
    for (uint8_t i = 0; i < ROW_SIZE; i++)
    {
        mx.setPoint(i, i, true);
        mx.setPoint(0, i, true);
        mx.setPoint(i, 0, true);
        delay(DELAYTIME);
    }

    delay(DELAYTIME * 3);
}

// Demonstrates the use of setRow()
void rows()
{
    PRINTS("\nRows 0->7");
    mx.clear();

    // Set each row lights up, and turn it off after the delay
    for (uint8_t row = 0; row < ROW_SIZE; row++)
    {
        mx.setRow(row, 0xff);
        delay(2 * DELAYTIME);
        mx.setRow(row, 0x00);
    }
}

// Nested rectangles spanning the entire display
void checkboard()
{
    uint8_t chkCols[][2] = {{0x55, 0xaa}, {0x33, 0xcc}, {0x0f, 0xf0}, {0xff, 0x00}};

    PRINTS("\nCheckboard");
    mx.clear();

    // Go through each pattern
    for (uint8_t pattern = 0; pattern < sizeof(chkCols) / sizeof(chkCols[0]); pattern++)
    {
        uint8_t col = 0;
        uint8_t idx = 0;
        uint8_t rep = 1 << pattern;

        // Go through each column on matrices
        while (col < mx.getColumnCount())
        {
            // Actually draw a rectangles
            for (uint8_t r = 0; r < rep; r++)
                mx.setColumn(col++, chkCols[pattern][idx]); // Use odd/even column masks
            idx++;
            if (idx > 1)
                idx = 0;
        }

        delay(10 * DELAYTIME);
    }
}

// Demonstrates the use of setColumn()
void columns()
{
    PRINTS("\nCols 0->max");
    mx.clear();

    // Set each column lights up, and turn it off after the delay
    for (uint8_t col = 0; col < mx.getColumnCount(); col++)
    {
        mx.setColumn(col, 0xff);
        delay(DELAYTIME / MAX_DEVICES);
        mx.setColumn(col, 0x00);
    }
}

// Combination of setRow() and setColumn() with user controlled display updates to ensure concurrent changes
void cross()
{
    PRINTS("\nMoving cross");
    mx.clear();
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    // Diagonally down the display R to L
    for (uint8_t i = 0; i < ROW_SIZE; i++)
    {
        for (uint8_t j = 0; j < MAX_DEVICES; j++)
        {
            mx.setColumn(j, i, 0xff);
            mx.setRow(j, i, 0xff);
        }
        mx.update();
        delay(DELAYTIME);
        for (uint8_t j = 0; j < MAX_DEVICES; j++)
        {
            mx.setColumn(j, i, 0x00);
            mx.setRow(j, i, 0x00);
        }
    }

    // Moving up the display on the R
    for (int8_t i = ROW_SIZE - 1; i >= 0; i--)
    {
        for (uint8_t j = 0; j < MAX_DEVICES; j++)
        {
            mx.setColumn(j, i, 0xff);
            mx.setRow(j, ROW_SIZE - 1, 0xff);
        }
        mx.update();
        delay(DELAYTIME);
        for (uint8_t j = 0; j < MAX_DEVICES; j++)
        {
            mx.setColumn(j, i, 0x00);
            mx.setRow(j, ROW_SIZE - 1, 0x00);
        }
    }

    // Diagonally up the display L to R
    for (uint8_t i = 0; i < ROW_SIZE; i++)
    {
        for (uint8_t j = 0; j < MAX_DEVICES; j++)
        {
            mx.setColumn(j, i, 0xff);
            mx.setRow(j, ROW_SIZE - 1 - i, 0xff);
        }
        mx.update();
        delay(DELAYTIME);
        for (uint8_t j = 0; j < MAX_DEVICES; j++)
        {
            mx.setColumn(j, i, 0x00);
            mx.setRow(j, ROW_SIZE - 1 - i, 0x00);
        }
    }
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
}

// Demonstrate the use of buffer based repeated patterns across all devices
void bullseye()
{
    PRINTS("\nBullseye");
    mx.clear();
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    // Repeat this 3 times
    for (uint8_t n = 0; n < 3; n++)
    {
        byte b = 0xff;
        int i = 0;

        // Clearing bits until each becomes 0
        while (b != 0x00)
        {
            // Go through each device
            for (uint8_t j = 0; j < MAX_DEVICES + 1; j++)
            {
                mx.setRow(j, i, b);
                mx.setColumn(j, i, b);
                mx.setRow(j, ROW_SIZE - 1 - i, b);
                mx.setColumn(j, COL_SIZE - 1 - i, b);
            }
            mx.update();
            delay(3 * DELAYTIME);

            // Go through each device
            for (uint8_t j = 0; j < MAX_DEVICES + 1; j++)
            {
                mx.setRow(j, i, 0);
                mx.setColumn(j, i, 0);
                mx.setRow(j, ROW_SIZE - 1 - i, 0);
                mx.setColumn(j, COL_SIZE - 1 - i, 0);
            }

            bitClear(b, i);
            bitClear(b, 7 - i);
            i++;
        }

        // Setting bits until each becomes 1
        while (b != 0xff)
        {
            // Go through each device
            for (uint8_t j = 0; j < MAX_DEVICES + 1; j++)
            {
                mx.setRow(j, i, b);
                mx.setColumn(j, i, b);
                mx.setRow(j, ROW_SIZE - 1 - i, b);
                mx.setColumn(j, COL_SIZE - 1 - i, b);
            }
            mx.update();
            delay(3 * DELAYTIME);

            // Go through each device
            for (uint8_t j = 0; j < MAX_DEVICES + 1; j++)
            {
                mx.setRow(j, i, 0);
                mx.setColumn(j, i, 0);
                mx.setRow(j, ROW_SIZE - 1 - i, 0);
                mx.setColumn(j, COL_SIZE - 1 - i, 0);
            }

            i--;
            bitSet(b, i);
            bitSet(b, 7 - i);
        }
    }

    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
}

// Demonstrates animation of a diagonal stripe moving across the display with points plotted outside the display region
// ignored
void stripe()
{
    const uint16_t maxCol = MAX_DEVICES * ROW_SIZE;
    const uint8_t stripeWidth = 10;

    PRINTS("\nEach individually by row then col");
    mx.clear();

    // Go through each column
    for (uint16_t col = 0; col < maxCol + ROW_SIZE + stripeWidth; col++)
    {
        // Go through each row
        for (uint8_t row = 0; row < ROW_SIZE; row++)
        {
            // Draw a diagonal stripe
            mx.setPoint(row, col - row, true);
            mx.setPoint(row, col - row - stripeWidth, false);
        }
        delay(DELAYTIME);
    }
}

// setPoint() used to draw a spiral across the whole display
void spiral()
{
    PRINTS("\nSpiral in");
    int rmin = 0, rmax = ROW_SIZE - 1;
    int cmin = 0, cmax = (COL_SIZE * MAX_DEVICES) - 1;

    mx.clear();
    while ((rmax > rmin) && (cmax > cmin))
    {
        // Do row
        for (int i = cmin; i <= cmax; i++)
        {
            mx.setPoint(rmin, i, true);
            delay(DELAYTIME / MAX_DEVICES);
        }
        rmin++;

        // Do column
        for (uint8_t i = rmin; i <= rmax; i++)
        {
            mx.setPoint(i, cmax, true);
            delay(DELAYTIME / MAX_DEVICES);
        }
        cmax--;

        // Do row
        for (int i = cmax; i >= cmin; i--)
        {
            mx.setPoint(rmax, i, true);
            delay(DELAYTIME / MAX_DEVICES);
        }
        rmax--;

        // Do column
        for (uint8_t i = rmax; i >= rmin; i--)
        {
            mx.setPoint(i, cmin, true);
            delay(DELAYTIME / MAX_DEVICES);
        }
        cmin++;
    }
}

// Animation of a bouncing ball
void bounce()
{
    const int minC = 0;
    const int maxC = mx.getColumnCount() - 1;
    const int minR = 0;
    const int maxR = ROW_SIZE - 1;

    int nCounter = 0;

    int r = 0, c = 2;
    int8_t dR = 1, dC = 1; // Delta row and column

    PRINTS("\nBouncing ball");
    mx.clear();

    // Repeat moving the ball 200 times
    while (nCounter++ < 200)
    {
        mx.setPoint(r, c, false);
        r += dR;
        c += dC;
        mx.setPoint(r, c, true);
        delay(DELAYTIME / 2);

        if ((r == minR) || (r == maxR))
            dR = -dR;
        if ((c == minC) || (c == maxC))
            dC = -dC;
    }
}

// Demonstrates the control of display intensity (brightness) across the full range
void intensity()
{
    uint8_t row;

    PRINTS("\nVary intensity ");

    mx.clear();

    // Grow and get brighter
    row = 0;
    for (int8_t i = 0; i <= MAX_INTENSITY; i++)
    {
        mx.control(Led_Matrix::INTENSITY, i);
        if (i % 2 == 0)
            mx.setRow(row++, 0xff);
        delay(DELAYTIME * 3);
    }

    mx.control(Led_Matrix::INTENSITY, 8);
}

// Uses the test function of the MAX72xx to blink the display on and off
void blinking()
{
    int nDelay = 1000;

    PRINTS("\nBlinking");
    mx.clear();

    // Blink until time runs out
    while (nDelay > 0)
    {
        mx.control(Led_Matrix::TEST, Led_Matrix::ON);
        delay(nDelay);
        mx.control(Led_Matrix::TEST, Led_Matrix::OFF);
        delay(nDelay);

        nDelay -= DELAYTIME;
    }
}

// Uses scan limit function to restrict the number of rows displayed
void scanLimit(void)
{
    PRINTS("\nScan Limit");
    mx.clear();

    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    for (uint8_t row = 0; row < ROW_SIZE; row++)
        mx.setRow(row, 0xff);
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);

    for (int8_t s = MAX_SCANLIMIT; s >= 0; s--)
    {
        mx.control(Led_Matrix::SCANLIMIT, s);
        delay(DELAYTIME * 5);
    }
    mx.control(Led_Matrix::SCANLIMIT, MAX_SCANLIMIT);
}

// Demonstrates the use of transform() to move bitmaps on the display
// In this case a user defined bitmap is created and animated
void transformation1()
{
    uint8_t arrow[COL_SIZE] = {0b00001000, 0b00011100, 0b00111110, 0b01111111,
                               0b00011100, 0b00011100, 0b00111110, 0b00000000};

    Led_Matrix::transformType_t t[] = {
        Led_Matrix::TSL,  Led_Matrix::TSL, Led_Matrix::TSL, Led_Matrix::TSL,  Led_Matrix::TSL,  Led_Matrix::TSL,
        Led_Matrix::TSL,  Led_Matrix::TSL, Led_Matrix::TSL, Led_Matrix::TSL,  Led_Matrix::TSL,  Led_Matrix::TSL,
        Led_Matrix::TSL,  Led_Matrix::TSL, Led_Matrix::TSL, Led_Matrix::TSL,  Led_Matrix::TFLR, Led_Matrix::TSR,
        Led_Matrix::TSR,  Led_Matrix::TSR, Led_Matrix::TSR, Led_Matrix::TSR,  Led_Matrix::TSR,  Led_Matrix::TSR,
        Led_Matrix::TSR,  Led_Matrix::TSR, Led_Matrix::TSR, Led_Matrix::TSR,  Led_Matrix::TSR,  Led_Matrix::TSR,
        Led_Matrix::TSR,  Led_Matrix::TSR, Led_Matrix::TSR, Led_Matrix::TRC,  Led_Matrix::TSD,  Led_Matrix::TSD,
        Led_Matrix::TSD,  Led_Matrix::TSD, Led_Matrix::TSD, Led_Matrix::TSD,  Led_Matrix::TSD,  Led_Matrix::TSD,
        Led_Matrix::TFUD, Led_Matrix::TSU, Led_Matrix::TSU, Led_Matrix::TSU,  Led_Matrix::TSU,  Led_Matrix::TSU,
        Led_Matrix::TSU,  Led_Matrix::TSU, Led_Matrix::TSU, Led_Matrix::TINV, Led_Matrix::TRC,  Led_Matrix::TRC,
        Led_Matrix::TRC,  Led_Matrix::TRC, Led_Matrix::TINV};

    PRINTS("\nTransformation1");
    mx.clear();

    // Use the arrow bitmap
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    for (uint8_t j = 0; j < mx.getDeviceCount(); j++)
        mx.setBuffer(((j + 1) * COL_SIZE) - 1, COL_SIZE, arrow);
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
    delay(DELAYTIME);

    // Run through the transformations
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::ON);
    for (uint8_t i = 0; i < (sizeof(t) / sizeof(t[0])); i++)
    {
        mx.transform(t[i]);
        delay(DELAYTIME * 4);
    }
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::OFF);
}

// Demonstrates the use of transform() to move bitmaps on the display
// In this case font characters are loaded into the display for animation
void transformation2()
{
    Led_Matrix::transformType_t t[] = {
        Led_Matrix::TINV, Led_Matrix::TRC, Led_Matrix::TRC, Led_Matrix::TRC,  Led_Matrix::TRC,  Led_Matrix::TINV,
        Led_Matrix::TSL,  Led_Matrix::TSL, Led_Matrix::TSL, Led_Matrix::TSL,  Led_Matrix::TSL,  Led_Matrix::TSR,
        Led_Matrix::TSR,  Led_Matrix::TSR, Led_Matrix::TSR, Led_Matrix::TSR,  Led_Matrix::TSR,  Led_Matrix::TSR,
        Led_Matrix::TSR,  Led_Matrix::TSR, Led_Matrix::TSR, Led_Matrix::TSR,  Led_Matrix::TSR,  Led_Matrix::TSR,
        Led_Matrix::TSL,  Led_Matrix::TSL, Led_Matrix::TSL, Led_Matrix::TSL,  Led_Matrix::TSL,  Led_Matrix::TSL,
        Led_Matrix::TSL,  Led_Matrix::TSL, Led_Matrix::TSR, Led_Matrix::TSR,  Led_Matrix::TSR,  Led_Matrix::TSD,
        Led_Matrix::TSU,  Led_Matrix::TSD, Led_Matrix::TSU, Led_Matrix::TFLR, Led_Matrix::TFLR, Led_Matrix::TFUD,
        Led_Matrix::TFUD};

    PRINTS("\nTransformation2");
    mx.clear();
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::OFF);

    // Draw something that will show changes
    for (uint8_t j = 0; j < mx.getDeviceCount(); j++)
    {
        mx.setChar(((j + 1) * COL_SIZE) - 1, '0' + j);
    }
    delay(DELAYTIME * 5);

    // Run thru transformations
    for (uint8_t i = 0; i < (sizeof(t) / sizeof(t[0])); i++)
    {
        mx.transform(t[i]);
        delay(DELAYTIME * 3);
    }
}

// Display text and animate scrolling using auto wraparound of the buffer
void wrapText()
{
    PRINTS("\nwrapText");
    mx.clear();
    mx.wraparound(Led_Matrix::ON);

    // Draw something that will show changes
    for (uint16_t j = 0; j < mx.getDeviceCount(); j++)
    {
        mx.setChar(((j + 1) * COL_SIZE) - 1, (j & 1 ? 'M' : 'W'));
    }
    delay(DELAYTIME * 5);

    // Run thru transformations
    for (uint16_t i = 0; i < 3 * COL_SIZE * MAX_DEVICES; i++)
    {
        mx.transform(Led_Matrix::TSL);
        delay(DELAYTIME / 2);
    }
    for (uint16_t i = 0; i < 3 * COL_SIZE * MAX_DEVICES; i++)
    {
        mx.transform(Led_Matrix::TSR);
        delay(DELAYTIME / 2);
    }
    for (uint8_t i = 0; i < ROW_SIZE; i++)
    {
        mx.transform(Led_Matrix::TSU);
        delay(DELAYTIME * 2);
    }
    for (uint8_t i = 0; i < ROW_SIZE; i++)
    {
        mx.transform(Led_Matrix::TSD);
        delay(DELAYTIME * 2);
    }

    // If this options is enabled, the edge is wrapped around to the opposite side
    mx.wraparound(Led_Matrix::OFF);
}

// Run through display of the the entire font characters set
void showCharset(void)
{
    mx.clear();
    mx.update(Led_Matrix::OFF);

    // Go through each char
    for (uint16_t i = 0; i < 256; i++)
    {
        mx.clear(0);
        mx.setChar(COL_SIZE - 1, i);

        // If there are more than 3 matrices, use hex format
        if (MAX_DEVICES >= 3)
        {
            char hex[3];

            sprintf(hex, "%02X", i);

            mx.clear(1);
            mx.setChar((2 * COL_SIZE) - 1, hex[1]);
            mx.clear(2);
            mx.setChar((3 * COL_SIZE) - 1, hex[0]);
        }

        mx.update();
        delay(DELAYTIME * 2);
    }
    mx.update(Led_Matrix::ON);
}

void setup()
{
    // Init matrix
    mx.begin();

#if DEBUG
    // Init serial communication if it's used
    Serial.begin(115200);
#endif

    // Print a message on the matrix and Serial Monitor
    PRINTS("\n[Led_Matrix Test & Demo]");
    scrollText("Led_Matrix Test  ");
}

void loop()
{
    // Graphics tests demo
    scrollText("Graphics");
    zeroPointSet();
    rows();
    columns();
    cross();
    stripe();
    checkboard();
    bullseye();
    bounce();
    spiral();

    // Controlling matrix demo
    scrollText("Control");
    intensity();
    scanLimit();
    blinking();

    // Transformation demo
    scrollText("Transform");
    transformation1();
    transformation2();

    // Charset demo
    scrollText("Charset");
    wrapText();
    showCharset();
}
