//#include <PinChangeInt.h>

#include "Station.h"
//#include <Sodaq_DS3231.h>

#include <dataaverage.h>

// Enable debug prints to serial monitor
//#define MY_DEBUG
#define MY_NODE_ID 5
// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <MyConfigFlea.h>

#include <MyConfig.h>
//#define MY_TRANSPORT_WAIT_READY_MS (0000ul)

#include <MySensors.h>

enum {
    CHILD_ID_TEMP,
    CHILD_ID_RELAIS_POMPE_BASSIN,
    CHILD_ID_RELAIS_POMPE_CUVE,
    CHILD_ID_RELAIS_PRISES_EXTERIEUR,
    CHILD_ID_RELAIS_LUMIERE_TERRASSE,
    CHILD_ID_INTER_GENERAL,
    CHILD_ID_INTER_POMPE_BASSIN,
    CHILD_ID_INTER_POMPE_CUVE,
    CHILD_ID_INTER_PRISES_EXTERIEUR,
    CHILD_ID_INTER_LUMIERE_TERRASSE,
    CHILD_ID_REMOTE_INTER_GENERAL,
    CHILD_ID_REMOTE_INTER_POMPE_BASSIN,
    CHILD_ID_REMOTE_INTER_POMPE_CUVE,
    CHILD_ID_REMOTE_INTER_PRISES_EXTERIEUR,
    CHILD_ID_REMOTE_INTER_LUMIERE_TERRASSE,
    CHILD_ID_REMOTE_PRIORITE_LOCAL,
    CHILD_ID_REMOTE_PRIORITE_REMOTE,
};

const uint8_t secondeTop = 00;
const uint8_t secondePretop = 59;

Station* weatherStation;

const uint8_t pinRELAIS_POMPE_BASSIN = 6;
const uint8_t pinRELAIS_POMPE_CUVE = A1;
const uint8_t pinRELAIS_PRISES_EXTERIEUR = A3;
const uint8_t pinRELAIS_LUMIERE_TERRASSE = 10;
const uint8_t pinINTER_GENERAL = 2;
const uint8_t pinINTER_POMPE_BASSIN = 3;
const uint8_t pinINTER_POMPE_CUVE = 4;
const uint8_t pinINTER_PRISES_EXTERIEUR = 1;
const uint8_t pinINTER_LUMIERE_TERRASSE = 9;

const uint8_t nbRelais = 4;
const uint8_t pinRelais[nbRelais] = {pinRELAIS_POMPE_BASSIN, pinRELAIS_POMPE_CUVE, pinRELAIS_PRISES_EXTERIEUR, pinRELAIS_LUMIERE_TERRASSE};
const uint8_t pinInter[nbRelais] = {pinINTER_POMPE_BASSIN, pinINTER_POMPE_CUVE, pinINTER_PRISES_EXTERIEUR, pinINTER_LUMIERE_TERRASSE};

bool etatRelais[nbRelais];
bool localCmdState[nbRelais];
bool remoteCmdState[nbRelais];

bool localStateINTER_GENERAL;
bool remoteStateINTER_GENERAL;
bool prioriteLocal;
bool prioriteRemote;

uint8_t syntheticRelaiState;
uint8_t syntheticRemoteCmd;
uint8_t syntheticLocalCmd;

MyMessage temperatureMsg(CHILD_ID_TEMP, V_TEMP);
MyMessage stateMessage;

bool lcdPresent = true;
uint8_t lcdAddress = 0x27;
uint8_t lcdType = Station::LCD_1602;
bool temperaturePresent = true;

