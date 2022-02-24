/**
 **************************************************
 *
 * @file        Led_Matrix_Test.ino
 * @brief       Program to exercise the Led_Matrix library
 * 
 *              Uses most of the functions in the library
 *
 * @authors     Goran Juric for Soldered.com
 * 
 *              Modified by Soldered for use on https://solde.red/333062, https://solde.red/333148,
 *              https://solde.red/333149, https://solde.red/333150, https://solde.red/333151 and
 *              https://solde.red/333152
 ***************************************************/

#include "Led-Matrix-SOLDERED.h"

// Turn on debug statements to the serial output
#define DEBUG 1

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
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE Led_Matrix::PAROLA_HW
#define MAX_DEVICES 11

#define CS_PIN 10 // or SS

// SPI hardware interface
Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// We always wait a bit between updates of the display
#define DELAYTIME 100 // in milliseconds

void scrollText(const char *p)
{
    uint8_t charWidth;
    uint8_t cBuf[8]; // this should be ok for all built-in fonts

    PRINTS("\nScrolling text");
    mx.clear();

    while (*p != '\0')
    {
        charWidth = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);

        for (uint8_t i = 0; i <= charWidth; i++) // allow space between characters
        {
            mx.transform(Led_Matrix::TSL);
            if (i < charWidth)
                mx.setColumn(0, cBuf[i]);
            delay(DELAYTIME);
        }
    }
}

void zeroPointSet()
// Demonstrates the use of setPoint and
// show where the zero point is in the display
{
    PRINTS("\nZero point highlight");
    mx.clear();

    if (MAX_DEVICES > 1)
        mx.setChar((2 * COL_SIZE) - 1, '0');

    for (uint8_t i = 0; i < ROW_SIZE; i++)
    {
        mx.setPoint(i, i, true);
        mx.setPoint(0, i, true);
        mx.setPoint(i, 0, true);
        delay(DELAYTIME);
    }

    delay(DELAYTIME * 3);
}

void rows()
// Demonstrates the use of setRow()
{
    PRINTS("\nRows 0->7");
    mx.clear();

    for (uint8_t row = 0; row < ROW_SIZE; row++)
    {
        mx.setRow(row, 0xff);
        delay(2 * DELAYTIME);
        mx.setRow(row, 0x00);
    }
}

void checkboard()
// nested rectangles spanning the entire display
{
    uint8_t chkCols[][2] = {{0x55, 0xaa}, {0x33, 0xcc}, {0x0f, 0xf0}, {0xff, 0x00}};

    PRINTS("\nCheckboard");
    mx.clear();

    for (uint8_t pattern = 0; pattern < sizeof(chkCols) / sizeof(chkCols[0]); pattern++)
    {
        uint8_t col = 0;
        uint8_t idx = 0;
        uint8_t rep = 1 << pattern;

        while (col < mx.getColumnCount())
        {
            for (uint8_t r = 0; r < rep; r++)
                mx.setColumn(col++, chkCols[pattern][idx]); // use odd/even column masks
            idx++;
            if (idx > 1)
                idx = 0;
        }

        delay(10 * DELAYTIME);
    }
}

void columns()
// Demonstrates the use of setColumn()
{
    PRINTS("\nCols 0->max");
    mx.clear();

    for (uint8_t col = 0; col < mx.getColumnCount(); col++)
    {
        mx.setColumn(col, 0xff);
        delay(DELAYTIME / MAX_DEVICES);
        mx.setColumn(col, 0x00);
    }
}

