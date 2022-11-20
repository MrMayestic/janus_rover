// const int switchPin = 2; // switch to turn on and off mouse control
#include <MemoryUsage.h>
#include <math.h>
#include <string.h>
#include <SPI.h>

#define joyX A0
#define joyY A1

int ENA;
int ENB;

int lastX;
int lastY;

int nowMode;

bool toggle = false;

int count0;

//-----------SEND DATA--------------------------------

void writeData(uint8_t *data, size_t len)
{
    uint8_t i = 0;
    digitalWrite(SS, LOW);

    SPI.transfer(0x02);
    SPI.transfer(0x00);

    while (len-- && i < 32)
    {
        SPI.transfer(data[i++]);
    }

    while (i++ < 32)
    {
        SPI.transfer(0);
    }

    digitalWrite(SS, HIGH);
}

void send(const char *message)
{
    Serial.print("Master: ");
    Serial.println(message);

    writeData((uint8_t *)message, strlen(message));

    delay(5);
}

//----------------------------------------------

void setup()
{
    Serial.begin(19200);
    SPI.begin();
}

//-----USTALENIE PROCENTU

float getPrec(float val)
{
    val = abs(512.5 - val);
    float wynik = (val / 512.5);

    return wynik;
}

float getPrec00(float val)
{
    val = abs(526 - val);
    float wynik = (val / 526);

    return wynik;
}

//-----KONWERSJA NA ZAKRES OD 0-255

int sterringValues(int x, int y)
{

    // Serial.print(x);
    // Serial.print("\t");
    // Serial.println(y);
    if (y < 525 && x > 527)
    {
        nowMode = 4;
        // LP
        ENA = (255 - (127 * getPrec(y))) * getPrec(x);
        ENB = 127 + (127 * getPrec(x));
    }
    else if (y > 528 && x > 527)
    {
        nowMode = 4;
        // PP
        ENA = 255 * getPrec(x);
        ENB = (255 - (127 * getPrec(y))) * getPrec(x);
    }
    else if (y < 525 && x < 525)
    {
        nowMode = 1;
        // LT
        ENA = (255 - (127 * getPrec(y))) * getPrec(x);
        ENB = 255 * getPrec(x);
    }
    else if (y > 528 && x < 524)
    {
        nowMode = 1;
        // PT
        ENA = 255 * getPrec(x); // 127 - (127 * getPrec(x));
        ENB = (255 - (127 * getPrec(y))) * getPrec(x);
    }
    else if (y < 525)
    {
        nowMode = 3;
        // L
        ENA = 0;
        ENB = 0 + (255 * getPrec(y));
    }
    else if (y > 528)
    {
        nowMode = 3;
        // P
        ENA = 0 + (255 * getPrec(y));
        ENB = 0;
    }
    else if (x > 527)
    {
        nowMode = 3;
        ENA = 0 + (255 * getPrec(x));
        ENB = 0 + (255 * getPrec(x));
    }

    else if (x < 525)
    {
        nowMode = 2;
        ENA = 0 + (255 * getPrec(x));
        ENB = 0 + (255 * getPrec(x));
    }
    else
    {
        ENA = 0;
        ENB = 0;
    }
    // if ((abs(526 - x) < 528 && abs(526 - x) > 496) && (abs(526 - y) == 0 || abs(526 - y) == 1))
    // {
    //     ENA *= 2;
    //     ENB *= 2;
    // }
    if (ENA > 0)
    {
        ENA = constrain(ENA, 15, 255);
    }

    if (ENB > 0)
    {
        ENB = constrain(ENB, 15, 255);
    }
    // Serial.print(ENA);
    // Serial.print("\t");
    // Serial.println(ENB);
}

// LOOP

int xValue;
int yValue;
void loop()
{
    // FREERAM_PRINT;
    delay(10);

    xValue = analogRead(joyX);

    delay(10);

    yValue = analogRead(joyY);

    sterringValues(xValue, yValue);

    delay(10);
    // print the values with to plot or view
    // Serial.print(abs(526-xValue));
    // Serial.print("\t");
    // Serial.println(abs(526-yValue));
    if ((ENA == 0) && (ENB == 0))
    {
        if (count0 == 3)
        {
            // Serial.println("jest10");
            String sendData = "x0y0t4";
            send(sendData.c_str());
            count0 = 1;
        }
        else if (count0 == 0)
        {
            // Serial.println("jest0");
            String sendData = "x0y0t4";
            send(sendData.c_str());
            count0++;
        }
        else
        {
            count0++;
        }
    }
    if (ENA != 0 || ENB != 0)
    {
        count0 = 0;
        // lastX = ENA;
        // lastY = ENB;
        String check = String(ENA);

        int len = check.length();

        String sendData = "x" + String(ENA) + "y" + String(ENB) + "t" + String(nowMode);
        send(sendData.c_str());
        // Serial.println(sendData);
    }
}
