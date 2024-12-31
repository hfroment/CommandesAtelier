/*
 * Board.cpp
 *
 *  Created on: 21 déc. 2013
 *      Author: flea_new
 */

#include <dht.h>

#include "ds18b20.h"

#include <Wire.h>

#include "Board.h"

Board *Board::instance = NULL;

Board::Board(bool useRtc, bool useVccio):
    mDht(0),
    mDhtDataOk(false),
    mDs18b20(0),
    mDs18b20DataOk(false),
    mUseVccio(useVccio)
{
    instance = this;
    // TODO Auto-generated constructor stub
    if (mUseVccio)
    {
        pinMode(mVccioPin, OUTPUT);
    }

}

Board::~Board()
{
    // TODO Auto-generated destructor stub
    if (mDs18b20 == 0)
    {
        delete mDs18b20;
    }
    if (mDht == 0)
    {
        delete mDht;
    }
}

bool Board::acqStage1()
{
    // Pour la température, il faut la lire en avance de phase
    mDs18b20DataOk = startDs18b20();
    return mDs18b20DataOk;
}

bool Board::acqStage2()
{
    // On lit tout
    mDhtDataOk = readTU();
    mDs18b20DataOk = readDs18b20();
    return true;
}

bool Board::gerer(bool top, bool pretop, bool& refresh)
{
    bool retour = false;
    refresh = false;

//    if (seconde)
//    {
//        cadencer1s();
//    }
    if (top || pretop)
    {
        if (pretop)
        {
            // Pour la température, il faut la lire en avance de phase
            mDs18b20DataOk = startDs18b20();
//            mCompteurLecture--;
        }
        if (top)
        {
//            mCompteurLecture = delaisLecure;
            // On lit tout
            mDhtDataOk = readTU();
            mDs18b20DataOk = readDs18b20();
            refresh = true;
        }
//        mCompteurItPrecedent = mCompteurIt;
        retour = true;
    }
    return retour;
}

void Board::initDht(dht::dhtmodels type, uint8_t dhht11Pin)
{
    if (mDht == 0)
    {
        if (type < dht::DHT12)
        {
            mDht = new dht1wire(dhht11Pin, type);
        }
        else if (type == dht::DHT12)
        {
            mDht = new dht12();
        }
        // On allume
        ioPower();
    }
}

void Board::initDs18b20(uint8_t ds18B20Pin)
{
    if (mDs18b20 == 0)
    {
        mDs18b20 = new Ds18b20(ds18B20Pin);
    }
}

bool Board::readTU(uint8_t& temperature, uint8_t& humidity)
{
    if (mDhtDataOk)
    {
        temperature = mDht->getTemperature();
        humidity = mDht->getHumidity();
    }
    return mDhtDataOk;
}

bool Board::readTU()
{
    bool retour = false;
    if (mDht != 0)
    {
        if (dht::OK == mDht->read())
        {
            retour = true;
        }
    }
    return retour;
}

// Fonction récupérant la température depuis le DS18B20
// Retourne true si tout va bien, ou false en cas d'erreur
bool Board::readDs18b20(float& temp)
{
    bool retour = false;

    if (mDs18b20 != 0)
    {
        retour = mDs18b20->temperature(temp);
    }

    return retour;
}

// Fonction récupérant la température depuis le DS18B20
// Retourne true si tout va bien, ou false en cas d'erreur
bool Board::readDs18b20()
{
    bool retour = false;

    if (mDs18b20 != 0)
    {
        // Un cycle après la lecture, on récupére les datas
        mDs18b20DataOk = mDs18b20->getData();
        retour = mDs18b20DataOk;
    }

    return retour;
}

bool Board::startDs18b20()
{
    bool retour = false;

    if (mDs18b20 != 0)
    {
        // une seconde avant les traitements cycliques
//        if (mCompteurLecture == 1)
        {
            retour = mDs18b20->startRead();
        }
    }

    return retour;
}

void Board::ioPower(bool on)
{
    if (mUseVccio)
    {
        if (on)
        {
            digitalWrite(mVccioPin, 0);
        }
        else
        {
            digitalWrite(mVccioPin, 1);
        }
    }
}