void cross()
// Combination of setRow() and setColumn() with user controlled
// display updates to ensure concurrent changes.
{
    PRINTS("\nMoving cross");
    mx.clear();
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    // diagonally down the display R to L
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

    // moving up the display on the R
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

    // diagonally up the display L to R
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

void bullseye()
// Demonstrate the use of buffer based repeated patterns
// across all devices.
{
    PRINTS("\nBullseye");
    mx.clear();
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    for (uint8_t n = 0; n < 3; n++)
    {
        byte b = 0xff;
        int i = 0;

        while (b != 0x00)
        {
            for (uint8_t j = 0; j < MAX_DEVICES + 1; j++)
            {
                mx.setRow(j, i, b);
                mx.setColumn(j, i, b);
                mx.setRow(j, ROW_SIZE - 1 - i, b);
                mx.setColumn(j, COL_SIZE - 1 - i, b);
            }
            mx.update();
            delay(3 * DELAYTIME);
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

        while (b != 0xff)
        {
            for (uint8_t j = 0; j < MAX_DEVICES + 1; j++)
            {
                mx.setRow(j, i, b);
                mx.setColumn(j, i, b);
                mx.setRow(j, ROW_SIZE - 1 - i, b);
                mx.setColumn(j, COL_SIZE - 1 - i, b);
            }
            mx.update();
            delay(3 * DELAYTIME);
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

void stripe()
// Demonstrates animation of a diagonal stripe moving across the display
// with points plotted outside the display region ignored.
{
    const uint16_t maxCol = MAX_DEVICES * ROW_SIZE;
    const uint8_t stripeWidth = 10;

    PRINTS("\nEach individually by row then col");
    mx.clear();

    for (uint16_t col = 0; col < maxCol + ROW_SIZE + stripeWidth; col++)
    {
        for (uint8_t row = 0; row < ROW_SIZE; row++)
        {
            mx.setPoint(row, col - row, true);
            mx.setPoint(row, col - row - stripeWidth, false);
        }
        delay(DELAYTIME);
    }
}

void spiral()
// setPoint() used to draw a spiral across the whole display
{
    PRINTS("\nSpiral in");
    int rmin = 0, rmax = ROW_SIZE - 1;
    int cmin = 0, cmax = (COL_SIZE * MAX_DEVICES) - 1;

    mx.clear();
    while ((rmax > rmin) && (cmax > cmin))
    {
        // do row
        for (int i = cmin; i <= cmax; i++)
        {
            mx.setPoint(rmin, i, true);
            delay(DELAYTIME / MAX_DEVICES);
        }
        rmin++;

        // do column
        for (uint8_t i = rmin; i <= rmax; i++)
        {
            mx.setPoint(i, cmax, true);
            delay(DELAYTIME / MAX_DEVICES);
        }
        cmax--;

        // do row
        for (int i = cmax; i >= cmin; i--)
        {
            mx.setPoint(rmax, i, true);
            delay(DELAYTIME / MAX_DEVICES);
        }
        rmax--;

        // do column
        for (uint8_t i = rmax; i >= rmin; i--)
        {
            mx.setPoint(i, cmin, true);
            delay(DELAYTIME / MAX_DEVICES);
        }
        cmin++;
    }
}

void bounce()
// Animation of a bouncing ball
{
    const int minC = 0;
    const int maxC = mx.getColumnCount() - 1;
    const int minR = 0;
    const int maxR = ROW_SIZE - 1;

    int nCounter = 0;

    int r = 0, c = 2;
    int8_t dR = 1, dC = 1; // delta row and column

    PRINTS("\nBouncing ball");
    mx.clear();

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

void intensity()
// Demonstrates the control of display intensity (brightness) across
// the full range.
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

void blinking()
// Uses the test function of the MAX72xx to blink the display on and off.
{
    int nDelay = 1000;

    PRINTS("\nBlinking");
    mx.clear();

    while (nDelay > 0)
    {
        mx.control(Led_Matrix::TEST, Led_Matrix::ON);
        delay(nDelay);
        mx.control(Led_Matrix::TEST, Led_Matrix::OFF);
        delay(nDelay);

        nDelay -= DELAYTIME;
    }
}

void scanLimit(void)
// Uses scan limit function to restrict the number of rows displayed.
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

void transformation1()
// Demonstrates the use of transform() to move bitmaps on the display
// In this case a user defined bitmap is created and animated.
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

    // use the arrow bitmap
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);
    for (uint8_t j = 0; j < mx.getDeviceCount(); j++)
        mx.setBuffer(((j + 1) * COL_SIZE) - 1, COL_SIZE, arrow);
    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
    delay(DELAYTIME);

    // run through the transformations
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::ON);
    for (uint8_t i = 0; i < (sizeof(t) / sizeof(t[0])); i++)
    {
        mx.transform(t[i]);
        delay(DELAYTIME * 4);
    }
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::OFF);
}

void transformation2()
// Demonstrates the use of transform() to move bitmaps on the display
// In this case font characters are loaded into the display for animation.
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

    // draw something that will show changes
    for (uint8_t j = 0; j < mx.getDeviceCount(); j++)
    {
        mx.setChar(((j + 1) * COL_SIZE) - 1, '0' + j);
    }
    delay(DELAYTIME * 5);

    // run thru transformations
    for (uint8_t i = 0; i < (sizeof(t) / sizeof(t[0])); i++)
    {
        mx.transform(t[i]);
        delay(DELAYTIME * 3);
    }
}

void wrapText()
// Display text and animate scrolling using auto wraparound of the buffer
{
    PRINTS("\nwrapText");
    mx.clear();
    mx.wraparound(Led_Matrix::ON);

    // draw something that will show changes
    for (uint16_t j = 0; j < mx.getDeviceCount(); j++)
    {
        mx.setChar(((j + 1) * COL_SIZE) - 1, (j & 1 ? 'M' : 'W'));
    }
    delay(DELAYTIME * 5);

    // run thru transformations
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

    mx.wraparound(Led_Matrix::OFF);
}

void showCharset(void)
// Run through display of the the entire font characters set
{
    mx.clear();
    mx.update(Led_Matrix::OFF);

    for (uint16_t i = 0; i < 256; i++)
    {
        mx.clear(0);
        mx.setChar(COL_SIZE - 1, i);

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
    mx.begin();

#if DEBUG
    Serial.begin(57600);
#endif
    PRINTS("\n[Led_Matrix Test & Demo]");
    //  scrollText("Led_Matrix Test  ");
}

void loop()
{
#if 1
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
#endif

#if 1
    scrollText("Control");
    intensity();
    scanLimit();
    blinking();
#endif

#if 1
    scrollText("Transform");
    transformation1();
    transformation2();
#endif

#if 1
    scrollText("Charset");
    wrapText();
    showCharset();
#endif
}
