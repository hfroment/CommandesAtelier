/*
 * Board.h
 *
 *  Created on: 21 déc. 2013
 *      Author: flea_new
 */

#ifndef WEATHERBOARD_H_
#define WEATHERBOARD_H_

#include "Arduino.h"

class Ds18b20;
#include "dht.h"

class Board
{
public:

    Board(bool useRtc = false, bool useVccio = false);
    virtual ~Board();

    void initDht(dht::dhtmodels type = dht::DHT11, uint8_t dhht11Pin = defaultDhtPin);
    bool readTU(uint8_t& temperature, uint8_t& humidity);
    // DS18b20
    void initDs18b20(uint8_t ds18B20Pin = defaultDs18b20Pin);
    bool readDs18b20(float& temp);

    // Fonction périodique
    bool gerer(bool top, bool pretop, bool& refresh);
    bool acqStage1();
    bool acqStage2();

    static Board *instance;

private:
    // TU
    static const uint8_t defaultDhtPin = A3;
    dht* mDht;
    bool mDhtDataOk;
    bool readTU();

    static const uint8_t defaultDs18b20Pin = A2;
    Ds18b20* mDs18b20;
    bool mDs18b20DataOk;
    bool startDs18b20();
    bool readDs18b20();

    bool mUseVccio;

    //static const uint8_t mLedPin = 13;
    static const uint8_t mVccioPin = 8;

    void ioPower(bool on = true);

};

#endif /* MINIWEATHERBOARD_H_ */