void before()
{
    for (int i = 0; i < nbRelais; i++)
    {
        pinMode(pinRelais[i], OUTPUT);
        pinMode(pinInter[i], INPUT_PULLUP);
        digitalWrite(pinRelais[i], LOW);
        etatRelais[i] = false;
        localCmdState[i] = false;
        remoteCmdState[i] = false;
    }
    pinMode(pinINTER_GENERAL, INPUT_PULLUP);
    localStateINTER_GENERAL = false;
    remoteStateINTER_GENERAL = false;
    prioriteLocal = true;
    prioriteRemote = false;

    syntheticRelaiState = 0;
    syntheticRemoteCmd = 0;
    syntheticLocalCmd = 0;

    Serial.println("before()");
}
void presentation()
{
    Serial.println("presentation()");
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Shaton atelier", "1.0");

    // Register all sensors to gateway (they will be created as child devices)
    if (temperaturePresent)
    {
        present(CHILD_ID_TEMP, S_TEMP);//, "Temp compteur");
    }
    present(CHILD_ID_RELAIS_POMPE_BASSIN, S_BINARY);//, F("RELAIS_POMPE_BASSIN"));
    present(CHILD_ID_RELAIS_POMPE_CUVE, S_BINARY);//, F("RELAIS_POMPE_CUVE"));
    present(CHILD_ID_RELAIS_PRISES_EXTERIEUR, S_BINARY);//, F("RELAIS_PRISES_EXTERIEUR"));
    present(CHILD_ID_RELAIS_LUMIERE_TERRASSE, S_BINARY);//, F("RELAIS_LUMIERE_TERRASSE"));
    present(CHILD_ID_INTER_GENERAL, S_BINARY);//, F("INTER_GENERAL"));
    present(CHILD_ID_INTER_POMPE_BASSIN, S_BINARY);//, F("INTER_POMPE_BASSIN"));
    present(CHILD_ID_INTER_POMPE_CUVE, S_BINARY);//, F("INTER_POMPE_CUVE"));
    present(CHILD_ID_INTER_PRISES_EXTERIEUR, S_BINARY);//, F("INTER_PRISES_EXTERIEUR"));
    present(CHILD_ID_INTER_LUMIERE_TERRASSE, S_BINARY);//, F("INTER_LUMIERE_TERRASSE"));
    present(CHILD_ID_REMOTE_INTER_GENERAL, S_BINARY);//, F("REMOTE_INTER_GENERAL"));
    present(CHILD_ID_REMOTE_INTER_POMPE_BASSIN, S_BINARY);//, F("REMOTE_INTER_POMPE_BASSIN"));
    present(CHILD_ID_REMOTE_INTER_POMPE_CUVE, S_BINARY);//, F("REMOTE_INTER_POMPE_CUVE"));
    present(CHILD_ID_REMOTE_INTER_PRISES_EXTERIEUR, S_BINARY);//, F("REMOTE_INTER_PRISES_EXTERIEUR"));
    present(CHILD_ID_REMOTE_INTER_LUMIERE_TERRASSE, S_BINARY);//, F("REMOTE_INTER_LUMIERE_TERRASSE"));
    present(CHILD_ID_REMOTE_PRIORITE_LOCAL, S_BINARY);//, F("CHILD_ID_REMOTE_PRIORITE_LOCAL"));
    present(CHILD_ID_REMOTE_PRIORITE_REMOTE, S_BINARY);//, F("CHILD_ID_REMOTE_PRIORITE_REMOTE"));
}

const byte interruptPin = A0;
const byte boutonLcdPin = A6;

void setup()
{
    weatherStation = new Station(false, temperaturePresent, lcdAddress, lcdType, 0, false, false);
    weatherStation->init();
    if (lcdPresent)
    {
        weatherStation->log(0, "Shaton" + String("atelier"));
        weatherStation->log(1, "Node ID = " + String(getNodeId()));
        delay(5000);
        weatherStation->clearScreen();
        //Serial.println(chainePresentation);
        //Serial.println("Node ID = " + String(getNodeId()));
    }
    else
    {
        //Serial.println(chainePresentation);
        //Serial.println("Node ID = " + String(getNodeId()));
    }
    testLcdButton(true);
}

static const uint8_t periodeEnvoi = 6;

DataAverage temperatureAverage(periodeEnvoi);

bool setBacklightOn = false;

const unsigned long sleepTime = 950;

void loop()
{
    static uint8_t secondeCourante = 59;
    static bool linkLost = false;
    static uint8_t lastSentMinute = 255;
    static bool backlightState = true;
    static float currentTemperature = -1000.0;
    static bool startup = true;
    static uint8_t cycleCourant = 0;

    bool top = false;
    bool pretop = false;

    //static bool isMinute = false;

    if (processButtons())
    {
        sendStates(true);
        testLcdButton(true);
        displayState(currentTemperature);
    }
    else
    {
        if (startup)
        {
            startup = false;
            sendStates(true);
        }
    }

    {
        secondeCourante = (secondeCourante + 1) % 60;
//        bool okToSend = false;
        if (secondeCourante == secondeTop)
        {
            top = true;
//            okToSend = true;
            //Serial.println(F("Minute"));
        }
        else if (secondeCourante == secondePretop)
        {
            pretop = true;
        }

        displayState(currentTemperature);

        bool linkOk = isTransportReady();
        weatherStation->showLink(linkOk);
        if (linkOk)
        {
        }
        else
        {
            linkLost = true;
        }

        if (weatherStation->gerer(top, pretop))
        {
            float temperature;
            if (weatherStation->getTemperature(temperature))
            {
                Serial.println("getTemperature");
                temperatureAverage.addSample(temperature);
                currentTemperature = temperature;
            }

            if (linkOk)
            {
                if (cycleCourant > 1)
                {
                    cycleCourant--;
                }
                else
                {
                    cycleCourant = periodeEnvoi;
                    if (temperatureAverage.sampleCount() > 0)
                    {
                        Serial.println("sendTemperature");
                        send(temperatureMsg.set(temperatureAverage.average(), 1));
                    }
                    sendStates(true);
                }
            }
        }
        weatherStation->showLink(linkOk);
        weatherStation->setBacklight(setBacklightOn);

        if (setBacklightOn != backlightState)
        {
            weatherStation->setBacklight(setBacklightOn);
            backlightState = setBacklightOn;
        }
    }

    wait(sleepTime);
    testLcdButton(false);
}

