// Program to exercise the Led_Matrix library
//
// Test the library transformation functions with range subsets
//
// Modified by Soldered for use on https://solde.red/333062

#include "Led-Matrix-SOLDERED.h"
#include <SPI.h>

// We always wait a bit between updates of the display
#define DELAYTIME 300 // in milliseconds

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted

#define MAX_DEVICES 8 // 2, 4, 6, or 8 work best - see Z array

#define CS_PIN 10 // or SS

// SPI hardware interface
Led_Matrix mx = Led_Matrix(CS_PIN, MAX_DEVICES);
// Arbitrary pins
// Led_Matrix mx = Led_Matrix(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Global variables
uint32_t lastTime = 0;

typedef struct
{
    uint8_t startDev; // start of zone
    uint8_t endDev;   // end of zone
    uint8_t ch;       // character to show
    Led_Matrix::transformType_t tt;
} zoneDef_t;

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

void runTransformation(void)
{
    mx.control(Led_Matrix::UPDATE, Led_Matrix::OFF);

    for (uint8_t i = 0; i < ARRAY_SIZE(Z); i++)
        mx.transform(Z[i].startDev, Z[i].endDev, Z[i].tt);

    mx.control(Led_Matrix::UPDATE, Led_Matrix::ON);
}

void setup()
{
    Serial.begin(57600);
    Serial.println("[Zone Transform Test]");

    mx.begin();
    mx.control(Led_Matrix::WRAPAROUND, Led_Matrix::ON);

    // set up the display characters
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
    if (millis() - lastTime >= DELAYTIME)
    {
        runTransformation();
        lastTime = millis();
    }
}

// Dasduino Connect (ESP8266)      Dasduino_Boards:esp8266:connect
// Dasduino ConnectPlus (ESP32)    Dasduino_Boards:esp32:connectplus
// Dasduino ConnectPlus LoRa ESP32 Dasduino_Boards:esp32:lora32
// Dasduino Core (Atmega328P)      Dasduino_Boards:avr:core
// Dasduino Core LoRa (Atmega328P) Dasduino_Boards:avr:LoRa
// Dasduino Lite                   Dasduino_Boards:megaavr:lite