#ifndef BRUMISATEUR_H
#define BRUMISATEUR_H

class Board;
class String;
class LiquidCrystal_I2C;

class Station
{
public:
    typedef enum
    {
        LCD_1602,
        LCD_2004,
        LCD_SSD1306
    }
    eTypeLcd;

    Station(bool useRtc = false, bool temperaturePresent = false, uint8_t lcdAddress = 0, eTypeLcd typeLcd = LCD_1602, uint8_t humidityType = 0 /*absent*/, bool pressurePresent = false, bool useLcd = true);

    void init();
    bool gerer(bool top, bool pretop = false);

    bool getTemperature(float& temperature);
    bool getHumidity(uint8_t & humidity);

    void log(uint8_t line, String& chaine);
    void clearScreen();
    void setBacklight(bool on);

    void showLink(bool ok = true);
    void showRtcSync(bool ok = true);

    // LCD
    void initLcd(uint8_t adresse = defaultLcdAddress, uint8_t nbCols = defaultLcdNbCols, uint8_t nbLignes = defaultLcdNbLines);
private:
    Board& mBoard;

    enum
    {
        temperatureChar = 0,
        humidityChar = 1,
        mystOnChar = 2,
        noInformationChar = 3,
        linkChar = 4,
        noClockChar = 5,
    };

    uint8_t mLcdAddress;
    eTypeLcd mTypeLcd;
    uint8_t mLcdNbCols;
    uint8_t mLcdNbLines;

    bool mUseRtc;
    bool mLcdPresent;
    bool mPressurePresent;
    uint8_t mHumidityType;
    bool mTemperaturePresent;
    bool mUseLcd;

    LiquidCrystal_I2C* mLcd;
    static const uint8_t defaultLcdAddress = 0x27;
    static const uint8_t defaultLcdNbCols = 16;
    static const uint8_t defaultLcdNbLines = 2;
};

#endif // BRUMISATEUR_H