void testLcdButton(bool init)
{
    static unsigned long cycleCourant = 0;
    const uint64_t nbCyclesAllumage = (uint64_t)60 * (uint64_t)1000 / (uint64_t)sleepTime;
    //Serial.print("testLcdButton : ");
    //Serial.println(analogRead(boutonLcdPin));
    if (init || (analogRead(boutonLcdPin) < 512))
    {
        setBacklightOn = true;
        cycleCourant = nbCyclesAllumage;
    }
    else
    {
        if (cycleCourant > 0)
        {
            //            Serial.print(F("Cycle "));
            //            Serial.println(cycleCourant);
            cycleCourant--;
        }
        else
        {
            setBacklightOn = false;
        }
    }
}

bool processButtons()
{
    bool retour = false;

    localStateINTER_GENERAL = digitalRead(pinINTER_GENERAL) == LOW;
    for (int i = 0; i < nbRelais; i++)
    {
        localCmdState[i] = digitalRead(pinInter[i]) == LOW;
    }

    bool newState[nbRelais];
    if (!prioriteLocal && !prioriteRemote)
    {
        for (int i = 0; i < nbRelais; i++)
        {
            if (localStateINTER_GENERAL && remoteStateINTER_GENERAL)
            {
                if (localCmdState[i] && remoteCmdState[i])
                {
                    newState[i] = true;
                }
                else
                {
                    newState[i] = false;
                }
            }
            else
            {
                newState[i] = false;
            }
        }
    }
    else
    {
        for (int i = 0; i < nbRelais; i++)
        {
            if ((localStateINTER_GENERAL && prioriteLocal) || (remoteStateINTER_GENERAL && prioriteRemote))
            {
                if ((localCmdState[i] && prioriteLocal) || (remoteCmdState[i] && prioriteRemote))
                {
                    newState[i] = true;
                }
                else
                {
                    newState[i] = false;
                }
            }
            else
            {
                newState[i] = false;
            }
        }
    }


    for (int i = 0; i < nbRelais; i++)
    {
        if (newState[i] != etatRelais[i])
        {
            retour = true;
        }
        etatRelais[i] = newState[i];
        digitalWrite(pinRelais[i], etatRelais[i]);
    }
    return retour;
}

void sendStates(bool all)
{
    //Serial.println("sendStates");
    if (isTransportReady())
    {
        stateMessage.setType(V_STATUS);
        stateMessage.setSensor(CHILD_ID_RELAIS_POMPE_BASSIN);
        send(stateMessage.set(etatRelais[0]));
        stateMessage.setSensor(CHILD_ID_RELAIS_POMPE_CUVE);
        send(stateMessage.set(etatRelais[1]));
        stateMessage.setSensor(CHILD_ID_RELAIS_PRISES_EXTERIEUR);
        send(stateMessage.set(etatRelais[2]));
        stateMessage.setSensor(CHILD_ID_RELAIS_LUMIERE_TERRASSE);
        send(stateMessage.set(etatRelais[3]));
        if (all)
        {
            stateMessage.setSensor(CHILD_ID_INTER_GENERAL);
            send(stateMessage.set(localStateINTER_GENERAL));
            stateMessage.setSensor(CHILD_ID_INTER_POMPE_BASSIN);
            send(stateMessage.set(localCmdState[0]));
            stateMessage.setSensor(CHILD_ID_INTER_POMPE_CUVE);
            send(stateMessage.set(localCmdState[1]));
            stateMessage.setSensor(CHILD_ID_INTER_PRISES_EXTERIEUR);
            send(stateMessage.set(localCmdState[2]));
            stateMessage.setSensor(CHILD_ID_INTER_LUMIERE_TERRASSE);
            send(stateMessage.set(localCmdState[3]));
            stateMessage.setSensor(CHILD_ID_REMOTE_INTER_GENERAL);
            send(stateMessage.set(remoteStateINTER_GENERAL));
            stateMessage.setSensor(CHILD_ID_REMOTE_INTER_POMPE_BASSIN);
            send(stateMessage.set(remoteCmdState[0]));
            stateMessage.setSensor(CHILD_ID_REMOTE_INTER_POMPE_CUVE);
            send(stateMessage.set(remoteCmdState[1]));
            stateMessage.setSensor(CHILD_ID_REMOTE_INTER_PRISES_EXTERIEUR);
            send(stateMessage.set(remoteCmdState[2]));
            stateMessage.setSensor(CHILD_ID_REMOTE_INTER_LUMIERE_TERRASSE);
            send(stateMessage.set(remoteCmdState[3]));
            stateMessage.setSensor(CHILD_ID_REMOTE_PRIORITE_LOCAL);
            send(stateMessage.set(prioriteLocal));
            stateMessage.setSensor(CHILD_ID_REMOTE_PRIORITE_REMOTE);
            send(stateMessage.set(prioriteRemote));
        }
    }
    else
    {
        Serial.println("not ready !!");
    }
}

