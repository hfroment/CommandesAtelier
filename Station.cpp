#include "Board.h"
#include <LiquidCrystal_I2C.h>
#include <core/MySensorsCore.h>

#include "Station.h"

//const String weatherForecast[] = { "stable", "sunny", "cloudy", "unstable", "thunderstorm", "unknown" };

static const uint8_t iconTemperature[8] PROGMEM = {0x4, 0xa, 0xa, 0xe, 0xe, 0x1f, 0x1f, 0xe}; // 8
static const uint8_t iconHumidity[8] PROGMEM = {0x0, 0x4, 0xa, 0x11, 0x11, 0xe, 0x0, 0x0}; // 8
//static const uint8_t iconMist[8] PROGMEM = {
//    0b00000,
//    0b00100,
//    0b01110,
//    0b11111,
//    0b11111,
//    0b01110,
//    0b00000,
//    0b00000
//};
static const uint8_t iconNoInformation[8] PROGMEM = {0x1f, 0x11, 0xa, 0x4, 0xa, 0x11, 0x1f};
// Double flèche verticale
static const uint8_t iconLink[8] PROGMEM = {
    0b00100,
    0b01110,
    0b10101,
    0b00100,
    0b00100,
    0b10101,
    0b01110,
    0b00100
};

static const uint8_t iconNoClock[8] PROGMEM = {
    0b00000,
    0b01110,
    0b10001,
    0b10101,
    0b10001,
    0b01110,
    0b00000,
    0b00000
};

Station::Station(bool useRtc, bool temperaturePresent, uint8_t lcdAddress, eTypeLcd typeLcd, uint8_t humidityType, bool pressurePresent, bool useLcd) :
    mBoard(*new Board(useRtc)),
    mLcdAddress(lcdAddress),
    mTypeLcd(typeLcd),
    mUseRtc(useRtc),
    mLcdPresent(lcdAddress != 0),
    mPressurePresent(pressurePresent),
    mHumidityType(humidityType),
    mTemperaturePresent(temperaturePresent),
    mLcd(0),
    mUseLcd(useLcd)
{
}

void Station::init()
{
    //Serial.print("DHT11 : ");
    //Serial.println(mHumidityType);
    if (mHumidityType > 0)
    {
        // DHT11 présent
        mBoard.initDht(mHumidityType - 1);
    }
    //Serial.print("DS18B20 : ");
    //Serial.println(mTemperaturePresent);
    if (mTemperaturePresent)
    {
        // DS18B20 présent
        mBoard.initDs18b20();
    }
    //Serial.print("LCD : ");
    //Serial.println(mLcdPresent);
    if (mLcdPresent)
    {
        if (mTypeLcd == LCD_SSD1306)
        {
            //            mBoard.initOled();
        }
        else
        {
            // Ecran LCD présent
            mLcdNbCols = 16;
            mLcdNbLines = 2;
            if (mTypeLcd == LCD_2004)
            {
                mLcdNbCols = 20;
                mLcdNbLines = 4;
            }
            initLcd(mLcdAddress, mLcdNbCols, mLcdNbLines);
            // caractère spéciaux
            if (mLcd != 0)
            {
                mLcd->createCharFromFlash(temperatureChar, iconTemperature);
                mLcd->createCharFromFlash(humidityChar, iconHumidity);
                //mLcd->createCharFromFlash(mystOnChar, iconMist);
                mLcd->createCharFromFlash(noInformationChar, iconNoInformation);
                mLcd->createCharFromFlash(linkChar, iconLink);
                mLcd->createCharFromFlash(noClockChar, iconNoClock);
            }
        }
    }
}

bool Station::getTemperature(float& temperature)
{
    bool retour = false;

    if (mTemperaturePresent)
    {
        retour = mBoard.readDs18b20(temperature);
    }
    else if (mHumidityType)
    {
        uint8_t humidity;
        uint8_t temp;
        retour = mBoard.readTU(temp, humidity);
        temperature = (float)temp;
    }
    return retour;
}

bool Station::getHumidity(uint8_t& humidity)
{
    bool retour = false;

    if (mHumidityType)
    {
        uint8_t temp;
        retour = mBoard.readTU(temp, humidity);
    }
    return retour;
}

bool Station::gerer(bool top, bool pretop)
{
    bool refresh = false;
    if (pretop)
    {
        mBoard.acqStage1();
    }
    if (top)
    {
        // si pretop, on laisse le temps de convertir
        if (pretop)
        {
            sleep(800);
        }
        refresh = mBoard.acqStage2();
        if (refresh)
        {

        }
    }
    return refresh;
}

void Station::log(uint8_t line, String &chaine)
{
    if (mLcd != 0)
    {
        mLcd->setCursor(0, line);
        mLcd->print(chaine.c_str());
    }
}

void Station::clearScreen()
{
    if (mLcd != 0)
    {
        mLcd->clear();
    }
}

void Station::setBacklight(bool on)
{
    if (mLcd != 0)
    {
        if (on)
        {
            mLcd->backlight();
        }
        else
        {
            mLcd->noBacklight();
        }
    }
}


//const String &Station::forecastString(int forecast)
//{
//    if ((forecast > 0) && (forecast <= Board::UNKNOWN))
//    {
//        return weatherForecast[forecast];
//    }
//    else
//    {
//        return weatherForecast[Board::UNKNOWN];
//    }
//}

void Station::showLink(bool ok)
{
    if (mLcd != 0)
    {
        mLcd->setCursor(mLcdNbCols - 1, 0);
        if (ok)
        {
            mLcd->write(linkChar);
        }
        else
        {
            mLcd->write(noInformationChar);
        }
    }
}


void Station::showRtcSync(bool ok)
{
    if (mLcd != 0)
    {
        mLcd->setCursor(mLcdNbCols - 2, 0);
        if (ok)
        {
            mLcd->write(' ');
        }
        else
        {
            mLcd->write(noClockChar);
        }
    }
}

void Station::initLcd(uint8_t adresse, uint8_t nbCols, uint8_t nbLignes)
{
    if (mLcd == 0)
    {
        mLcd = new LiquidCrystal_I2C(adresse, nbCols, nbLignes); // set the LCD address to 0x20 for a 16 chars and 2 line display
        mLcd->init();                      // initialize the lcd
        mLcd->clear();
        mLcd->backlight();
    }
}