void receive(const MyMessage &message)
{
    // We only expect one type of message from controller. But we better check anyway.
    if (message.getType() == V_STATUS)
    {
        Serial.println(message.getSensor());
        switch(message.getSensor())
        {
        default:
        case CHILD_ID_RELAIS_POMPE_BASSIN:
        case CHILD_ID_RELAIS_POMPE_CUVE:
        case CHILD_ID_RELAIS_PRISES_EXTERIEUR:
        case CHILD_ID_RELAIS_LUMIERE_TERRASSE:
        case CHILD_ID_INTER_GENERAL:
        case CHILD_ID_INTER_POMPE_BASSIN:
        case CHILD_ID_INTER_POMPE_CUVE:
        case CHILD_ID_INTER_PRISES_EXTERIEUR:
        case CHILD_ID_INTER_LUMIERE_TERRASSE:
            break;
        case CHILD_ID_REMOTE_INTER_GENERAL:
            remoteStateINTER_GENERAL = message.getBool();
            break;
        case CHILD_ID_REMOTE_PRIORITE_LOCAL:
            prioriteLocal = message.getBool();
            break;
        case CHILD_ID_REMOTE_PRIORITE_REMOTE:
            prioriteRemote = message.getBool();
            break;
        case CHILD_ID_REMOTE_INTER_POMPE_BASSIN:
            remoteCmdState[0] = message.getBool();
            break;
        case CHILD_ID_REMOTE_INTER_POMPE_CUVE:
            remoteCmdState[1] = message.getBool();
            break;
        case CHILD_ID_REMOTE_INTER_PRISES_EXTERIEUR:
            remoteCmdState[2] = message.getBool();
            break;
        case CHILD_ID_REMOTE_INTER_LUMIERE_TERRASSE:
            remoteCmdState[3] = message.getBool();
            break;
        }
    }
}

void displayState(float temperature)
{
    //    String ligne = F("Commandes");
    //    weatherStation->log(0, ligne);
    syntheticRelaiState = 0;
    syntheticRemoteCmd = 0;
    syntheticLocalCmd = 0;
    for (int i = 0; i < nbRelais; i++)
    {
        syntheticLocalCmd += localCmdState[i] << i;
        syntheticRemoteCmd += remoteCmdState[i] << i;
        syntheticRelaiState += etatRelais[i] << i;
    }
    syntheticLocalCmd += localStateINTER_GENERAL << nbRelais;
    syntheticRemoteCmd += remoteStateINTER_GENERAL << nbRelais;
    syntheticLocalCmd += 1 << nbRelais + 1;
    syntheticRemoteCmd += 1 << nbRelais + 1;
    syntheticRelaiState += 1 << nbRelais;
    String local = String(syntheticLocalCmd, BIN);
    String remote = String(syntheticRemoteCmd, BIN);
    local[0] = 'L';
    remote[0] = 'R';
    String priorite;
    if (prioriteLocal && prioriteRemote)
    {
        priorite = F("|");
    }
    else if (prioriteLocal && !prioriteRemote)
    {
        priorite = F(">");
    }
    else if (!prioriteLocal && prioriteRemote)
    {
        priorite = F("<");
    }
    else
    {
        priorite = F("&");
    }
    String ligne = local + priorite + remote;
    //    Serial.println(syntheticLocalCmd);
    //    Serial.println(syntheticRemoteCmd);
    //    Serial.println(ligne);
    weatherStation->log(1, ligne);
    String relais = String(syntheticRelaiState, BIN);
    relais[0] = '=';
    String temp;
    if (temperature != -1000.0)
    {
        temp = String(temperature + 0.05, 1);
        temp += String(F("\337C ")); // °C
    }
    ligne = String(F("R")) + relais + " " + temp;
    weatherStation->log(0, ligne);
    //    Serial.println(ligne);
}
