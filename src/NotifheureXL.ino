
// **********************************************
//***********************************************
//*********** NOTIFHEURE XL *********************
//***********************************************
//***********************************************
// *********  Byfeel 2019 ***********************
// **********************************************
const String ver = "0.9.6";
const String hardware = "NotifheureXL";
const String vInterface="";
//**************************
// Variable pour bibliotheque MQTT
#define MQTT_MAX_PACKET_SIZE 1024
// Bibliotheque à inclure
//********** eeprom / SPIFFS
#include <EEPROM.h>
#include <FS.h>   //Include SPIFFS
//*************************
// matrix
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
// Bibliotheque police de caractére
//****** XL
#include "Big_Segment_Clock.h"
#include "bigfontext.h"
//******** small
#include "Font_smallclock.h"
#include "extAscii_ParolaFont.h"
// bibliotheque JSON
#include <ArduinoJson.h>
//NTP
#include <NTPClient.h>
#include <TimeLib.h>
//#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// wifimanager - Network
#include <ESP8266WebServer.h>
//#include <WebSocketsServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>  //mqtt
//*************************
// Options
//*************************
// bibliotheque temperature
#include <DHTesp.h>
// Bibliotheque dfplayer ( mp3 )
#include "SoftwareSerial.h"    // pour les communications series avec le DFplayer
#include "DFRobotDFPlayerMini.h"  // bibliotheque pour le DFPlayer
// bibliotheque RTTTL pour jouer les melodies
#include <NonBlockingRtttl.h>
//librairies click bouton
#include <ClickButton.h>
// bibliotheque adafruit neopixel
#include <Adafruit_NeoPixel.h>
#include <TZ.h>
//***************************
// admin page config
//***************************
const char* www_username = "admin";
const char* www_password = "notif";

//*************************************************************
//**  Paramétre à ajuster en fonction de votre équipement *****
//*************************************************************

//****************************************************
// En fonction de vos matrices , si probléme         *
// d'affichage ( inversé , effet miroir , etc .....) *
// ***************************************************
// matrix   - decocher selon config matrix    ********
//#define HARDWARE_TYPE MD_MAX72XX::FC16_HW        //***
//#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW    //***
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW //***
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW   //***
// ***************************************************
// Branchement des matrices
#define CLK_PIN   14  // SCK (D5 wemos D1R1 ou mini )
#define DATA_PIN  13  // MOSI ( D7 wemos D1R1 ou mini )
#define CS_PIN    15  // SS ou CS ( D10 sur D1R1  ou D8 sur Mini )
//#define CS_PIN    12  // SS ( D10 sur D1R1  ou D6  sur Mini )  ---- Pour NOtifheure 1
// si modif CS_PÏN modifier AUDIOPINRX 15 !!!!!
// Pour info ancienne version NotifHeure
//#define CLK_PIN   14
//#define DATA_PIN  13
//#define CS_PIN    12
//************************************
// Options notif visuelle Si installé
//************************************
// PIN Notif visuel ( led , neopixel ou relais )
#define LEDPIN 5    // Sortie dédié notification lumineuse
//************************
//option NeoPixel
// Nombre de led si Ring ou strip neopixel en place
#define LED_COUNT  12
#define NEOTYPELED NEO_GRB
//   NEO_GRB     Pixels are wired for GRB bitstream (La plupart des produit neopixel vendu)
//   NEO_GRBW    Pixels are wired for GRBW bitstream (Si vous avez un neopixel avec Blanc en plus LED RGB + Blanc )
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2) _ Plus rare d'en trouver
//**************************************
// Notification sonore
// *************************************
// PIN AUDIO ( AUDIOPINTX = MP3PLAYER TX ou Sortie HP / Buzzer ou sortie RElais ou sortie Digital)
#define AUDIOPINTX 4    // Sortie principale pour Audio
// Si DFPLAYER MP3
// PINAUDIORX : uniquement our MP3player
// PIN qui serviront pour la communication série sur le module ESP
#define AUDIOPINRX 12   // Entree pour DFP audio uniquement
//#define AUDIOPINRX 15   // Inversé Entree pour DFP audio uniquement  si CS_PIN sur 12

// *******************
// Autres PIN utilisés
// *******************
// Même si options pas installés , laisser les valeurs par défaut - Auto détection
// *** PIN ******
// ***************
// PIN DHT
#define dhtpin 16 // GPIO16  egale a D2 sur WEMOS D1R1  ou D0 pour mini ( a verifier selon esp )
// Entree analogique pour auto luminosité
// PIN Analogique pour photocell
#define PINAUTO_LUM A0
//Boutons
#define PIN_BTN1 2
#define PIN_BTN2 0
// ************************
// *********** FIN PIN ****
// ************************

//
//************************************************
//************ Options matériels par défaut ******
//************ PAS NECESSAIRE DE MODIFIER ********
//************ Configuration dans SETUP **********
//************************************************
#define LED_OUT 0  // 0 aucune , 1 Led interne , 2 relais ,3 Ring neopixel ,4 Digital output
#define AUDIO_OUT 0 // 0 aucun , 1 buzzer , 2 MP3player , 3 autres ( sortie relais ou digital)
#define BOUTON1 false
#define BOUTON2 false
// ********** DHT ***************
// Detection auto si presence DHT
// ne pas modifier , sauf si Auto ne fonctionne pas
#define DHTTYPE DHTesp::AUTO_DETECT
//#define DHTTYPE DHTesp::DHT11  // si DHT11
//#define DHTTYPE DHTesp::DHT12  // si DHT12

//***************************
// Valeur par defaut config :
//***************************
#define TAILLENOTIF 4
#define TAILLECLOCK 4
#define NBZONE_SUP 0
#define MODEXL false
#define TZNAME "Europe/Paris"
#define NTPSERV "pool.ntp.org"
#define TEMPONTP 15  //Boucle NTP pour test ( environ 20 secondes au max )
#define TEMPODDHT 5 //temporisation pour affichage info DHT (min)
#define OFFSET_VALUE 60  // decalage en minute du fuseau horaire
#define OFFSET_ADJUST 0
#define DEBUG_DEF false
#define VOLUME 50 // volume audio de 0 à 100
#define _MP3START 1
#define _MP3NOTIF 3
#define TXTALARM "Votre Alarme vient de se déclencher !!!"
#define TXTMINUT "Fin Minuteur ! votre minuteur est terminé"
// gestion des effets
#define PAUSE_TIME 0
#define SCROLL_SPEED 40
// luminosité Led
#define BRIGHTNESS 30   // de 0 à 100
// historique
#define HF "NIQJ"
//**************************

// Message a afficher
#define  BUF_SIZE  160      // Taille max des notification ( nb de caractéres max )
#define MAX_HIST 10       // Nombres de message dans historique

//***************** MQTT
//mqtt server
//char mqtt_server[] = "192.168.8.210";
// basetopic
#define BASETOP "byfeel"
// Active broker
#define BROKER_VALID false
#define BROKER_IP ""
#define USERBROKER ""
#define PASSBROKER ""
#define PORT_BROKER 1883
#define TOPIC_NOTIF "/message"
#define TOPIC_OPTIONS "/options"
#define TOPIC_STATE "/state"
#define TEMPO_BROKER 120  // 120 secondes pour mise à jour états auto
#define DISCOVERY_PREFIX "homeassistant"

// Config valeur max sytéme ( ne pas modifier )
// matrices max gérés par systémes
#define MAX_DEVICES 64
// Max zone à gerer
#define MAX 8

//*********************************************
//********** AUDIO OUT BUZZER / HP ************
//*********************************************
const char* buzMusic[] = {
  "",
  "MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d",
  "StarWars:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#.6,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#6",
  "Indiana:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.,g,8p,8a,8b,8p,1f6,p,a,8p,8b,2c6,2d6,2e6,e,8p,8f,8g,8p,1c6,p,d6,8p,8e6,1f.6,g,8p,8g,e.6,8p,d6,8p,8g,e.6,8p,d6,8p,8g,f.6,8p,e6,8p,8d6,2c6",
  "PinkPante:d=4,o=6,b=100:8g_5,2a5,8b5,2c,8g_5,8a5,8b5,8c,8f,8e,8a5,8c,8e,2d_,16d,16c,16a5,8g5,1a5,8g_5,2a5,8b5,2c,8g_5,8a5,8b5,8c,8f,8e,8c,8e,8a,1g_,8g_5,2a5,8b5,2c,16g_5,8a5,8b5,8c,8f,8e,8a5,8c,8e,2d_,8d,16c,16a5",
  "AdamsFami:d=4,o=6,b=127:8c,f,8a,f,8c,b5,2g,8f,e,8g,e,8e5,a5,2f,8c,f,8a,f,8c,b5,2g,8f,e,8c,d,8e,1f,8c,8d,8e,8f,1p,8d,8e,8f_,8g,1p,8d,8e,8f_,8g,p,8d,8e,8f_,8g,p,c,8e,1f",
  "TheExo:d=4,o=6,b=160:8e5,8a5,8e5,8b5,8e5,8g5,8a5,8e5,8c,8e5,8d,8e5,8b5,8c,8e5,8a5,8e5,8b5,8e5,8g5,8a5,8e5,8c,8e5,8d,8e5,8b5,8c",
  "The_Simpsons:d=4,o=5,b=160:c.6,e6,f#6,8a6,g.6,e6,c6,8a,8f#,8f#,8f#,2g,8p,8p,8f#,8f#,8f#,8g,a#.,8c6,8c6,8c6,c6",
  "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a",
  "Arkanoid:d=4,o=5,b=140:8g6,16p,16g.6,2a#6,32p,8a6,8g6,8f6,8a6,2g6",
  "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6",
  "Xfiles:d=4,o=5,b=125:e,b,a,b,d6,2b.,1p,e,b,a,b,e6,2b.,1p,g6,f#6,e6,d6,e6,2b.,1p,g6,f#6,e6,d6,f#6,2b.,1p,e,b,a,b,d6,2b.,1p,e,b,a,b,e6,2b.,1p,e6,2b.",
  "AxelF:d=4,o=5,b=125:32p,8g,8p,16a#.,8p,16g,16p,16g,8c6,8g,8f,8g,8p,16d.6,8p,16g,16p,16g,8d#6,8d6,8a#,8g,8d6,8g6,16g,16f,16p,16f,8d,8a#,2g,p,SS,16f6,8d6,8c6,8a#,g,8a#.,16g,16p,16g,8c6,8g,8f,g,8d.6,16g,16p,16g,8d#6,8d6,8a#,8g,8d6,8g6,16g,16f,16p,16f,8d,8a#,2g",
  "PacMan:b=160:32b,32p,32b6,32p,32f#6,32p,32d#6,32p,32b6,32f#6,16p,16d#6,16p,32c6,32p,32c7,32p,32g6,32p,32e6,32p,32c7,32g6,16p,16e6,16p,32b,32p,32b6,32p,32f#6,32p,32d#6,32p,32b6,32f#6,16p,16d#6,16p,32d#6,32e6,32f6,32p,32f6,32f#6,32g6,32p,32g6,32g#6,32a6,32p,32b.6",
  "Dambuste:d=4,o=5,b=63:4f6,8a#6,8f6,8f6,16d#6,16d6,8d#6,8f6,4d6,8f6,8d6,8d6,16c6,16a#,8a,8c6,8a#.,16c6,8d6,8g6,8f.6,16d6,4f6,8c6,8f6,16g6,16a6,8a#6,4a6,4p",
  "Muppets:d=4,o=5,b=250:c6,c6,a,b,8a,b,g,p,c6,c6,a,8b,8a,8p,g.,p,e,e,g,f,8e,f,8c6,8c,8d,e,8e,8e,8p,8e,g,2p,c6,c6,a,b,8a,b,g,p,c6,c6,a,8b,a,g.,p,e,e,g,f,8e,f,8c6,8c,8d,e,8e,d,8d,c",
  "Bond:d=4,o=5,b=80:32p,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d#6,16d#6,16c#6,32d#6,32d#6,16d#6,8d#6,16c#6,16c#6,16c#6,16c#6,32e6,32e6,16e6,8e6,16d#6,16d6,16c#6,16c#7,c.7,16g#6,16f#6,g#.6",
  "TakeOnMe:d=4,o=4,b=160:8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5,8f#5,8e5,8f#5,8f#5,8f#5,8d5,8p,8b,8p,8e5,8p,8e5,8p,8e5,8g#5,8g#5,8a5,8b5,8a5,8a5,8a5,8e5,8p,8d5,8p,8f#5,8p,8f#5,8p,8f#5,8e5,8e5",
  "A-Team:d=8,o=5,b=125:4d#6,a#,2d#6,16p,g#,4a#,4d#.,p,16g,16a#,d#6,a#,f6,2d#6,16p,c#.6,16c6,16a#,g#.,2a#",
  "TopGun:d=4,o=4,b=31:32p,16c#,16g#,16g#,32f#,32f,32f#,32f,16d#,16d#,32c#,32d#,16f,32d#,32f,16f#,32f,32c#,16f,d#,16c#,16g#,16g#,32f#,32f,32f#,32f,16d#,16d#,32c#,32d#,16f,32d#,32f,16f#,32f,32c#,g#",
  "Smurfs:d=32,o=5,b=200:4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8f#,p,8a#,p,4g#,4p,g#,p,a#,p,b,p,c6,p,4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8b,p,8f,p,4f#",
  "Entertainer:d=4,o=5,b=140:8d,8d#,8e,c6,8e,c6,8e,2c.6,8c6,8d6,8d#6,8e6,8c6,8d6,e6,8b,d6,2c6,p,8d,8d#,8e,c6,8e,c6,8e,2c.6,8p,8a,8g,8f#,8a,8c6,e6,8d6,8c6,8a,2d6",
  "Looney:d=4,o=5,b=140:32p,c6,8f6,8e6,8d6,8c6,a.,8c6,8f6,8e6,8d6,8d#6,e.6,8e6,8e6,8c6,8d6,8c6,8e6,8c6,8d6,8a,8c6,8g,8a#,8a,8f",
  "20thCenFox:d=16,o=5,b=140:b,8p,b,b,2b,p,c6,32p,b,32p,c6,32p,b,32p,c6,32p,b,8p,b,b,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,g#,32p,a,32p,b,8p,b,b,2b,4p,8e,8g#,8b,1c#6,8f#,8a,8c#6,1e6,8a,8c#6,8e6,1e6,8b,8g#,8a,2b",
  "GoodBad:d=4,o=5,b=56:32p,32a#,32d#6,32a#,32d#6,8a#.,16f#.,16g#.,d#,32a#,32d#6,32a#,32d#6,8a#.,16f#.,16g#.,c#6,32a#,32d#6,32a#,32d#6,8a#.,16f#.,32f.,32d#.,c#,32a#,32d#6,32a#,32d#6,8a#.,16g#.,d#",
  "BackToTh:d=4,o=6,b=180:2c,8b5,8a5,b5,a5,g5,1a5,p,d,2c,8b5,8a5,b5,a5,g5,1a5"
};

//***************************************

//******************
// variable systemes
//******************

// Constante systémes
const unsigned long synchroNTP=600000;
const unsigned long MAGICEEP=12379025;
const uint32_t MAGICEP=0xAABBCCDD;
const byte EP_VERSION = 1;
const int eeAddress = 64;
const unsigned long _refTime=1514800000;
const unsigned long _refTimeHigh=2081030400;
#define BUFFEEP 280
//Hard Config  ( sauvé en EEPROM )
struct sHardConfig {
  uint32_t magic;
  byte EPvers;
  bool setup;
  byte maxDisplay;
  byte zoneTime;
  byte maxZonesMsg;
  bool XL;
  bool perso;
  byte ZP[MAX];
  int Offset;
  char TZname[35];
  char nom[20];              // Nom notifheure
  bool btn1;
  bool btn2;
  int typeAudio;
  int typeLED;
};
sHardConfig hardConfig;

// software config  ( sauvegarde SPIFFS )
struct sConfigSys {
//  bool setup;                // verifie si setup initial ok
  bool config;               // verifie si fichier config existe
  int pauseTime;             // Temps de pause entre chaque animation
  int scrollSpeed;           // vitesse de defliement
  byte intensity;            // Itensité par defaut matrice
  char NTPSERVER[30];        // Adresse NTPserveur
  bool DEBUG;                 // si debug actif
  byte posClock;              //
  char hostName[20];          // nom reseau
  bool broker;                //activation BROKER
  char servbroker[50];          //adresse ip serveur broker
  char userbroker[20];
  char passbroker[20];
  int portbroker;
  int tempobroker;
  char prefixdiscovery[20];    // prefix discovery pour HA
  bool SEC;
  bool HOR;
  bool LUM;
  bool REV;
  bool DHT;                   // True si affichage DHT demandé sur notif
  byte tempDDHT;              // Tempo Display DHT
  byte timeREV[2];            // Heure reveil
  char msgAlarme[80];         // Message a affiché pour alarme
  char msgMinuteur[80];       // Message a affiché pour fin de minuteur
  bool LED;                   // etat LED
  int LEDINT;                 // Intensité par defaut de la veilleuse
  byte color;                 // couleur par défaut veilleuse
  char charOff;               // caractére / symbole à afficher sur horloge eteinte
  char textnotif[20];         // texte prénotif
  bool ALN;                   // Auto Longueur Notif
  bool box;                    // si urltobox actif
  char URL_Action1[130];
  char URL_Action2[130];
  char URL_Action3[130];
  char URL_Update[130];
  bool alDay[7];              // jour alarme
  int fxint;                  // intensité par defaut fx visuel
  byte fxcolor;               //colueur par defaut fx
  char hflag[10];              // flag autorisé pour historique
  int CrTime;
  byte fxCR;                  // fx minuteur par defaut
  byte fxAL;                  // fx Alarme par defaut
  byte fxSoundCR;             // fx Sound Minuteur par defaut
  byte fxSoundAL;             // fx Sound Alarme par defaut
  byte action[2];             // Action pour minuteur / alarme
  int MP3Start;               // numero MP3 à jouer au démarrage
  int MP3Notif;               // numero MP3 a jouer pour notification
  int typeAudio;
  int volumeAudio;             // volume audio par defaut
  int timeAdjust;               // décalage dayLightSaving
  bool btn1;
  bool btn2;
  byte btnclic[2][3];          // valeur par defaut des boutons
};
sConfigSys configSys;
const size_t capacityMQTT = JSON_OBJECT_SIZE(10) + 250;
const size_t capacityConfig = JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(6) + JSON_ARRAY_SIZE(8) + JSON_OBJECT_SIZE(105) + 2000;
const size_t capacityHisto =3*JSON_ARRAY_SIZE(10)  + JSON_OBJECT_SIZE(4) + 500;
const char *fileconfig = "/config/config.json";  // fichier config
const char *fileHist = "/config/Historique.json";  // fichier config

// init network (wifi , broker )
WiFiUDP ntpUDP;
WiFiClient espClient;
HTTPClient http;
PubSubClient MQTTclient(espClient);

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset

NTPClient timeClient(ntpUDP, NTPSERV,0, synchroNTP);

//variable pour inversion zone superieur selon module
bool invertUpperZone = false;  // Type ICS ou FC16

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Declare votre neopixel Ring
Adafruit_NeoPixel ring(LED_COUNT, LEDPIN, NEOTYPELED + NEO_KHZ800);

// init audio
SoftwareSerial mySoftwareSerial(AUDIOPINTX,AUDIOPINRX); //  Tx,RX ( Dfplayer )
DFRobotDFPlayerMini myDFPlayer;  // init player


//  boutons
ClickButton bouton1(PIN_BTN1, LOW, CLICKBTN_PULLUP);
ClickButton bouton2(PIN_BTN2, LOW, CLICKBTN_PULLUP);
//ClickButton bouton2(PINbouton2, LOW, CLICKBTN_PULLUP);

// init dht
DHTesp dht;
float humidity=0;
float temperature=0;
float heatIndex=0;
float dewPoint=0;
byte per=0;
long int stamp_DHT;
String Modele;

// définition des chaines de stockage pour les zones inf et sup
char msgH[15];
char msgL[15]; // un buffer de 15 caractères au max


//char Notifs[BUF_SIZE];
// variable notif
bool AUDIONOTIF = false;
bool Notif=false;
byte nzofx;
//variable minuteur
bool CR=false;
bool CRStop=false;
int minuteur=0;
// variable reveil
byte notifREV;              // 0 aucune notif , 1 notif led , 2 notif audio , 3 les deux

//variable affichage Zone
byte _spaceChar=1;
int xl=0;
byte _maxDisplayMsg;
byte _maxZones;
byte ZonesWide[MAX];
byte ZWP[MAX];   // zone Wide perso
byte Zones[MAX];
enum {Time_Lo,Time_Up,ZN_1,ZN_2,ZN_3,ZN_4,ZN_5,ZN_6};
byte zoneXL_L,zoneXL_H,zoneTime,zoneMsg;

//variable sys
byte upOpt=0;   // pour mise a jour options
String msgDebug="";
String _pagehtml="index.html";
bool _photocell=false;
bool _dht=false;
String  hostname;
long int _lastSynchro = 0;
long int _startTime=0;
long int _upTime=0;
long lastReconnectAttempt = 0;
String mdnsName;
String topicName;
String idNotif;
bool statebroker=false;
String infoBOX="OK";
bool sendBox=false;
bool ntpOK;
bool netOK;
int netcode;
bool checkntp=false;
bool checknet=false;
bool startSend=true;

//nom fichier charge
File fsUploadFile;
void handleFileUpload();

// boutons
byte clic=0;
//MP3
int totalMP3=0;
int nMP3_1=0;
//************* Structure notif
struct sNotif {
  char Notif[BUF_SIZE];
  textPosition_t pos;
  byte fxIn;
  byte fxOut;
  uint16_t speed;
  uint16_t pause;
  byte AnIn;
  byte AnOut;
  bool Alert;
  int type;
  int intensity;
  int cycle;
};

sNotif Notification[7];
byte tab_notif=1;
sNotif fxNotif;

//variable notif
int Nz,Sc,Pa,Cyc;
byte Ntype,Fi,Fo,An,LuN;
textPosition_t Npos;
char Nflag;
/*
 * Structure historique
 */
struct sHistNotif {
  char Notif[BUF_SIZE];
  long int date;
  char flag;
};
sHistNotif histNotif[MAX_HIST];


byte indexHist = 0;

const String matrices[]={"PAROLA","GENERIC","ICSTATION","FC16"};
// couleur Led

const uint32_t couleur[]={
  //Couleur RVB
  ring.Color(255, 255, 255),// Blanc
  ring.Color(255,   0,   0), //Rouge
  ring.Color(0,   0,   255), // Bleue
  ring.Color(0, 255,   0), // Vert
  ring.Color(255, 255,   0), // Jaune
  // couleur HSV
  ring.gamma32(ring.ColorHSV(5462, 255, 255)), // Orange
  ring.gamma32(ring.ColorHSV(52000, 255, 255)), // violet
  ring.gamma32(ring.ColorHSV(60080)) // rose
};

enum {White,Red,Blue,Green,Yellow,Orange,Purple,Pink};

struct snotifLed {
  int fx;
  int lum;
  byte color;
  int speed;
  int loop;
};
struct snotifLed notifLed={0,BRIGHTNESS,White,50,1};
const struct snotifLed nofxled={0,0,White,40,0};
const struct snotifLed on={1,100,White,40,1};
const struct snotifLed flash={2,100,White,40,4};
const struct snotifLed breath={3,100,White,50,2};
const struct snotifLed rainbow={4,100,White,50,4};
const struct snotifLed colorWipe={5,100,White,50,2};
const struct snotifLed colorWipeFill={6,100,White,50,1};
const struct snotifLed chaseColor={7,100,White,50,1};

snotifLed FX_led[]={nofxled,on,flash,breath,rainbow,colorWipe,colorWipeFill,chaseColor};

struct snotifAudio {
  int fx;
  int piste;
  byte volume;
  byte nzo;
  bool state;
};
struct snotifAudio notifAudio;
//const struct snotifAudio audioBuz={};

//******** fx *******
textEffect_t  effect[] =
{
  PA_PRINT,
  PA_SCROLL_LEFT,
  PA_SCROLL_UP_LEFT,
  PA_SCROLL_DOWN_LEFT,
  PA_SCROLL_UP,
  PA_GROW_UP,
  PA_SCAN_HORIZ,
  PA_BLINDS,
  PA_WIPE,
  PA_SCAN_VERTX,
  PA_SLICE,
  PA_FADE,
  PA_OPENING_CURSOR,
  PA_NO_EFFECT,
  PA_SPRITE,
  PA_CLOSING,
  PA_SCAN_VERT,
  PA_WIPE_CURSOR,
  PA_SCAN_HORIZX,
  PA_DISSOLVE,
  PA_MESH,
  PA_OPENING,
  PA_CLOSING_CURSOR,
  PA_GROW_DOWN,
  PA_SCROLL_DOWN,
  PA_SCROLL_DOWN_RIGHT,
  PA_SCROLL_UP_RIGHT,
  PA_SCROLL_RIGHT,
  PA_RANDOM,
};


//** service WEB
ESP8266WebServer server(80);         // serveur WEB sur port 80
//WebSocketsServer webSocket = WebSocketsServer(81);

//*****************************************
// UTF8 - Ascii etendu
//*****************************************
uint8_t utf8Ascii(uint8_t ascii)
// Convert a single Character from UTF8 to Extended ASCII according to ISO 8859-1,
// also called ISO Latin-1. Codes 128-159 contain the Microsoft Windows Latin-1
// extended characters:
// - codes 0..127 are identical in ASCII and UTF-8
// - codes 160..191 in ISO-8859-1 and Windows-1252 are two-byte characters in UTF-8
//                 + 0xC2 then second byte identical to the extended ASCII code.
// - codes 192..255 in ISO-8859-1 and Windows-1252 are two-byte characters in UTF-8
//                 + 0xC3 then second byte differs only in the first two bits to extended ASCII code.
// - codes 128..159 in Windows-1252 are different, but usually only the €-symbol will be needed from this range.
//                 + The euro symbol is 0x80 in Windows-1252, 0xa4 in ISO-8859-15, and 0xe2 0x82 0xac in UTF-8.
//
// Modified from original code at http://playground.arduino.cc/Main/Utf8ascii
// Extended ASCII encoding should match the characters at http://www.ascii-code.com/
//
// Return "0" if a byte has to be ignored.
{
  static uint8_t cPrev;
  uint8_t c = '\0';

  if (ascii < 0x7f)   // Standard ASCII-set 0..0x7F, no conversion
  {
    cPrev = '\0';
    c = ascii;
  }
  else
  {
    switch (cPrev)  // Conversion depending on preceding UTF8-character
    {
    case 0xC2: c = ascii;  break;
    case 0xC3: c = ascii | 0xC0;  break;
    case 0x82: if (ascii==0xAC )   c = 0x80; // Euro symbol special case

    }
    cPrev = ascii;   // save last char
  }

  return(c);
}



void utf8Ascii(char* s)
// In place conversion UTF-8 string to Extended ASCII
// The extended ASCII string is always shorter.
{
  uint8_t c;
  char *cp = s;

  while (*s != '\0')
  {
    c = utf8Ascii(*s++);
    if (c != '\0')
      *cp++ = c;
  }
  *cp = '\0';   // terminate the new string
}

// *****************************
// ANIMATION
// SPRITE DEFINITION (PAC MAn )
// *****************************
// Sprite Definitions
const uint8_t F_PMAN1 = 6;
const uint8_t W_PMAN1 = 8;
const uint8_t PROGMEM pacman1[F_PMAN1 * W_PMAN1] =  // gobbling pacman animation
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
};

const uint8_t F_PMAN2 = 6;
const uint8_t W_PMAN2 = 18;
const uint8_t PROGMEM pacman2[F_PMAN2 * W_PMAN2] =  // ghost pursued by a pacman
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
};

const uint8_t F_WAVE = 14;
const uint8_t W_WAVE = 14;
const uint8_t PROGMEM wave[F_WAVE * W_WAVE] =  // triangular wave / worm
{
  0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10,
  0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20,
  0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40,
  0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
  0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
  0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10,
  0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08,
  0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02, 0x04,
  0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x02,
  0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02,
  0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04,
  0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08,
};

const uint8_t F_ROLL1 = 4;
const uint8_t W_ROLL1 = 8;
const uint8_t PROGMEM roll1[F_ROLL1 * W_ROLL1] =  // rolling square
{
  0xff, 0x8f, 0x8f, 0x8f, 0x81, 0x81, 0x81, 0xff,
  0xff, 0xf1, 0xf1, 0xf1, 0x81, 0x81, 0x81, 0xff,
  0xff, 0x81, 0x81, 0x81, 0xf1, 0xf1, 0xf1, 0xff,
  0xff, 0x81, 0x81, 0x81, 0x8f, 0x8f, 0x8f, 0xff,
};

const uint8_t F_ROLL2 = 4;
const uint8_t W_ROLL2 = 8;
const uint8_t PROGMEM roll2[F_ROLL2 * W_ROLL2] =  // rolling octagon
{
  0x3c, 0x4e, 0x8f, 0x8f, 0x81, 0x81, 0x42, 0x3c,
  0x3c, 0x72, 0xf1, 0xf1, 0x81, 0x81, 0x42, 0x3c,
  0x3c, 0x42, 0x81, 0x81, 0xf1, 0xf1, 0x72, 0x3c,
  0x3c, 0x42, 0x81, 0x81, 0x8f, 0x8f, 0x4e, 0x3c,
};

const uint8_t F_LINES = 3;
const uint8_t W_LINES = 8;
const uint8_t PROGMEM lines[F_LINES * W_LINES] =  // spaced lines
{
  0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00,
  0xff, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff,
};

const uint8_t F_ARROW1 = 3;
const uint8_t W_ARROW1 = 10;
const uint8_t PROGMEM arrow1[F_ARROW1 * W_ARROW1] =  // arrow fading to center
{
  0x18, 0x3c, 0x7e, 0xff, 0x7e, 0x00, 0x00, 0x3c, 0x00, 0x00,
  0x18, 0x3c, 0x7e, 0xff, 0x00, 0x7e, 0x00, 0x00, 0x18, 0x00,
  0x18, 0x3c, 0x7e, 0xff, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x18,
};

const uint8_t F_ARROW2 = 3;
const uint8_t W_ARROW2 = 9;
const uint8_t PROGMEM arrow2[F_ARROW2 * W_ARROW2] =  // arrow fading to outside
{
  0x18, 0x3c, 0x7e, 0xe7, 0x00, 0x00, 0xc3, 0x00, 0x00,
  0x18, 0x3c, 0x7e, 0xe7, 0xe7, 0x00, 0x00, 0x81, 0x00,
  0x18, 0x3c, 0x7e, 0xe7, 0x00, 0xc3, 0x00, 0x00, 0x81,
};

const uint8_t F_SAILBOAT = 1;
const uint8_t W_SAILBOAT = 11;
const uint8_t PROGMEM sailboat[F_SAILBOAT * W_SAILBOAT] =  // sail boat
{
  0x10, 0x30, 0x58, 0x94, 0x92, 0x9f, 0x92, 0x94, 0x98, 0x50, 0x30,
};

const uint8_t F_STEAMBOAT = 2;
const uint8_t W_STEAMBOAT = 11;
const uint8_t PROGMEM steamboat[F_STEAMBOAT * W_STEAMBOAT] =  // steam boat
{
  0x10, 0x30, 0x50, 0x9c, 0x9e, 0x90, 0x91, 0x9c, 0x9d, 0x90, 0x71,
  0x10, 0x30, 0x50, 0x9c, 0x9c, 0x91, 0x90, 0x9d, 0x9e, 0x91, 0x70,
};

const uint8_t F_HEART = 5;
const uint8_t W_HEART = 9;
const uint8_t PROGMEM heart[F_HEART * W_HEART] =  // beating heart
{
  0x0e, 0x11, 0x21, 0x42, 0x84, 0x42, 0x21, 0x11, 0x0e,
  0x0e, 0x1f, 0x33, 0x66, 0xcc, 0x66, 0x33, 0x1f, 0x0e,
  0x0e, 0x1f, 0x3f, 0x7e, 0xfc, 0x7e, 0x3f, 0x1f, 0x0e,
  0x0e, 0x1f, 0x33, 0x66, 0xcc, 0x66, 0x33, 0x1f, 0x0e,
  0x0e, 0x11, 0x21, 0x42, 0x84, 0x42, 0x21, 0x11, 0x0e,
};

const uint8_t F_INVADER = 2;
const uint8_t W_INVADER = 10;
const uint8_t PROGMEM invader[F_INVADER * W_INVADER] =  // space invader
{
  0x0e, 0x98, 0x7d, 0x36, 0x3c, 0x3c, 0x36, 0x7d, 0x98, 0x0e,
  0x70, 0x18, 0x7d, 0xb6, 0x3c, 0x3c, 0xb6, 0x7d, 0x18, 0x70,
};

const uint8_t F_ROCKET = 2;
const uint8_t W_ROCKET = 11;
const uint8_t PROGMEM rocket[F_ROCKET * W_ROCKET] =  // rocket
{
  0x18, 0x24, 0x42, 0x81, 0x99, 0x18, 0x99, 0x18, 0xa5, 0x5a, 0x81,
  0x18, 0x24, 0x42, 0x81, 0x18, 0x99, 0x18, 0x99, 0x24, 0x42, 0x99,
};

const uint8_t F_FBALL = 2;
const uint8_t W_FBALL = 11;
const uint8_t PROGMEM fireball[F_FBALL * W_FBALL] =  // fireball
{
  0x7e, 0xab, 0x54, 0x28, 0x52, 0x24, 0x40, 0x18, 0x04, 0x10, 0x08,
  0x7e, 0xd5, 0x2a, 0x14, 0x24, 0x0a, 0x30, 0x04, 0x28, 0x08, 0x10,
};

const uint8_t F_CHEVRON = 1;
const uint8_t W_CHEVRON = 9;
const uint8_t PROGMEM chevron[F_CHEVRON * W_CHEVRON] =  // chevron
{
  0x18, 0x3c, 0x66, 0xc3, 0x99, 0x3c, 0x66, 0xc3, 0x81,
};

const uint8_t F_WALKER = 5;
const uint8_t W_WALKER = 7;
const uint8_t PROGMEM walker[F_WALKER * W_WALKER] =  // walking man
{
    0x00, 0x48, 0x77, 0x1f, 0x1c, 0x94, 0x68,
    0x00, 0x90, 0xee, 0x3e, 0x38, 0x28, 0xd0,
    0x00, 0x00, 0xae, 0xfe, 0x38, 0x28, 0x40,
    0x00, 0x00, 0x2e, 0xbe, 0xf8, 0x00, 0x00,
    0x00, 0x10, 0x6e, 0x3e, 0xb8, 0xe8, 0x00,
};

struct
{
  const uint8_t *data;
  uint8_t width;
  uint8_t frames;
}
sprite[] =
{
  { pacman1, W_PMAN1, F_PMAN1 },
  { arrow1, W_ARROW1, F_ARROW1 },
  { roll1, W_ROLL1, F_ROLL1 },
  { walker, W_WALKER, F_WALKER },
  { invader, W_INVADER, F_INVADER },
  { chevron, W_CHEVRON, F_CHEVRON },
  { heart, W_HEART, F_HEART },
  { steamboat, W_STEAMBOAT, F_STEAMBOAT },
  { sailboat, W_SAILBOAT, F_SAILBOAT },
  { fireball, W_FBALL, F_FBALL },
  { rocket, W_ROCKET, F_ROCKET },
  { lines, W_LINES, F_LINES },
  { wave, W_WAVE, F_WAVE },
  { pacman2, W_PMAN2, F_PMAN2 },
  { arrow2, W_ARROW2, F_ARROW2 },
  { roll2, W_ROLL2, F_ROLL2 }
};



 //void setType() {MD_MAX72XX::setModuleType(HARDWARE_TYPE);}
/*************************
* * *lecture eeprom * * *
*************************/
void debugEeprom(bool rEp=true) {
  Serial.println("taille structure EEPROM :"+String(sizeof(hardConfig)));
  if (rEp) {
    Serial.println("test lecture eeprom");
  EEPROM.begin(BUFFEEP);
  EEPROM.get( eeAddress, hardConfig);
  EEPROM.end();
} else Serial.println("test lecture structure hardonfig");
  Serial.println("************************************");
  Serial.println("Configuration EEprom");
  Serial.println("Magic Number constante :"+String(MAGICEP));
  Serial.println("Magic Number = "+String( hardConfig.magic));
  Serial.println("Version EEProm  = "+String( hardConfig.EPvers));
  Serial.println("Mode XL  = "+String( hardConfig.XL));
  Serial.println("Mode maxDisplay  = "  +String( hardConfig.maxDisplay));
  Serial.println("Mode perso  = "+String( hardConfig.perso));
  Serial.println("Mode zoneTime  = "+String( hardConfig.zoneTime));
  Serial.println("Mode maxZonesMsg  = "+String( hardConfig.maxZonesMsg));
  Serial.println("Mode Setup  = "+String( hardConfig.setup));
  for (int i=0;i<MAX;i++) {
    Serial.println("Mode ZP-"+String(i)+" = "+String(hardConfig.ZP[i]));
  }
  Serial.println("Nom  = "+String(hardConfig.nom));
  Serial.println("Nom TZ  = "+String(hardConfig.TZname));
  Serial.println("offset TZ = "+String(hardConfig.Offset));
  Serial.println("BTN1 = "+String(hardConfig.btn1));
  Serial.println("BTN2 = "+String(hardConfig.btn2));
  Serial.println("Type Audio = "+String(hardConfig.typeAudio));
  Serial.println("Type LED = "+String(hardConfig.typeLED));
  Serial.println("************************************");
}

void writeEEPROM() {
  EEPROM.begin(BUFFEEP);
 hardConfig.magic=MAGICEP;
 hardConfig.EPvers=EP_VERSION;
 EEPROM.put(eeAddress,hardConfig);
 EEPROM.commit();
 EEPROM.end();
}


void readEConfig(bool reset=false) {

EEPROM.begin(BUFFEEP);
bool err=false;
bool writeOK=false;
EEPROM.get( eeAddress, hardConfig);
if (hardConfig.magic != MAGICEP) err=true;
else err=false;
if (reset) err=true;
if (err) {
  writeOK=true;
  Serial.println("Pas de configuration enregistrement par défaut");
 hardConfig.maxDisplay=TAILLENOTIF;
 hardConfig.zoneTime=TAILLECLOCK;
 hardConfig.maxZonesMsg=NBZONE_SUP;
 hardConfig.XL=MODEXL;
 hardConfig.perso=false;
 for (int i=0;i<MAX;i++) {
 hardConfig.ZP[i]=i;
 }
 strlcpy(hardConfig.TZname,TZNAME,sizeof(hardConfig.TZname));
 strlcpy(hardConfig.nom,"New",sizeof(hardConfig.nom));
 hardConfig.Offset=OFFSET_VALUE;
 hardConfig.setup=false;
 hardConfig.btn1=BOUTON1;
 hardConfig.btn2=BOUTON2;
 hardConfig.typeAudio=AUDIO_OUT;
 hardConfig.typeLED=LED_OUT;
}
/*
if (hardConfig.EPvers<2) {
  writeOK=true;

}
*/
debugEeprom(false);
EEPROM.end();



if (writeOK) writeEEPROM();

//verif ecriture

debugEeprom();
}

//lecture fichier Historique
String loadHisto(const char *fileHist, sHistNotif *histo ) {
String info;
info="H:ok";
  File file = SPIFFS.open(fileHist, "r");
   if (!file) {
     info="H:err";
   }
   DynamicJsonDocument docHisto(capacityHisto);
   DeserializationError err = deserializeJson(docHisto, file);
   if (err) {
      info="H:new";
     }
// lecture fichier historique - si n'existe pas valeur par default

   JsonArray notif = docHisto["notif"];
   JsonArray stamp = docHisto["date"];
   JsonArray flag = docHisto["flag"];
   int index = notif.size();

   for (int i=0;i<index;i++) {
   strlcpy(histo[i].Notif,notif[i] ,sizeof(histo[i].Notif) );
   histo[i].date=stamp[i];
   histo[i].flag=flag[i];
  }
   indexHist=index;
return info;
}


String createHisto ( sHistNotif *histo ) {
  String json;
  DynamicJsonDocument docHisto(capacityHisto);
  docHisto["index"]=indexHist;
  JsonArray docNotif = docHisto.createNestedArray("notif");
  JsonArray docDate = docHisto.createNestedArray("date");
  JsonArray docFlag = docHisto.createNestedArray("flag");
  for (int i=0;i<indexHist;i++) {
  docNotif.add(histo[i].Notif);

  docDate.add(histo[i].date);

  docFlag.add(histo[i].flag);
}
  // envoie config json
  serializeJsonPretty(docHisto,json);
  return json;
}
void saveHisto(const char *fileHist,String json) {
  File  f = SPIFFS.open(fileHist, "w");
  if (!f) {
    //InfoDebugtInit=InfoDebugtInit+" Fichier config Jeedom absent -";
    if (configSys.DEBUG) Serial.println("Fichier historique absent - création fichier");
   }
   f.print(json);  // sauvegarde de la chaine
   f.close();
}

// gestion Historique
void addHisto(String msg,int NZO,char flag) {
  if (indexHist>0) {
    //decalege tableau si plus dun enregistrement
    for (int i=indexHist;i>0;i--)
    {
      memcpy( histNotif[i].Notif,histNotif[i-1].Notif, sizeof(histNotif[i].Notif) );
      histNotif[i].date=histNotif[i-1].date;
      histNotif[i].flag=histNotif[i-1].flag;
    }
  }
  //ajout enregistrement sur index en cours
  msg.toCharArray( histNotif[0].Notif,BUF_SIZE);
  histNotif[0].date=now();
  histNotif[0].flag=flag;
  // test index
  if (indexHist > MAX_HIST-1) {
    indexHist=MAX_HIST-1;
  } else indexHist++;
}

bool checkFlag(char F) {
  String flags=configSys.hflag;
  if ( flags.indexOf(F)<0) return false;
  else return true;
}

// JSON
// load config JSON Suystem
void loadConfigSys(const char *fileconfig, sConfigSys  &config) {
   //Init buffer json config
//const size_t capacityConfig = 2*JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(41) + 1500;
  // Open file config
  File file = SPIFFS.open(fileconfig, "r");
   if (!file) {
     msgDebug+="Fonction load : Fichier Config absent ---";
     //InfoDebugtInit=InfoDebugtInit+" Fichier config Jeedom absent -";
     //if (configSys.DEBUG) Serial.println("Fichier Config absent");
    }
  DynamicJsonDocument docConfig(capacityConfig);
  DeserializationError err = deserializeJson(docConfig, file);
  if (err) {
      msgDebug+="deserializeJson() failed: ";
      msgDebug+=err.c_str();
    }

  // Initialisation des variables systémes pour configuration , si non présente valeur par defaut affecté
  strlcpy(config.NTPSERVER,docConfig["NTPSERVER"] | "pool.ntp.org",sizeof(config.NTPSERVER));
  strlcpy(config.hostName,docConfig["SUFFIXEHOST"] | "notifXL",sizeof(config.hostName));
  strlcpy(config.msgAlarme,docConfig["MSGALARM"] | TXTALARM,sizeof(config.msgAlarme));
  strlcpy(config.msgMinuteur,docConfig["MSGMINUT"] | TXTMINUT,sizeof(config.msgMinuteur));
  strlcpy(config.hflag,docConfig["HFLAG"] | HF,sizeof(config.hflag));
  //config.timeZone = docConfig["TIMEZONE"] | UTC;
  config.config = docConfig["CONFIG"] | false;
  config.broker = docConfig["BROKER"] | BROKER_VALID;
  strlcpy(config.servbroker,docConfig["SRVBROKER"] | BROKER_IP,sizeof(config.servbroker));
  strlcpy(config.userbroker,docConfig["UBROKER"] | USERBROKER,sizeof(config.userbroker));
  strlcpy(config.passbroker,docConfig["PBROKER"] | PASSBROKER,sizeof(config.passbroker));
  strlcpy(config.prefixdiscovery,docConfig["PREFIXHA"] | DISCOVERY_PREFIX,sizeof(config.prefixdiscovery));
  config.portbroker=docConfig["PORTBROKER"] | PORT_BROKER;
  config.tempobroker=docConfig["TEMPOBROKER"] | TEMPO_BROKER;
  config.DEBUG = docConfig["DEBUG"] | DEBUG_DEF;
  config.pauseTime=docConfig["PAUSE"] | PAUSE_TIME;
  config.scrollSpeed=docConfig["SPEED"] | SCROLL_SPEED;
  config.intensity=docConfig["INTENSITY"] | 2;
  config.SEC=docConfig["SEC"] | true;
  config.HOR=docConfig["HOR"] | true;
  config.LUM=docConfig["LUM"] | true;
  config.REV=docConfig["REV"] | false;
  config.DHT=docConfig["DDHT"] | false;
  config.tempDDHT=docConfig["TEMPDDHT"] | TEMPODDHT;
  config.timeREV[0]=docConfig["TIMEREV"][0] | 7;
  config.timeREV[1]=docConfig["TIMEREV"][1] | 0;
  config.alDay[0]=docConfig["ALDAY"][0] | false;
  config.alDay[1]=docConfig["ALDAY"][1] | false;
  config.alDay[2]=docConfig["ALDAY"][2] | true;
  config.alDay[3]=docConfig["ALDAY"][3] | true;
  config.alDay[4]=docConfig["ALDAY"][4] | true;
  config.alDay[5]=docConfig["ALDAY"][5] | true;
  config.alDay[6]=docConfig["ALDAY"][6] | true;
  config.alDay[7]=docConfig["ALDAY"][7] | false;
  config.charOff=docConfig["CHAROFF"] | ' ';
  config.timeAdjust=docConfig["TIMEADJUST"] | OFFSET_ADJUST;
  strlcpy(config.textnotif,docConfig["TEXTNOTIF"] | "Notif",sizeof(config.textnotif));
  config.ALN=docConfig["ALN"] | true;
  config.CrTime=docConfig["CRTIME"] | 5;
  config.fxCR=docConfig["CRFX"] | 0;
  config.fxAL=docConfig["ALFX"] | 0;
  config.fxSoundCR=docConfig["CRFXSOUND"] | 0;
  config.fxSoundAL=docConfig["ALFXSOUND"] | 0;
  config.fxint=docConfig["FXINT"] | 100;
  config.fxcolor=docConfig["FXCOLOR"] | 0;
  config.MP3Start=docConfig["MP3START"] | _MP3START;
  config.MP3Notif=docConfig["MP3NOTIF"] | _MP3NOTIF;

  config.volumeAudio=docConfig["VOLUME"] | VOLUME ;
  //config.typeLED=docConfig["TYPELED"] | LED_OUT ;

  config.LED=docConfig["LED"] | false;
  config.LEDINT=docConfig["LEDINT"]|BRIGHTNESS;
  config.color=docConfig["COLOR"]| 0;
  //config.btn1=docConfig["BTN1"] | BOUTON1;
  //config.btn2=docConfig["BTN2"] | BOUTON2;
  config.btnclic[1][1] = docConfig["btnclic"][0] | 1;
  config.btnclic[1][2] = docConfig["btnclic"][1] | 2;
  config.btnclic[1][3] = docConfig["btnclic"][2] | 3;
  config.btnclic[2][1] = docConfig["btnclic"][3] | 6;
  config.btnclic[2][2] = docConfig["btnclic"][4] | 7;
  config.btnclic[2][3] = docConfig["btnclic"][5] | 4;
  config.box=docConfig["BOX"] | false;
  strlcpy(config.URL_Action1, docConfig["URL_ACT1"] | "N/A", sizeof(config.URL_Action1));
  strlcpy(config.URL_Action2, docConfig["URL_ACT2"] | "N/A", sizeof(config.URL_Action2));
  strlcpy(config.URL_Action3, docConfig["URL_ACT3"] | "N/A", sizeof(config.URL_Action3));
  strlcpy(config.URL_Update, docConfig["URL_UPD"] | "N/A", sizeof(config.URL_Update));
  config.action[0] = docConfig["ACTION"][0] | 0;
  config.action[1] = docConfig["ACTION"][1] | 0;
  file.close();

} // fin fonction loadconfig

String createJson(sConfigSys  &config,bool flagCreate=false) {
  String json;

  //const size_t capacityConfig = 2*JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(41) + 1500;
  DynamicJsonDocument docConfig(capacityConfig);
  docConfig["HARDWARE"]=hardware;
  if (flagCreate) docConfig["CONFIG"]=true;
  else docConfig["CONFIG"]=config.config;
  docConfig["NOM"]=hardConfig.nom;
  docConfig["SEC"]=config.SEC;
  docConfig["HOR"]=config.HOR;
  docConfig["LUM"]=config.LUM;
  docConfig["REV"]=config.REV;
  docConfig["DDHT"]=config.DHT;
  docConfig["TEMPDDHT"]=config.tempDDHT;
  docConfig["MSGALARM"]=config.msgAlarme;
  docConfig["MSGMINUT"]=config.msgMinuteur;
  docConfig["HFLAG"]=config.hflag;
  JsonArray doctime = docConfig.createNestedArray("TIMEREV");
  doctime.add(config.timeREV[0]);
  doctime.add(config.timeREV[1]);
  docConfig["NTPSERVER"]=config.NTPSERVER;
  docConfig["SUFFIXEHOST"]=config.hostName;
  docConfig["TZOFFSET"]=hardConfig.Offset;
  docConfig["TIMEADJUST"]=config.timeAdjust;
  docConfig["IDNOTIF"]=idNotif;
  //docConfig["DST"]=hardConfig.DST;
  docConfig["TZNAME"]=hardConfig.TZname;
  docConfig["DEBUG"]=config.DEBUG;
  docConfig["SPEED"] =config.scrollSpeed;
  docConfig["PAUSE"] =config.pauseTime;
  docConfig["INTENSITY"]=config.intensity;
  docConfig["CHAROFF"]=config.charOff;
  docConfig["TEXTNOTIF"]=config.textnotif;
  docConfig["ALN"]=config.ALN;
  //info systeme
  docConfig["MEMOJSON"]=docConfig.memoryUsage();
  docConfig["VERSION"]=ver;
  docConfig["MAC"]=WiFi.macAddress();
  docConfig["RSSI"] = String(WiFi.RSSI())+" dBm";
  docConfig["SSID"] = WiFi.SSID();
  docConfig["CHANNEL"] = WiFi.channel();
  docConfig["BSSID"] =WiFi.BSSIDstr();
  docConfig["IP"] =WiFi.localIP().toString();
  docConfig["HOSTNAME"] = WiFi.hostname();
  docConfig["MDNS"]=mdnsName+".local";
  docConfig["DNS"] = WiFi.dnsIP().toString();
  docConfig["DNS2"] =  WiFi.dnsIP(1).toString();
  docConfig["PASSERELLE"] = WiFi.gatewayIP().toString();
  docConfig["MASQUE"] = WiFi.subnetMask().toString();
  //options
  docConfig["PHOTOCELL"] = _photocell;
  docConfig["TYPEAUDIO"] = hardConfig.typeAudio;
  docConfig["VOLUME"] = config.volumeAudio;
  docConfig["MP3START"]=config.MP3Start;
  docConfig["MP3NOTIF"]=config.MP3Notif;
  docConfig["TOTALMP3"]=totalMP3;
  docConfig["MP3_1"]=nMP3_1;
  docConfig["TYPELED"] = hardConfig.typeLED;
  docConfig["LED"]=config.LED;
  docConfig["LEDINT"]=config.LEDINT;
  docConfig["COLOR"]=config.color;
  //temps
  _upTime=now()-_startTime;
  docConfig["STARTTIME"]=_startTime;
  docConfig["UPTIME"]=_upTime;
  docConfig["LASTSYNCHRO"]=_lastSynchro;
  docConfig["DATE"]=now();
  //DHT
  GetTemp();
  docConfig["DHT"] = _dht;
  docConfig["TEMP"]= temperature;
  docConfig["HUM"]= humidity;
  docConfig["HI"]=heatIndex;
  docConfig["ROSE"]=dewPoint;
  docConfig["PER"]=per;
  docConfig["STAMPDHT"]=stamp_DHT;
  docConfig["DHTMODEL"]=Modele;
  docConfig["DHTSTATUS"]= dht.getStatusString();
//minuteur
  docConfig["CR"]=CR;
  docConfig["CRSTOP"]=CRStop;
  docConfig["CRTIME"]=config.CrTime;

  //FX
  docConfig["FXINT"]=config.fxint;
  docConfig["FXCOLOR"]=config.fxcolor;
  docConfig["CRFX"]=config.fxCR;
  docConfig["ALFX"]=config.fxAL;
  docConfig["CRFXSOUND"]=config.fxSoundCR;
  docConfig["ALFXSOUND"]=config.fxSoundAL;
  //MQTT
  docConfig["BROKER"]=config.broker;
  docConfig["SRVBROKER"]=config.servbroker;
  docConfig["UBROKER"]=config.userbroker;
  docConfig["PBROKER"]=config.passbroker;
  docConfig["PORTBROKER"]=config.portbroker;
  docConfig["TOPIC"]=topicName+TOPIC_NOTIF;
  docConfig["TOPICSTATE"]=topicName+TOPIC_STATE;
  docConfig["TOPICOPT"]=topicName+TOPIC_OPTIONS;
  docConfig["TEMPOBROKER"]=config.tempobroker;
  docConfig["STATEBROKER"]=statebroker;
  docConfig["PREFIXHA"]=config.prefixdiscovery;
  //box
  docConfig["INFO"]=infoBOX;
  docConfig["BOX"]=config.box;
  docConfig["URL_ACT1"]=config.URL_Action1;
  docConfig["URL_ACT2"]=config.URL_Action2;
  docConfig["URL_ACT3"]=config.URL_Action3;
  docConfig["URL_UPD"]=config.URL_Update;
  JsonArray action = docConfig.createNestedArray("ACTION");
  action.add(config.action[0]);
  action.add(config.action[1]);
  //boutons
  docConfig["BTN1"]=hardConfig.btn1;
  docConfig["BTN2"]=hardConfig.btn2;
    JsonArray btnclic = docConfig.createNestedArray("btnclic");
    btnclic.add(config.btnclic[1][1]);
    btnclic.add(config.btnclic[1][2]);
    btnclic.add(config.btnclic[1][3]);
    btnclic.add(config.btnclic[2][1]);
    btnclic.add(config.btnclic[2][2]);
    btnclic.add(config.btnclic[2][3]);
  // alarme jours
  JsonArray aday = docConfig.createNestedArray("ALDAY");
    aday.add(config.alDay[0]);
    aday.add(config.alDay[1]);
    aday.add(config.alDay[2]);
    aday.add(config.alDay[3]);
    aday.add(config.alDay[4]);
    aday.add(config.alDay[5]);
    aday.add(config.alDay[6]);
    aday.add(config.alDay[7]);
  //donnee hardConfig
  docConfig["NETCODE"]=netcode;
  docConfig["NETOK"]=netOK;
  docConfig["NTPOK"]=ntpOK;
  docConfig["TYPEMATRICE"]=String(HARDWARE_TYPE);
  docConfig["XL"]=hardConfig.XL;
  docConfig["MAXDISPLAY"]=hardConfig.maxDisplay;
  docConfig["MAXZONEMSG"]=hardConfig.maxZonesMsg;
  docConfig["ZONETIME"]=hardConfig.zoneTime;
  docConfig["TOTALZONE"]=_maxZones;
  docConfig["PERSO"]=hardConfig.perso;
  docConfig["SETUP"]=hardConfig.setup;
  JsonArray docZP = docConfig.createNestedArray("ZP");
  for (int i=0;i<MAX;i++) {
  docZP.add(hardConfig.ZP[i]);
  }
  if (flagCreate) docConfig["CONFIG"]=true;
  // envoie config json
  serializeJsonPretty(docConfig,json);
  return json;
}

void saveConfigSys(const char *fileconfig,String json) {
  File  f = SPIFFS.open(fileconfig, "w");
  if (!f) {
    //InfoDebugtInit=InfoDebugtInit+" Fichier config Jeedom absent -";
    if (configSys.DEBUG) Serial.println("Fichier Config absent - création fichier");
    Serial.println("Fichier Config absent - création fichier");
   }
   f.print(json);  // sauvegarde de la chaine
   f.close();
}

//// Mesure DHT
void GetTemp() {
  int ModelDHT;
    ModelDHT=dht.getModel();
    temperature = dht.getTemperature();
    _dht=true;
    humidity= dht.getHumidity();
    heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    dewPoint = dht.computeDewPoint(temperature,humidity,false);
    per = dht.computePerception(temperature, humidity,true);
    stamp_DHT=now();

  if (isnan(humidity) || isnan(temperature)) {
    _dht=false;
    temperature = 0;
    humidity= 0;
    heatIndex = 0;
    dewPoint = 0;
    per = 0;
  }

   // Mise a jour des valeurs dht dans JSON
    //DHTsensor

    switch(ModelDHT) {
      case 1 : Modele="DHT11";
      break;
      case 2 : Modele="DHT22";
      break;
      default : Modele="Inconnu";
      break;
    }

  if (configSys.DEBUG) {
      Serial.println("Valeur Sensor DHT :"+String(_dht));
       Serial.println(" T:" + String(temperature) + "°C  H:" + String(humidity) + "%  I:" + String(heatIndex) + " D:" + String(dewPoint) + "  et per :"+ String(per));
  }
}

// fonction audio ( 0:none , 1 : Buzzer , 2 = MP3player , 4 = relais , 5 = sortie PIN digital)
void audio(char action='P')
{
switch (hardConfig.typeAudio) {
  case 1 : // Buzzer - HP
          if (notifAudio.fx>0)
          {
          //  String song= "AdamsFami:d=4,o=6,b=127:8c,f,8a,f,8c,b5,2g,8f,e,8g,e,8e5,a5,2f,8c,f,8a,f,8c,b5,2g,8f,e,8c,d,8e,1f,8c,8d,8e,8f,1p,8d,8e,8f_,8g,1p,8d,8e,8f_,8g,p,8d,8e,8f_,8g,p,c,8e,1f";
                notifAudio.state=true;

                rtttl::begin(AUDIOPINTX, buzMusic[notifAudio.fx]);
            //    rtttl::begin(AUDIOPINTX, song);
                rtttl::play();
             }
          else if (action=='S') {
            rtttl::stop();
            notifAudio.fx=0;
            notifAudio.state=false;
            Serial.println("fonction stop");
          }
      // fin 1
        break;
      case 2: // MP3 player
      {
       mySoftwareSerial.begin(9600);
       int vol;
        if (action=='P') {
          vol=floor(notifAudio.volume*30/100);
          Serial.println("Morceau : "+String(configSys.MP3Notif)+"  volume :"+String(vol));

          //myDFPlayer.begin(mySoftwareSerial);

          myDFPlayer.volume(vol);
          myDFPlayer.playFolder(1,configSys.MP3Notif);
          //myDFPlayer.playMp3Folder(2);
          //myDFPlayer.advertise(configSys.MP3Notif);
        }
        else if (action=='S') {
          myDFPlayer.pause();
        }
      }  // fin 2
      break;
  } // fin switch
  if (action=='P') AUDIONOTIF=true;
  else AUDIONOTIF=false;
}


void cmdLED(bool val,int intLED=configSys.LEDINT,byte C=configSys.color) {
  uint32_t color=couleur[C];
  if (intLED<=0 && hardConfig.typeLED !=2 ) val=false;
  switch (hardConfig.typeLED) {
    case 1 : // Led interne  - true = LOW
    {
      if (val) {
          intLED=constrain(intLED, 0, 100);
        //  configSys.LEDINT=intLED;
          intLED=1024-(round(intLED*1024/100));
      }
      else intLED=1024;
      analogWrite(LEDPIN,intLED);
    } // fin 1
    break;
    case 2 : // relais ou autre sortie ( true= High)
    {

      digitalWrite(LEDPIN,val);
    }
    break;
    case 3 : //Ring
{
  if (val) {
      ring.setBrightness(intLED);
      ring.fill(color);
      ring.show();
    }
    else { ring.clear(); ring.show();}

}
break;
} // fin switch
if (notifLed.fx==0 ) configSys.LED=val;
}

// Fonction LED **************
// ***************************
//
void flashLED(int Lo=3,int In=configSys.fxint,int Sp=50) {
  for (int i=0;i<Lo;i++) {
    cmdLED(true,In,notifLed.color);
    delay(Sp);
    cmdLED(false,0);
    delay(Sp);
      }
}

void fadeLED(int Lo=1,int In=configSys.fxint) {
for (int j=0;j<Lo;j++) {  // boucle loop
  for (int i=0;i<In;i=i+5) {
    cmdLED(true,i,notifLed.color);
    delay(40);
  }
  for (int i=In;i>0;i=i-5) {
    cmdLED(true,i,notifLed.color);
    delay(40);
  }
  cmdLED(false,0);
}
}

// fonction Ring qui allume les leds unes apres les autres de la même couleur
//void colorWipe(uint32_t color, int speed=50,bool single=false,int loops=1) {
void colorWipeNEO(byte C, int speed=50,bool single=false,int loops=1) {
uint32_t color = couleur[C];
for (int j=0;j<loops;j++) {
  for(int i=0; i<ring.numPixels(); i++) {
  if (single) ring.clear();
    ring.setPixelColor(i, color);
    ring.show();
    delay(speed);

  }
}
}

void chaseColorNEO(byte C, int speed=50,int loops=1) {
uint32_t color = couleur[C];
uint32_t black = ring.Color(0,0,0);
for (int j=0;j<loops;j++) {
  for(int i=0; i<ring.numPixels(); i++) {
    ring.setPixelColor(i, color);
    ring.show();
    delay(speed);
  }
  for(int i=0; i<ring.numPixels(); i++) {
    ring.setPixelColor(i, black);
    ring.show();
    delay(speed);
  }
}
}

void rainbowNEO(int rainbowLoops,int F=100,int speed=3, int S=255) {
  int fadeVal=0, fadeMax;
  fadeMax=constrain(F, 1, 100);
  for(uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops*65536L;
    firstPixelHue += 256) {

    for(int i=0; i<ring.numPixels(); i++) {
      uint32_t pixelHue = firstPixelHue + (i * 65536L / ring.numPixels());
      ring.setPixelColor(i, ring.gamma32(ring.ColorHSV(pixelHue, S,255* fadeVal / fadeMax )));
    }

    ring.show();
    delay(speed);

    if(firstPixelHue < 65536) {                              // Premiere boucle,
      if(fadeVal < fadeMax) fadeVal++;                       // fade in
    } else if(firstPixelHue >= ((rainbowLoops-1) * 65536)) { // Derniere boucle loop,
      if(fadeVal > 0) fadeVal--;                             // fade out
    } else {
      fadeVal = fadeMax; // boucle intermediaire fade=fademax
    }
  }
}
/*
void fxLED(int fx,int var1=configSys.LEDINT,int var2=50) {
  switch (configSys.typeLED) {
    case 1 : // Led interne  - true = LOW
      {
        if (fx==1) flashLED(3,var1,var2);
        else if (fx==2) fadeLED(1,var1);
    } // fin cas 1
  break;
    case 3 : // ring
    {
      if (fx==1) rainbow(2);
      else if (fx==2) colorWipe(Blue,var2,true);
      ring.clear();
      ring.show();
    }  // fin cas 3
  break;
} //fin switch
}
*/

void fxLED(byte c=configSys.fxcolor) {
  if (configSys.DEBUG) {
    Serial.println(" FXLED : valeur fx="+String(notifLed.fx));
    Serial.println(" Valeur typeLED ="+String(hardConfig.typeLED));
    Serial.println(" Valeur couleur ="+String(c));
  }
  if (hardConfig.typeLED==1) constrain(notifLed.fx, 1, 3);
  else if (hardConfig.typeLED==2) {notifLed.fx=1;notifLed.speed=100;}
  else if (hardConfig.typeLED==3) notifLed.color=c;
  else notifLed.fx=0;
  switch (notifLed.fx) {
    case 1 : cmdLED(true,100);
    break;
    case 2 : flashLED(notifLed.loop,notifLed.lum,notifLed.speed);
    break;
    case 3 : fadeLED(notifLed.loop,notifLed.lum);
    break;
    case 4 :
    ring.clear();
    ring.show();
    rainbowNEO(notifLed.loop,notifLed.lum);
    ring.clear();
    ring.show();
    break;
    case 5:
    //ring.clear();
    //ring.show();
     colorWipeNEO(c,notifLed.speed,true,notifLed.loop);
     ring.clear();
     ring.show();
    break;
    case 6: // colorWipe Full
     colorWipeNEO(c,notifLed.speed);
     ring.clear();
     ring.show();
    break;
    case 7: //  color chase
    chaseColorNEO(c,notifLed.speed,notifLed.loop);
    ring.clear();
    ring.show();
    break;
} //fin switch
if (notifLed.fx!=1) notifLed.fx=0;
}

void displayNotif(String Msg,int NZO=zoneMsg,byte type=0,textPosition_t pos=PA_LEFT, uint16_t S=configSys.scrollSpeed ,uint16_t P=configSys.pauseTime , byte fi=1,byte fo=1,char flag='D',byte An=0,int cycle=1) {
  if (configSys.DEBUG) {
      Serial.println("**********************");
      Serial.println("valeur Notification : ");
      Serial.println("Message : "+Msg);
      Serial.println("Zone : "+String(NZO)+" Type = "+String(type));
      Serial.println("Position: "+String(pos)+" Speed = "+String(S)+" Pause : "+String(P));
      Serial.println("Effet fxIn : "+String(fi)+"  fxOut : "+String(fo)+"  Anim : "+String(An));
      Serial.println("Flag : "+String(flag));
      Serial.println("Intensité : "+String(LuN));
      Serial.println("**********************");
    }


  char tabflag[11]={'D','F','F','A','A','A','X','Y','x','S','T'};
 // Verification Zones
 if (NZO >_maxZones-1 || _maxZones==xl) NZO=zoneMsg;
 // calcul largeur notif
 int maxLed,L,largeurMsg;
 largeurMsg=Msg.length();
 if (NZO==zoneTime) L=xl*5;
 else L=5;
 maxLed=ZWP[NZO]*8;
 if (largeurMsg>floor(maxLed/L) && configSys.ALN && type!=9 && type!=10) type=0;
 // adapte la pause au millieme
 P=P*1000;
 if (P==0) P=1;  // bug sur scroll si egale à zero
 //historique
 if (flag!='I') {
   if (type!=0) flag=tabflag[type];
 }
 if (checkFlag(flag)) addHisto(Msg,NZO,flag);
 //construction notif
 //transfert message
 Msg.toCharArray( Notification[NZO].Notif,BUF_SIZE);
 utf8Ascii(Notification[NZO].Notif);
  Notification[NZO].Alert=true;
  Notification[NZO].type=type;
  Notification[NZO].pos=pos;
  Notification[NZO].speed=S;
  Notification[NZO].pause=P;
  Notification[NZO].fxIn=fi;
  Notification[NZO].fxOut=fo;
  Notification[NZO].intensity=LuN;
  Notification[NZO].cycle=cycle;

  //if (configSys.typeLED>0 && notifLED>0) fxLED(1,notifLED);
  if (hardConfig.typeLED>0 && notifLed.fx>0 ) {nzofx=NZO;fxLED(notifLed.color);}
  if (hardConfig.typeAudio>0 && notifAudio.fx>0 ) {
      notifAudio.nzo=NZO;
      audio();
      delay(500);
    }
  //if (music>0 && configSys.typeAudio>0 ) audio('P',music);
  Serial.println("valeur du switch type :"+String(type));
  switch (type) {
    case 0: // type message scroll
    Serial.println("mode scroll");
     Notification[NZO].fxIn=1;
     Notification[NZO].fxOut=1;
     break;
    case 1: // type info
    Serial.println("mode info");
    //Notification[NZO].speed=40;
    if (P<1000) Notification[NZO].pause=1000;
    Notification[NZO].pos=PA_CENTER;
    Notification[NZO].fxIn=24;
    Notification[NZO].fxOut=4;
    break;
    case 2: // type fix
    Serial.println("mode fix");
    //Notification[NZO].speed=40;
    Notification[NZO].pause=0;
    Notification[NZO].pos=PA_CENTER;
    Notification[NZO].fxIn=0;
    Notification[NZO].fxOut=13;
    break;
    case 3: // animation Pac
    Serial.println("Animation pac man");
    Notification[NZO].speed=round((24/ZWP[NZO])*5+15);
    //Notification[NZO].pause=0;
    Notification[NZO].pos=PA_LEFT;
    Notification[NZO].fxIn=14;
    Notification[NZO].fxOut=14;
    Notification[NZO].AnIn=0;
    Notification[NZO].AnOut=13;
    break;
    case 4: // animation Pac
    Serial.println("Animation fleche");
    //P.setSpriteData(heart, W_HEART, F_HEART,heart, W_HEART, F_HEART);
    //Notification[NZO].speed=40;
    //Notification[NZO].pause=0;
    Notification[NZO].pos=PA_LEFT;
    Notification[NZO].fxIn=14;
    Notification[NZO].fxOut=14;
    Notification[NZO].AnIn=1;
    Notification[NZO].AnOut=14;
    break;
    case 5: // animation Pac
    Serial.println("Animation roll");

    //Notification[NZO].speed=60;
    //Notification[NZO].pause=0;
    Notification[NZO].pos=PA_LEFT;
    Notification[NZO].fxIn=14;
    Notification[NZO].fxOut=14;
    Notification[NZO].AnIn=2;
    Notification[NZO].AnOut=15;
    break;
    case 6: // fx perso
    Serial.println("fx perso");
    if (Notification[NZO].fxIn==10) Notification[NZO].speed=10;
    if (Notification[NZO].fxIn==11) Notification[NZO].speed=60;
    if (P<1000) Notification[NZO].pause=1000;  // minimum 1s pour voir les effets
    //Notification[NZO].pos=PA_CENTER;
    break;
    case 7: // fx perso
    Serial.println("animation perso");
    Notification[NZO].pos=PA_LEFT;
    Notification[NZO].fxIn=14;
    Notification[NZO].fxOut=14;
    Notification[NZO].AnIn=An;
    Notification[NZO].AnOut=An;
    break;
    case 9 : // type infoSys
    Serial.println("mode infoSys");
    Notification[NZO].speed=30;
    Notification[NZO].pause=1000;
    Notification[NZO].pos=PA_CENTER;
    Notification[NZO].fxIn=24;
    Notification[NZO].fxOut=4;
    break;
    case 10 : // affichage DHT
    Serial.println("mode Print dHT");
    Notification[NZO].speed=20;
    Notification[NZO].pause=3000;
    Notification[NZO].pos=PA_CENTER;
    Notification[NZO].fxIn=24;
    Notification[NZO].fxOut=15;
    break;
  }

}  // fin displaynotif

//lecture historique
void displayHisto() {
LuN=configSys.intensity;
String msgHisto;
char djh[5];
static int iH=0;
static uint32_t timerhist=0;
if( millis() - timerhist >= 60000) {
              iH = 0;
               timerhist = millis();
              }
time_t t = histNotif[iH].date;
sprintf(djh,"%02d:%02d",hour(t),minute(t));
msgHisto=String(iH+1)+"/"+String(indexHist)+"-(";
msgHisto+=String(djh)+") ";
msgHisto+=histNotif[iH].Notif;
displayNotif(msgHisto);
if (iH>=indexHist-1) iH=0;
else iH++;
}

// chaine decalage
void createHStringXL(char *pH, char *pL)
{

  for (; *pL != '\0'; pL++)  *pH++ = *pL | 0x80; // decalage caractére en positionnant le 8 bit ( 128) à 1
    *pH = '\0'; // termine la chaine
}



void createSecondes(char *Sec)
{

  for (; *Sec != '\0'; Sec++)  *Sec = *Sec + 32;   // decalage caractére de 32
    *Sec = '\0'; // termine la chaine
}

void displayTimer(char *psz,bool f) {
   int h,m,s;
      h  = minuteur / 60 / 60 % 24;
      m = minuteur / 60 % 60;
      s = minuteur % 60;
  char Sec[2];
  sprintf(Sec,"%02d",s);
  createSecondes(Sec);
    if (h>0) sprintf(psz,"%c %1d%c%02d%s",(CRStop ? 91 : 90 ),h,(f || CRStop ? ':' : ' '),m,Sec);
    else if (m>0) sprintf(psz,"%c %02d%s",(CRStop ? 91 : 90 ),m,Sec);
    else sprintf(psz,"%c %02d",(CRStop ? 91 : 90),s);

      // fin minuteur
       if (minuteur==0) {
        CR=false;
        notifAudio.fx=configSys.fxSoundCR;
        LuN=configSys.intensity;
        displayNotif(configSys.msgMinuteur,zoneTime);
        notifLed=FX_led[configSys.fxCR];
        if (configSys.box) BoutonAction(10 , configSys.action[0] );
       }
}

void displayClock() {
    if (hardConfig.XL) {
      P.setFont(zoneTime,numeric7SegXL);
      P.setFont(zoneXL_H,numeric7SegXL);
      //createHStringXL(msgH, msgL);  // fabrique la chaine Supérieur
      P.displayZoneText( zoneTime , msgL, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      P.displayZoneText( zoneXL_H , msgH, PA_CENTER, 0,0, PA_PRINT, PA_NO_EFFECT);
      P.synchZoneStart();
    }
    else {
      P.setFont(zoneTime,clockFont);
      P.displayZoneText( zoneTime, msgL, PA_CENTER, 0,0, PA_PRINT, PA_NO_EFFECT);
    }
}

void getFormatClock(char *psz, bool f = true)
// Code pour affichage time
{

  if (configSys.HOR) {
  if (zoneTime==zoneMsg) displayClock();
  char Sec[2];
  if ((hardConfig.zoneTime<5 && configSys.REV) || (hardConfig.zoneTime<5 && minuteur>0)) configSys.SEC=false;

  if (CR) displayTimer(psz,f);
  else {
    if (configSys.SEC) { sprintf(Sec,"%02d",second());
                        createSecondes(Sec);
                        }
  //sprintf(psz, "%02d%c%02d%s", timeClient.getHours(), (f ? ':' : ' '), timeClient.getMinutes(),Sec);
    sprintf(psz, "%c%02d%c%02d%s%c",(minuteur>0?(CRStop?92:93):' ') ,hour(), (f ? ':' : ' '), minute(),Sec,(configSys.REV?'.':' '));
    }
}
else sprintf(psz,"%c",configSys.charOff);

//sprintf(psz, "%02d%c%02d", timeClient.getHours(), (f ? ':' : ' '), timeClient.getMinutes());
}

void displayDHT() {
  String printDHT;
  LuN=configSys.intensity;
  GetTemp();
  printDHT=String(temperature,1)+" °C";
    displayNotif(printDHT,zoneTime,10);
}

void ToBox(char *Url,byte numero=0) {
  int httpCode;
  if (configSys.box)  {
    httpCode=sendURL(Url);
    String json="";
    infoBOX=String(numero)+":"+String(httpCode);
    json=createJson(configSys);
    saveConfigSys(fileconfig,json);
  }
}


int sendURL(char *Url) {
String url = Url;
int httpCode=0;
int testurl= url.length();
if (testurl>=6) {
    http.begin(url);
    httpCode = http.GET();
    if (configSys.DEBUG) {
          Serial.println("valeur de URL dans tobox : " + url);
          Serial.println("valeur httpcode : " + httpCode);
        }
  }
return httpCode;
}

// luminosite auto
// fonction reglage auto luminosite
int lumAuto() {
  static int cptTestCell=0;
  static int valTestCell=0;
  int sensorValue,lum;
  sensorValue = analogRead(0); // read analog input pin 0
  cptTestCell++;
  if (sensorValue)
  valTestCell +=sensorValue;
  if (configSys.DEBUG) {
      Serial.println("valeur photocell dans boucle auto : "+String(sensorValue));
      //Serial.println("valeur test "+String(valTestCell)+" boucle lum : "+String(cptTestCell));
    }
  if (cptTestCell>=6 ) {
    if (round(valTestCell/cptTestCell) <= 12)  _photocell=false;
    else _photocell=true;
    cptTestCell=0;
    valTestCell=0;
    }
    if (_photocell) {
          lum =round((sensorValue*1.5)/100);
          lum = constrain(lum,0,15);
    } else lum=configSys.intensity;
   return lum;
}

void cmdLum(bool val,byte I=configSys.intensity) {
  String msgInfo="";
  configSys.LUM=val;
  if (val) {
    lumAuto();
    msgInfo="Auto";
  } else {
    configSys.intensity=I;
    switch (I) {
      case 0 : msgInfo="Min";
      break;
      case 15 : msgInfo="Max";
      break;
      default:msgInfo="L : "+String(I);
      break;
    }
  }
  LuN=configSys.intensity;
  displayNotif(msgInfo,zoneMsg,9);
}

//websockets
/*
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  String s((const __FlashStringHelper*) payload);
    switch(type) {
        case WStype_DISCONNECTED:
          //  USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
              //  USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

				// send message to client
				webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:

          //  USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
        //    USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
        //    hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
		case WStype_ERROR:
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
    }

}
*/

//MDNS
void serverMDNS( String Nom_MDNS) {
  // Set up mDNS responder:
 if (!MDNS.begin(Nom_MDNS.c_str())) {
    if (configSys.DEBUG) Serial.println("Error setting up MDNS responder!");
  }

  if (configSys.DEBUG) Serial.println("mDNS responder started : "+Nom_MDNS);

  //mdns.register("fishtank", { description="Top Fishtank", service="http", port=80, location='Living Room' })
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  //MDNS.addService("ws", "tcp", 81);
  MDNS.addService("notifheure", "tcp", 8888); // Announce notifeur service port 8888 TCP
  //MDNS.addServiceTxt("notifheure","tcp", "nom", configSys.hostName);
}

String queryMdns() {
//  MDNSResponder::hMDNSServiceQuery               hMDNSServiceQuery       = 0;
  String ReponseMdns="MDNSreponse:";
  int n = MDNS.queryService("notifheure", "tcp"); // Send out query for esp tcp services

  //if (n==0) ReponseMdns+="0";
for (int i = 0; i < n; ++i) {
    ReponseMdns+= MDNS.IP(i).toString();
    ReponseMdns+=",";
}
int m = MDNS.queryService("notifeur", "tcp"); // Pour les anciennes versions
for (int i = 0; i < m; ++i) {
  ReponseMdns+= MDNS.IP(i).toString();
//  ReponseMdns+="_";
//  ReponseMdns+= MDNS.hostname(i);
 ReponseMdns+=",";
}
if (n==0 && m==0 ) ReponseMdns+="0";
return ReponseMdns;
}

// fonction Options
bool optionsBool( bool *pOpt, String val) {
  bool result=true;
  bool newval;
  if (configSys.DEBUG) Serial.println("Debug valeur bool ="+String(val));
  if (val == "true" || val =="1")   newval=true; //*pOpt=true;
  else if (val == "false" || val =="0")  newval=false;  //*pOpt=false;
  else result=false;
  if (newval != *pOpt) *pOpt=newval;
  else result=false;
  if (configSys.DEBUG)  Serial.println("valeur test = "+String(result));
  return result;
}
// pour objet type byte
void optionsNum(byte *pOpt, String val,int a , int b) {
  int v;
  v = val.toInt();
  *pOpt = constrain(v,a,b);
}
//pour Objet type int
void optionsNum(int *pOpt, String val,int a , int b) {
  int v;
  v = val.toInt();
  *pOpt = constrain(v,a,b);
}
bool validString(String val,int s ,int e) {
  if (val.length()>=s && val.length() <=e) return true;
  else return false;
}

// pour Decoupage chaine
void optionsSplit(byte *Opt,String Val,char split,int t=0) {
  Serial.println("valeur de function split :  "+Val);
  int  r=0;

   for (int i=0; i < Val.length(); i++)
   {
    if(Val.charAt(i) == split)
     {
       Opt[t] = Val.substring(r, i).toInt();
       r=(i+1);
     t++;
     }
   }
}

void optionsSplit(bool *Opt,String Val,char split,int t=0) {
  Serial.println("valeur de function split bool :  "+Val);
  int  r=0;
   for (int i=0; i < Val.length(); i++)
   {
    if(Val.charAt(i) == split)
     {
       if (Val.substring(r, i)=="true") Opt[t]=true;
       else Opt[t]=false;
       r=(i+1);
       t++;
     }
   }
}

void handleConfig() {
  String rep="Aucune modification";
  String key,info,value;
  int maxMsg;
  bool reboot=false,rst=false,test=false;
  int mem=0;
  for (int i = 0; i < server.args(); i++) {
        key=server.argName(i);
        key.toLowerCase();
        if (key=="xl") if (optionsBool(&hardConfig.XL,server.arg(i))) mem=2;
        if (key=="maxdisplay") {optionsNum(&hardConfig.maxDisplay,server.arg(i),4,80);mem=2;}
        if (key=="zonetime") {optionsNum(&hardConfig.zoneTime,server.arg(i),4,hardConfig.maxDisplay);mem=2;}
        if (key=="maxzonemsg") {
          maxMsg=floor(hardConfig.maxDisplay - hardConfig.zoneTime);
          optionsNum(&hardConfig.maxZonesMsg,server.arg(i),0,maxMsg);
          mem=2;
        }
        if (key=="perso") if (optionsBool(&hardConfig.perso,server.arg(i))) mem=2;
        if (key=="charoff") {configSys.charOff=server.arg(i).toInt(); mem=1;}
        if (key=="setup") if (optionsBool(&hardConfig.setup,server.arg(i))) mem=2;
        if (key=="zp") {optionsSplit(hardConfig.ZP,server.arg(i),',');mem=2;}
        if (key=="speed") {optionsNum(&configSys.scrollSpeed,server.arg(i),5,100);mem=1;}
        if (key=="pause") {optionsNum(&configSys.pauseTime,server.arg(i),0,180);mem=1;}
        if (key=="debug") if (optionsBool(&configSys.DEBUG,server.arg(i))) mem=1;
        if (key=="automsg") if (optionsBool(&configSys.ALN,server.arg(i))) mem=1;
        if (key=="box") if (optionsBool(&configSys.box,server.arg(i))) mem=1;
        if (key=="flag") if(validString(server.arg(i),0,10)) {
          value=server.arg(i);
            value.toCharArray(configSys.hflag,sizeof(configSys.hflag));
            mem=1;
        }
        if (key=="url1") if(validString(server.arg(i),7,130)) {
          value=server.arg(i);
            value.toCharArray(configSys.URL_Action1,sizeof(configSys.URL_Action1));
            mem=1;
        }
        if (key=="url2") if(validString(server.arg(i),7,130)) {
          value=server.arg(i);
            value.toCharArray(configSys.URL_Action2,sizeof(configSys.URL_Action2));
            mem=1;
        }
        if (key=="url3") if(validString(server.arg(i),7,130)) {
          value=server.arg(i);
            value.toCharArray(configSys.URL_Action3,sizeof(configSys.URL_Action3));
            mem=1;
        }
        if (key=="urlbox") if(validString(server.arg(i),7,130)) {
          value=server.arg(i);
            value.toCharArray(configSys.URL_Update,sizeof(configSys.URL_Update));
            mem=1;
        }
        if (key=="action") {optionsSplit(configSys.action,server.arg(i),',');mem=1;}
        if (key=="broker") if (optionsBool(&configSys.broker,server.arg(i))) {mem=1;reboot=true;}
        if (key=="ipbroker") if (validString(server.arg(i),8,50))
            {  value=server.arg(i);
              value.toCharArray(configSys.servbroker,sizeof(configSys.servbroker));
              mem=1;
            }
        if (key=="ubroker") if (validString(server.arg(i),1,20))
                  {  value=server.arg(i);
                    value.toCharArray(configSys.userbroker,sizeof(configSys.userbroker));
                    mem=1;

                  }
        if (key=="pbroker") if (validString(server.arg(i),1,20))
                  {  value=server.arg(i);
                    value.toCharArray(configSys.passbroker,sizeof(configSys.passbroker));
                    mem=1;
                  }
        if (key=="prefixha") if (validString(server.arg(i),1,20))
          {  value=server.arg(i);
            value.toCharArray(configSys.prefixdiscovery,sizeof(configSys.prefixdiscovery));
            mem=1;
          }
        if (key=="portbroker") {optionsNum(&configSys.portbroker,server.arg(i),1,66000);mem=1;}
        if (key=="tempobroker") {optionsNum(&configSys.tempobroker,server.arg(i),20,600);mem=1;}
        if (key=="tempoddht") {optionsNum(&configSys.tempDDHT,server.arg(i),1,120);mem=1;}
        if (key=="btn1") if (optionsBool(&hardConfig.btn1,server.arg(i))) mem=2;
        if (key=="clicbtn1") {optionsSplit(configSys.btnclic[1],server.arg(i),',',1);mem=1;}
        if (key=="btn2") if (optionsBool(&hardConfig.btn2,server.arg(i))) mem=2;
        if (key=="clicbtn2") {optionsSplit(configSys.btnclic[2],server.arg(i),',',1);mem=1;}
        if (key=="typeled") {optionsNum(&hardConfig.typeLED,server.arg(i),0,3);mem=2;}
        if (key=="fxint") {optionsNum(&configSys.fxint,server.arg(i),0,100);mem=1;}
        if (key=="intled") {optionsNum(&configSys.LEDINT,server.arg(i),0,100);mem=1;}
        if (key=="color") {optionsNum(&configSys.fxcolor,server.arg(i),0,7);mem=1;}
        if (key=="fxcr") {optionsNum(&configSys.fxCR,server.arg(i),0,5);mem=1;}
        if (key=="fxal") {optionsNum(&configSys.fxAL,server.arg(i),0,5);mem=1;}
        if (key=="aucr") {optionsNum(&configSys.fxSoundCR,server.arg(i),0,30);mem=1;}
        if (key=="aual") {optionsNum(&configSys.fxSoundAL,server.arg(i),0,30);mem=1;}
        if (key=="typeaudio") {optionsNum(&hardConfig.typeAudio,server.arg(i),0,4);mem=2;}
        if (key=="tzname") { value=server.arg(i);;
          value.toCharArray(hardConfig.TZname,sizeof(hardConfig.TZname));
          mem=2;
        }
        if (key=="tzoffset") { optionsNum(&hardConfig.Offset,server.arg(i),-780,+840);mem=2;}
        if (key=="ntpserver") if (validString(server.arg(i),4,50)) {
                    value=server.arg(i);
                    value.toCharArray(configSys.NTPSERVER,sizeof(configSys.NTPSERVER));
                    mem=1;
            }

    //    if (key=="dstvalue") { optionsNum(&hardConfig.DSTvalue,server.arg(i),0,180);mem=2;}
        if (key=="hostname") if (validString(server.arg(i),2,15)) {
                          value=server.arg(i);
                          value.toLowerCase();
                          value.toCharArray(configSys.hostName,sizeof(configSys.hostName));
                          //toLowerCase();
                          mem=1;
                        }
        if (key=="name")  if (validString(server.arg(i),3,20)) {
                            value=server.arg(i);
                            value.toLowerCase();
                            value.toCharArray(hardConfig.nom,sizeof(hardConfig.nom));
                            //toLowerCase();
                            mem=2;
                            }
        if (key=="crtext")   if (validString(server.arg(i),1,80)) {
                                            value=server.arg(i);
                                            value.toCharArray(configSys.msgMinuteur,sizeof(configSys.msgMinuteur));
                                            mem=1;
                                            }
        if (key=="altext")   if (validString(server.arg(i),1,80)) {
                                            value=server.arg(i);
                                            value.toCharArray(configSys.msgAlarme,sizeof(configSys.msgAlarme));
                                            mem=1;
                                            }
      if (key=="crtime") {optionsNum(&configSys.CrTime,server.arg(i),1,120);mem=1;}
        if (key=="checktime") {checkntp=true;rep="mise a jour horloge";displayNotif("upTime",zoneTime,2);}
        if (key=="checknet") {testnet();rep=String(netOK);}
        if (key=="mqttconfig") { MQTTconfig();rep="mqtt config";}

        if (key=="wifireset" && server.arg(i)=="true") {rep="Reset WIFI - reboot";rst=true;reboot=true;}
        if (key=="allreset" && server.arg(i)=="true") {rep="Reset usine - reboot -";readEConfig(true);rst=true;reboot=true;}
        if (key=="reboot" && server.arg(i)=="true") {rep="reboot en cours";reboot=true;}
        if (key=="setupreset" && server.arg(i)=="true") {readEConfig(true);rep="Reset configuration setup";reboot=true;}
        if (key=="purge" && server.arg(i)=="true") {SPIFFS.remove(fileHist);delay(100);rep=loadHisto(fileHist,histNotif);}

if (configSys.DEBUG) {
info = "Arg n"+ String(i) + "–>";
info += server.argName(i) + ": ";
info += server.arg(i) + "\n";
Serial.println(info);

                  }// fin debug

}
Serial.println("valeur switch mem : "+String(mem));
bool verif=true;
  switch (mem) {
    case 1 : {
        rep=createJson(configSys);
        saveConfigSys(fileconfig,rep);
        rep="OK - Modification config System ";
        if (reboot) rep+=" - REBOOT -";
        break;
      }
    case 2 :{
        if (configSys.DEBUG) debugEeprom(false);
        hardConfig.setup=true;
        if (hardConfig.XL && hardConfig.zoneTime*2 > hardConfig.maxDisplay) verif=false;
        if (!hardConfig.XL && hardConfig.zoneTime > hardConfig.maxDisplay) verif=false;
        if (verif) {
        writeEEPROM();
        if (configSys.DEBUG) debugEeprom();
        rep="OK - > Modification System -- Le Notifheure REBOOT -";
        reboot=true;
      } else  {
        reboot=false;
        rep="Erreur , les valeurs ne semblent pas coherentes";
      }
        break;
        }
  }

     server.send(200, "text/plane",rep);
     delay(2000);
     if (rst) wifiReset();
     if (reboot) ESP.restart();
}

String SetOptions(String key,String value) {
  String rep="No Options";
  upOpt=0;

    if (configSys.DEBUG) Serial.println("option : "+key+" - "+value);
    if (key=="LED") if (optionsBool(&configSys.LED,value)) {upOpt=1;rep="LED:"+String(configSys.LED);cmdLED(configSys.LED);}
    if (key=="COLOR") { optionsNum(&configSys.color,value,0,7);upOpt=1;rep="COLOR :"+String(configSys.color);cmdLED(configSys.LED);}
    if (key=="LEDINT") {
      optionsNum(&configSys.LEDINT,value,0,100);
      cmdLED(configSys.LED,configSys.LEDINT);
      rep="LEDINT : "+String(configSys.LEDINT);
      upOpt=1;
    }
    if (key=="DDHT") if (optionsBool(&configSys.DHT,value)) {upOpt=1;rep="DHT:"+String(configSys.DHT);}
    if (key=="REV") if (optionsBool(&configSys.REV,value)) {upOpt=1;rep="REV:"+String(configSys.REV);}
    if (key=="SEC") if (optionsBool(&configSys.SEC,value)) {upOpt=1;rep="SEC:"+String(configSys.SEC);}
    if (key=="HOR") if (optionsBool(&configSys.HOR,value)) {upOpt=1;rep="HOR:"+String(configSys.HOR);}
    if (key=="LUM") if (optionsBool(&configSys.LUM,value)) {
        upOpt=1;
        cmdLum(configSys.LUM);
        rep="LUM : "+String(configSys.LUM)+" et INT: "+String(configSys.intensity);
      }
      if (key=="TIMEREV") {
        optionsSplit(configSys.timeREV,value+":",':');
        upOpt=1;
        configSys.REV=true;
        rep="TIMEREV :"+String(configSys.timeREV[0])+"-"+String(configSys.timeREV[1]);
      }
    if (key=="INT") {
      optionsNum(&configSys.intensity,value,0,15);
      cmdLum(false,configSys.intensity);
      upOpt=1;
      rep="INT:"+String(configSys.intensity);
    }
    if (key=="MIN") {
        optionsNum(&minuteur,value,0,35999);
        CR=true;
        rep="MIN:"+String(minuteur/60)+" mn";

        P.displayReset(zoneTime);
        P.displayClear(zoneTime);
      if (hardConfig.XL)  P.displayReset(zoneXL_H);
    }
    if (key=="CR") if (optionsBool(&CR,value)) {rep="CR:"+String(CR);upOpt=2;}
    if (key=="CRSTP") if (optionsBool(&CRStop,value)) {rep="CRSTOP:"+String(CRStop);}
    if (key=="ALD") {optionsSplit(configSys.alDay,value,',');rep="ALD:"+value;upOpt=1;}
    return rep;
}
void updateOptions() {
  if (upOpt==1) {
    String json="";
    json=createJson(configSys,true);
    saveConfigSys(fileconfig,json);
  }
  if ( upOpt==1 || upOpt==2 ) {
      if (configSys.broker) MQTTsend();
      if (configSys.box) sendBox=true;
    }
  upOpt=0;
}

void handleOptions() {
  String key,value,rep;
  for (int i = 0; i < server.args(); i++) {
    key=server.argName(i);
    key.toUpperCase();
    value=server.arg(i);
    rep=SetOptions(key,value);
  }
  if (upOpt>0) updateOptions();
  server.send(200,"text/plane",rep);
}

void handleGetInfo() {
  String rep;
  if ( server.hasArg("MDNS")) {
      rep=queryMdns();
    }
else if ( server.hasArg("HISTO")) {

  //   loadHisto(fileHist, histNotif,rep);
     rep=createHisto(histNotif);
     saveHisto(fileHist,rep);
}
  else {
  rep=createJson(configSys);
}
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json",rep);

}

void initNotif() {
  // initialisation variable Notification
  Nz=zoneMsg;
  Ntype=0;
  Npos=PA_LEFT;
  Sc=configSys.scrollSpeed ;
  Pa=configSys.pauseTime;
  Nflag='N';
  notifLed.fx=0;
  notifLed.lum=configSys.fxint;
  notifLed.loop=1;
  notifLed.color=configSys.color;
  notifLed.speed=50;
  notifAudio.fx=0;
  notifAudio.volume=configSys.volumeAudio;
  notifAudio.piste=configSys.MP3Notif;
  Fi=1;
  Fo=1;
  An=0;
  LuN=configSys.intensity;
  Cyc=1;
}

String prepNotif(String key,String val) {
  String rep="Erreur-NoMsg";
  //displayNotif(String Msg,int NZO=zoneMsg,byte type=0,textPosition_t pos=PA_LEFT, uint16_t S=configSys.scrollSpeed ,uint16_t P=configSys.pauseTime , byte fi=1,byte fo=1)
  key.toUpperCase();
  if (key=="MSG" && val!="") { rep=val;}
  if (key=="NZO" ) { Nz=val.toInt();}
  if (key=="AUDIO") {
    optionsNum(&notifAudio.volume,val,0,100);
    if (notifAudio.volume>0) notifAudio.fx=1;
  }
  if (key=="NUM") {
    if (hardConfig.typeAudio==1) {
            optionsNum(&notifAudio.fx,val,0,30);

          }
    else if (hardConfig.typeAudio==2) optionsNum(&notifAudio.piste,val,0,totalMP3);
  }
  if (key=="LEDFX") {
        optionsNum(&notifLed.fx,val,0,10);
        if (notifLed.fx==2) notifLed.loop=3;
      }
  if (key=="COLOR") { optionsNum(&notifLed.color,val,0,7);}
  if (key=="LOOP") { optionsNum(&notifLed.loop,val,1,10);}
  if (key=="LEDLUM") { optionsNum(&notifLed.lum,val,0,100);}
  if (key=="INTNOTIF" || key=="LUM") { optionsNum(&LuN,val,0,15);}
  if (key=="FLASH") notifLed=flash;
  if (key=="BREATH") notifLed=breath;

  if (key=="SPEED" ) { optionsNum(&Sc,val,10,100);}
  if (key=="PAUSE" ) { optionsNum(&Pa,val,0,180);}
  if (key=="FX" ) { optionsNum(&Fo,val,0,28);optionsNum(&Fi,val,0,28);optionsNum(&Ntype,"6",0,9);}
  if (key=="ANIM" ) { optionsNum(&An,val,0,15);optionsNum(&Ntype,"7",0,9);}
  if (key=="CYCLE") { optionsNum(&Cyc,val,0,1000);}
  if (key=="TYPE") {
          if (val=="INFO") val=1;
          else if (val=="FIX") val=2;
          else if (val=="PAC") val=3;
          else if (val=="ARROW") val=4;
          else if (val=="ROLL") val=5;
          optionsNum(&Ntype,val,0,9);
        }
  //if (key=="FIX") Ntype=2;
if (key=="FI" ) { optionsNum(&Fi,val,0,28);}
if (key=="FO" ) { optionsNum(&Fo,val,0,28);}
if (key=="FIO" ) { optionsNum(&Fo,val,0,28);optionsNum(&Fi,val,0,28);}
if (key=="PLUGIN" ) { Nflag='J';}
if (key=="IMPORTANT" ) { Nflag='I';}
  return rep;
}

void handleNotif() {
  String notif,rep,key,value;
  initNotif();
  rep="";
  notif="";

  //parcours argument http
  for (int i = 0; i < server.args(); i++) {
    key=server.argName(i);
    value=server.arg(i);
    rep=prepNotif(key,value);
    if (rep!="Erreur-NoMsg") notif=rep;
  }

  if (notif!="") {
    displayNotif(notif,Nz,Ntype,Npos,Sc,Pa,Fi,Fo,Nflag,An,Cyc);
    rep=notif;
    //rep+=": type="+String(Ntype)+" - fi = "+String(Fi)+" - Anim:"+String(An);
  }
  server.send(200, "text/plane","Message : "+rep);
}

void initPrintSystem() {
  P.begin(1);
  P.setZone(0,0,hardConfig.zoneTime-1);
  P.setFont(ExtASCII);
}

// Fonction serveur
// ************** OTA
void Ota(String hostname) {
  //******* OTA ***************
// Port defaults to 8266
// ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
ArduinoOTA.setHostname((const char *)hostname.c_str());

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) type = "sketch";
          else  type = "SPIFFS";
          if ((type)=="SPIFFS") SPIFFS.end();   // NOTE: demontage disque SPIFFS pour mise a jour over the Air
        initPrintSystem();
        if (hardConfig.zoneTime <5) {
          P.print("OTA");
          delay(500);
          P.print(type);
        }  else P.print("OTA "+type);
        delay(500);
     //Serial.println("Start updating " + type);
    });
  ArduinoOTA.onEnd([]() {
      P.print("Reboot ...");
     delay(500);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

    if (hardConfig.zoneTime <5) sprintf(Notification[1].Notif,"Up %u%%", (progress / (total / 100)));
    else sprintf(Notification[1].Notif,"Upload  %u%%", (progress / (total / 100)));
    P.print(Notification[1].Notif);
  });
  ArduinoOTA.onError([](ota_error_t error) {
      P.print("ER ...");
      delay(500);
      ESP.restart();
    // OTAerreur=true;
   //if (config.DEBUG) Serial.printf("Error[%u]: ", error);
  });
  ArduinoOTA.begin();
}
//******* FIN OTA ***************
// reset parametre wifi , relance AP
void wifiReset() {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  //delay(1000);
  //ESP.restart();
}
// callback fonction wifimanager
void configModeCallback (WiFiManager *myWifiManager) {
  Serial.println("Entrez dans mode AP");
  Serial.println(WiFi.softAPIP());
  P.print("AP mode");
  delay(1000);
  P.print(myWifiManager->getConfigPortalSSID());

  Serial.println(myWifiManager->getConfigPortalSSID());
}

void wifiMan() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  //set adresse ip static
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  //timeout de 3 mn sur AP
  wifiManager.setConfigPortalTimeout(150);
  WiFiManagerParameter custom_text("<p>Portail AP Notifheure XL</p>");
  wifiManager.addParameter(&custom_text);
  wifiManager.setAPCallback(configModeCallback);
  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  //wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
if(!wifiManager.autoConnect("AP-NotifXL")) {
          P.print("erreur AP");
            delay(200);
            //reset and try again, or maybe put it to deep sleep
            ESP.reset();
            delay(200);
      }

}

void checkTime() {
  byte cpttime=0;
  String infomsg="Ok";
  while(!timeClient.forceUpdate() && cpttime<TEMPONTP ) {
  cpttime++;
  }
if (cpttime>=TEMPONTP) {
      if (configSys.DEBUG) Serial.println("Erreur synchro NTP");
      ntpOK=false;
      infomsg="Erreur";
  }
    else {
    setTime(timeClient.getEpochTime());
    if ( _startTime < _refTime ) _startTime=now();
    _lastSynchro=now();
    ntpOK=true;
  }
  displayNotif(infomsg,zoneTime,9);
}


/*
void Audio_out() {
  switch (configSys.typeAudio) {
    case 1 : // Buzzer
    {
      // Buzzer
      pinMode(AUDIOPINTX, OUTPUT);
      digitalWrite(AUDIOPINTX, LOW);
      //noTone(AUDIOPINTX);
    }
    break;
    case 2 : // MP3PLAYER
    {
      // serial softawre (pour player )
//     mySoftwareSerial.begin(9600);
//     if (!myDFPlayer.begin(mySoftwareSerial,false)) {  //Utilisation de  softwareSerial pour communiquer
     Serial.println("Pb communication:");
     Serial.println("1.SVP verifier connexion serie!");
     Serial.println("2.SVP verifier SDcard !");
     while(true){
          delay(0); // Code to compatible with ESP8266 watch dog.
        }
     }
      Serial.println("DFPlayer Mini En ligne.");
      myDFPlayer.setTimeOut(500); // Définit un temps de time out sur la communication série à 500 ms

      //----Controle volume----
      int vol=configSys.volumeAudio*3;
      vol=constrain(vol,0,30);
      myDFPlayer.volume(vol);  //Monte le volume à 18 ( valeur de 0 à 30 )
    // ---- indique d'utiliser le player de carte SD interne
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

    // optionel , permet d'afficher quelques infos
    Serial.println(myDFPlayer.readFileCounts()); //Le nombre total de fichier mp3 sur la carte ( dossier inclus )
    Serial.println(myDFPlayer.readFileCountsInFolder(1)); // l'index courant
    // Joue le premier morceau de la liste
    myDFPlayer.playFolder(1,configSys.MP3Start);
    }
    break;
  }
}
*/

void Led_out() {
  if (configSys.DEBUG) Serial.println("setup led : "+String(hardConfig.typeLED));

  switch (hardConfig.typeLED) {
    case 1 : // Led interne
    {
    pinMode(LEDPIN,OUTPUT);
    digitalWrite(LEDPIN,!configSys.LED);
    }
    break;
    case 2 : // autres , relais
    {
    pinMode(LEDPIN,OUTPUT);
    digitalWrite(LEDPIN,configSys.LED);
    }
    case 3 : // ring neopixel
    {
      int Br;
      Br=map(configSys.fxint,0,100,0,255);
      ring.begin();           // INITIALIZE NeoPixel ring object (REQUIRED)
      ring.setBrightness(Br); // Set BRIGHTNESS to about 1/5 (max = 255)
      ring.show();            // Turn OFF all pixels ASAP
    }
    break;
  } // fin switch
}

void BoutonAction(byte btn , byte btnclic ) {
int actionClick=0;
int m=0;
static byte clicstate=1;
if (btn==10) actionClick=btnclic;
else if (btnclic < 4 ) actionClick=configSys.btnclic[btn][btnclic];
else if (btnclic>100)  actionClick=btnclic;
else actionClick=0;

switch (actionClick) {

    case 1: //  Affiche ou non les secondes
          configSys.SEC=!configSys.SEC;
          m=1;
      break;
      case 2: //ON / OFF horloge
         configSys.HOR=!configSys.HOR;
          m=1;
      break;
      case 3:
      if (clicstate==1) {
        cmdLum(false,0);
        }
        if (clicstate==2) {
          cmdLum(false,15);
        }
        if (clicstate==3) {
        cmdLum(true);
        }
          ++clicstate;
          m=1;
      break;
      case 4 :cmdLED(!configSys.LED);
      m=1;
  break;
      case 5 :  //affichage Historique
      if (indexHist>0) displayHisto();
      else {
        LuN=configSys.intensity;
        displayNotif("Aucun historique");
      }
      break;
      case 6 : // affichage / masque Minuteur
              CR=!CR;
              break;
      case 7 : //Lancer Minuteur
              minuteur = configSys.CrTime*60;
              CR=true;
      break;
      case 8 : //URL Action 1
                ToBox(configSys.URL_Action1,1);
      break;
      case 9 : //URL Action 1
                ToBox(configSys.URL_Action2,2);
      break;
      case 10 : //URL Action 1
                ToBox(configSys.URL_Action3,3);
      break;
    default:

      break;
  }
  if (m==1) {
if (configSys.broker) MQTTsend();
sendBox=true;
}
}

void testnet() {
  netcode=sendURL("http://byfeel.info/testxl/");
  if (netcode == 301 ) netOK=true;
  else netOK=false;
}

void infoSetup(int step,String txt="") {
P.print(txt);
notifLed.fx=step;
notifLed.lum=BRIGHTNESS;
if (hardConfig.typeLED==1)  {
  notifLed.loop=3;
}
else if (hardConfig.typeLED==3)  {
notifLed.fx=notifLed.fx+2;
}
else notifLed.fx=0;
if (notifLed.fx>0) fxLED(Blue);
}

void alarme() {
  // detection jour weelday()
  bool okDay=false;
  for (int i=1;i<8;i++) {
    if (configSys.alDay[i]==true && weekday()==i )  okDay=true;
  }
  // Si jour ok
  if (okDay==true) {
        Serial.println("Mode alarme");
        LuN=configSys.intensity;
        notifLed=FX_led[configSys.fxAL];
        notifAudio.fx=configSys.fxSoundAL;
        displayNotif(configSys.msgAlarme,zoneTime);
        if (configSys.box) BoutonAction(10 , configSys.action[1] );
      }
}

//mqtt
void MQTTsend() {
  //update info temp
      GetTemp();
  String t;
//  char topicS[80];
  char buffer[512];
  DynamicJsonDocument docMqtt(capacityConfig);
  if (_dht) {
  docMqtt["temperature"] = (String)temperature;
  docMqtt["humidity"]= (String)humidity;
  }
  docMqtt["sec"] = configSys.SEC;
  docMqtt["hor"] = configSys.HOR;
  docMqtt["lum"] = configSys.LUM;
  //docMqtt["rev"] = configSys.REV;
  //docMqtt["cr"] = CR;
  // Si led
  if (hardConfig.typeLED >0 ) {
    docMqtt["led"] =configSys.LED;
    if (configSys.LED)  docMqtt["ledState"] ="on";
      else docMqtt["ledState"] ="off";
    if (hardConfig.typeLED == 1 || hardConfig.typeLED == 3) {
    docMqtt["ledint"] =configSys.LEDINT;
    // home assistant brillance sur 8 bits
    int brightnessLed=configSys.LEDINT;
   brightnessLed = map(brightnessLed,0,100,0,255);
    docMqtt["brightnessLed"] =brightnessLed;
    }
  }
  Serial.println("envoie publication MQTT");
  //docMqtt["temperature"]=String(temperature);
  size_t n = serializeJson(docMqtt, buffer);
  t=topicName+String(TOPIC_STATE);
  Serial.println(t);
  //t.toCharArray(topicS,80);
  MQTTclient.publish(t.c_str(), buffer,n);
}


void MQTTcallback(char* topic, byte* payload, unsigned int length) {
String t=topic;
DynamicJsonDocument docMqtt(capacityConfig);
deserializeJson(docMqtt, payload, length);
String KeyValue,key,val;
if (configSys.DEBUG)   Serial.print("Message MQTT ["+t+"] ");
// notification
if (t.indexOf(TOPIC_NOTIF)>0) {
  int  r=0;
  int index=0;
  char split=';';
  if (docMqtt.containsKey("msg")) {
    String msg,opt;
    msg=docMqtt["msg"]|"";
    opt=docMqtt["opt"]|"";
    if (msg!="") {
          initNotif();
          if (opt!="") opt+=split;
          Serial.println("msg:"+msg+" opt:"+opt);
          for (int i=0; i < opt.length(); i++)
              {
                if(opt.charAt(i) == split)
                  {
                    KeyValue= opt.substring(r, i);
                    KeyValue+="=";
                    index=KeyValue.indexOf("=");
                    if (index>=0) {
                      key=KeyValue.substring(0,index);
                      val=KeyValue.substring(index+1,KeyValue.length()-1);
                      Serial.println("key = "+key+" val = "+val);
                      prepNotif(key,val);
                      }
                  r=(i+1);
                  }
                }  // fin for
                Nflag='Q';
                displayNotif(msg,Nz,Ntype,Npos,Sc,Pa,Fi,Fo,Nflag,An);
          }  // fin test message non vide
  } // fin clé message
} // fin topic messages
else if (t.indexOf(TOPIC_OPTIONS)>0) {
  JsonObject root = docMqtt.as<JsonObject>();
  for (JsonPair kv : root) {
      key=kv.key().c_str();
      key.toUpperCase();
      val=kv.value().as<char*>();
      if (key=="LEDINT" ) {
            int valnumber;
            valnumber = atoi(val.c_str());  
            valnumber = map(valnumber,0,255,0,100);
            val=String(valnumber);
      } 
      SetOptions(key,val);
  }
      if (upOpt>0) updateOptions();
      if (configSys.DEBUG) Serial.println("clé="+key+" - val="+String(val));
}
  /*
  payload[length] = '\0';
  String s = String((char*)payload);
  Serial.println("test s : "+s);
  /*for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }*/
}

void MQTTsouscription(String t) {
  char topicN[80];
  t.toCharArray(topicN,80);
  Serial.print("topic souscription : ");
  Serial.println(topicN);
  if (MQTTclient.subscribe(topicN,0)) {
      if (configSys.DEBUG) Serial.println("souscription ok");
  } else   if (configSys.DEBUG) Serial.println("Erreur souscription");
}

void MQTTconfig() {
  char topicN[80];
  String t="";
  String tt="";
  String nomHA="";
  //preparation json to send
  tt=String(configSys.prefixdiscovery)+"/sensor/"+idNotif;
  nomHA="notif_"+String(hardConfig.nom);
      char buffer[500];
      DynamicJsonDocument docMqtt(capacityMQTT);
      if (_dht) {
      // creation sensor
      // Creation capteur temperature  topicName+String(TOPIC_STATE)
      docMqtt["~"]=topicName;
      docMqtt["stat_t"] ="~"+String(TOPIC_STATE);
      // temperature
      docMqtt["name"] =nomHA+"_T";
      docMqtt["device_class"] ="temperature";
      docMqtt["val_tpl"] ="{{ value_json.temperature}}";
      docMqtt["unit_of_meas"] ="°C";
      serializeJson(docMqtt, buffer);
      t=tt+"_T"+"/config";
      t.toCharArray(topicN,80);
      MQTTclient.publish(topicN, buffer,true);
//    Humidity
      docMqtt["name"] =nomHA+"_H";
      docMqtt["device_class"] ="humidity";
      docMqtt["val_tpl"] ="{{ value_json.humidity}}";
      docMqtt["unit_of_meas"] ="%";
      serializeJson(docMqtt, buffer);
      t=tt+"_H"+"/config";
      t.toCharArray(topicN,80);
      MQTTclient.publish(topicN, buffer,true);
      }

  //  Switch
    docMqtt.clear();
      tt=String(configSys.prefixdiscovery)+"/switch/"+idNotif;
  //switch mqtt secondes
    docMqtt["~"]=topicName;
    docMqtt["stat_t"] ="~"+String(TOPIC_STATE);
    docMqtt["name"]=nomHA+"_sec";
    docMqtt["cmd_t"]="~"+String(TOPIC_OPTIONS);
    docMqtt["val_tpl"] ="{{ value_json.sec }}";
    docMqtt["stat_on"] =true;
    docMqtt["stat_off"] =false;
    docMqtt["pl_on"] ="{'sec':'true'}";
    docMqtt["pl_off"] ="{'sec':'false'}";
    size_t n = serializeJson(docMqtt, buffer);
    t=tt+"_sec"+"/config";
    t.toCharArray(topicN,80);
    MQTTclient.publish(topicN, buffer,n);

    //switch mqtt horloge
    docMqtt["name"]=nomHA+"_hor";
    docMqtt["val_tpl"] ="{{ value_json.hor }}";
    docMqtt["pl_on"] ="{'hor':'true'}";
    docMqtt["pl_off"] ="{'hor':'false'}";
    n = serializeJson(docMqtt, buffer);
    t=tt+"_hor"+"/config";
    t.toCharArray(topicN,80);
    MQTTclient.publish(topicN, buffer,n);

if ( _photocell) {
        //switch mqtt auto
    docMqtt["name"]=nomHA+"_lum";
    docMqtt["val_tpl"] ="{{ value_json.lum }}";
    docMqtt["pl_on"] ="{'lum':'true'}";
    docMqtt["pl_off"] ="{'lum':'false'}";
    n = serializeJson(docMqtt, buffer);
    t=tt+"_lum"+"/config";
    t.toCharArray(topicN,80);
    MQTTclient.publish(topicN, buffer,n);
  }
if (hardConfig.typeLED >0 ) {
  docMqtt.clear();
   tt=String(configSys.prefixdiscovery)+"/light/"+idNotif;
  docMqtt["~"]=topicName;
  docMqtt["stat_t"] ="~"+String(TOPIC_STATE);
  docMqtt["name"]=nomHA+"_led";
  docMqtt["schema"] = "template";
  docMqtt["cmd_t"]="~"+String(TOPIC_OPTIONS);
  docMqtt["stat_tpl"] ="{{ value_json.ledState }}";
  docMqtt["cmd_off_tpl"] ="{'led':'false'}";
  if (hardConfig.typeLED == 1 || hardConfig.typeLED == 3 ) {
  docMqtt["cmd_on_tpl"] ="{'led':'true'  {%- if brightness is defined -%},'ledint':'{{ brightness }}' {%- endif -%} }";
  docMqtt["bri_tpl"] ="{{ value_json.brightnessLed }}";
  }
  else docMqtt["cmd_on_tpl"] ="{'led':'true'}";
    n = serializeJson(docMqtt, buffer);
    t=tt+"_led"+"/config";
    t.toCharArray(topicN,80);
    MQTTclient.publish(topicN, buffer,n);
  }
  startSend=false;
}

boolean MQTTconnect() {
  String t="";
  //    Serial.print("Attente  MQTT connection...");
      String clientId = "NotifheureClient-";
      //clientId += String(random(0xffff), HEX);
      clientId += idNotif;
    // Attempt to connect
  if (MQTTclient.connect(clientId.c_str(),configSys.userbroker,configSys.passbroker)) {
      // connexion ok
        statebroker=true;

      //****************************************
      // Souscription pour reception des message
      //*****************************************
      // souscription notification
        t=topicName+String(TOPIC_NOTIF);
        MQTTsouscription(t);
      // souscription options
        t=topicName+String(TOPIC_OPTIONS);
        MQTTsouscription(t);
    // envoie de la config auto à la premiere connection
    if ( startSend) MQTTconfig();

    } else {  // si erreur connexion
        statebroker=false;
        if (configSys.DEBUG) {
            Serial.print("ECHEC, rc=");
            Serial.print(MQTTclient.state());
          }
    }
  return MQTTclient.connected();
}

// ****************************
/******************************
 ********* SETUP **************
 ******************************/
// ****************************

void setup() {
  // serial monitor pour debug
 Serial.begin(9600);
 delay(200);
 String infoSys="OK";
 // Eeprom lecture config
 readEConfig();
 // ******************** init screen --- Start


 initPrintSystem();
 infoSetup(0,"Start ...");

 // ******************* chargement donnée systémes
 // init spiffs
 SPIFFS.begin();
   // lecture fichier json dans memoire SPIFFS
   loadConfigSys(fileconfig, configSys);
   if (hardConfig.setup) _pagehtml="index.html";
   else _pagehtml="setup.html";
   //Activation led si presente
   Led_out();
 //****************** Etpae 5 ** Reseau WIFI
 infoSetup(3," 5");
// Connexion WIFI
  wifiMan();
//MakeName - creation hostname wifi
// Set Hostname.
   idNotif=String(ESP.getChipId(), HEX);;
   hostname = hardConfig.nom;
   hostname +=configSys.hostName;
   mdnsName = hostname;
   hostname +="-"+idNotif;
   WiFi.hostname(hostname);
   // preparation topic
   String l;
   l=hardConfig.nom;
   l.replace(" ", "_");   // enleve les espaces de la chaines
   topicName=BASETOP;
   topicName+="/"+hardware+"/"+idNotif+"/"+l;
//
// ************************ Etape 4 ** Service reseau
infoSetup(3," 4");
// Debug fsinfo
if (configSys.DEBUG) {
Serial.println(" Version Matrice : "+String(HARDWARE_TYPE)+" test FC16 "+String(MD_MAX72XX::FC16_HW) );
Serial.println(msgDebug);
 //info SPIFFS
 FSInfo fsInfo;
// info fs
SPIFFS.info(fsInfo);
//fsInfo.totalBytes
float total=(fsInfo.totalBytes/1024);
Serial.println("info totalbytes : "+String(total));
Serial.println("info usedbytes : "+String(fsInfo.usedBytes));
Serial.println("info blocksize : "+String(fsInfo.blockSize));
Serial.println("info pagesize: "+String(fsInfo.pageSize));
Serial.println("info maxOpenFiles : "+String(fsInfo.maxOpenFiles));
Serial.println("info maxPathLength : "+String(fsInfo.maxPathLength));
}
//************** Services MDNS
mdnsName.toLowerCase();
serverMDNS(mdnsName);
 // Serveur OTA
Ota(hostname);
//Init MQTT
if (configSys.broker) {
  MQTTclient.setServer(configSys.servbroker, configSys.portbroker);
  MQTTclient.setCallback(MQTTcallback);
  // buffer mqtt
  MQTTclient.setBufferSize(1024);
}
//serveur WEB
//server.on("/Notification",handleNotif); //Gestion des Notifications
 server.on("/Notification",handleNotif); //Gestion des Notifications
 server.on("/Config",handleConfig); //page gestion Options
 server.on("/Options",handleOptions); //page gestion Options
 server.on("/getInfo",handleGetInfo); //page gestion Options
 // Gestion securisé page config
 server.on("/setup.html", []() {
      if (!server.authenticate(www_username, www_password)) {
     return server.requestAuthentication();
   }
      if (!handleFileRead("/setup.html")) {
     server.send(404, "text/plain", "FileNotFound");
   }
    });
    // server upload file
server.on("/Upload", HTTP_GET, []() {
      if (!server.authenticate(www_username, www_password)) {
        return server.requestAuthentication();
      }
      if (!handleFileRead("/upload.html")) {
        server.send(404, "text/plain", "FileNotFound");

      }
    });

server.on("/Upload", HTTP_POST, []() {
      server.send ( 200, "text/html", getPage());
    }, handleFileUpload);
    server.onNotFound([]() {
      if (!handleFileRead(server.uri())) {
        server.send(404, "text/plain", "FileNotFound");
      }
    });
server.onNotFound([]() {
  if (!handleFileRead(server.uri())) {
    server.send(404, "text/plain", "FileNotFound");
  }
});
 server.begin();

//WebSocketsServer
//webSocket.begin();
//webSocket.onEvent(webSocketEvent);
//***************** Etape 3  *******
//options Audio / LED / DHT
// config Sortie AUDIO Si present
  infoSetup(3," 3");
  switch (hardConfig.typeAudio) {
    case 1 : // Buzzer
    {
      // Buzzer
      pinMode(AUDIOPINTX, OUTPUT);
      digitalWrite(AUDIOPINTX, LOW);
      //noTone(AUDIOPINTX);
    }
    break;
    case 2 : // MP3PLAYER
    {

      // serial softawre (pour player )
     mySoftwareSerial.begin(9600);
     if (!myDFPlayer.begin(mySoftwareSerial,false)) {  //Utilisation de  softwareSerial pour communiquer

     Serial.println("Pb communication:");
     Serial.println("1.SVP verifier connexion serie!");
     Serial.println("2.SVP verifier SDcard !");
     while(true){
          delay(0); // Code to compatible with ESP8266 watch dog.
        }
     }
      //Serial.println("DFPlayer Mini En ligne.");
      myDFPlayer.setTimeOut(500); // Définit un temps de time out sur la communication série à 500 ms
      infoSetup(3," 3..");
      //----Controle volume----
      int vol=floor(configSys.volumeAudio*30/100);
      vol=constrain(vol,0,30);
      myDFPlayer.volume(vol);  //Monte le volume à 12 ( valeur de 0 à 30 )
    // ---- indique d'utiliser le player de carte SD interne
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

    // optionel , permet d'afficher quelques infos
    totalMP3=myDFPlayer.readFileCounts(); //Le nombre total de fichier mp3 sur la carte ( dossier inclus )
    nMP3_1=myDFPlayer.readFileCountsInFolder(1); // dossier 1 , nombre fichier
    // Joue le premier morceau de la liste
    myDFPlayer.playFolder(1,configSys.MP3Start);
    }
    break;
  }
  //*************dHT
  // init options
  // Initialize temperature sensor
   int pin(dhtpin);
   dht.setup(pin,DHTTYPE);
   delay(200);

   //init variable Audio
   notifAudio={0,_MP3START,VOLUME,zoneMsg,false};

   //***************** Etape 2  *******
   //  TIME
     infoSetup(3," 2");
 // recuperation info temps
  timeClient.setPoolServerName(configSys.NTPSERVER);
  timeClient.begin();
   bool s=true;
   String MsgInfo;
   byte cpttime=0;
   ntpOK=true;
while(!timeClient.update() && cpttime<TEMPONTP) {
  // timeClient.forceUpdate();
   MsgInfo=(s ? " 2" : " 2 .");
   infoSetup(3,MsgInfo);
   s=!s;
   cpttime++;
   if (configSys.DEBUG) Serial.print(String(cpttime)+" - ");
 }
if (cpttime>=TEMPONTP) {
  if (configSys.DEBUG) Serial.println("Erreur synchro serveur temps");
infoSetup(2," 2");
infoSys="Erreur synchro Horloge - ";
//checkTime();
ntpOK=false;
}
if (ntpOK) {
 timeClient.setTimeOffset(hardConfig.Offset*60);
 setTime(timeClient.getEpochTime());
 _startTime=now();
 _lastSynchro=_startTime;

if (configSys.DEBUG) Serial.println("time ok :"+String(now())+" offset :"+String(hardConfig.Offset)+" minutes");
}
// historique
loadHisto(fileHist,histNotif );

//***************** Etape 1  *******
//  Configuration Matrices
  infoSetup(3," 1");
  //ZONES
 // init variable  - Assignation zone
 _spaceChar = 1;
// variable par defaut pour calcul des Zones
int Start,End,WZN;
// verification si Zone ok
if (hardConfig.maxDisplay<hardConfig.zoneTime) {
    hardConfig.zoneTime=hardConfig.maxDisplay;
    hardConfig.XL=false;
  }
if (hardConfig.XL) {
        ZonesWide[0]= hardConfig.zoneTime;
        ZonesWide[1]= hardConfig.zoneTime;
        xl=2;
    }
else {
    ZonesWide[0]= hardConfig.zoneTime;
    xl=1;
}
if (configSys.DEBUG) Serial.println("valeur xl = "+String(xl));
_maxZones=xl;

_maxDisplayMsg=hardConfig.maxDisplay-xl*hardConfig.zoneTime;
Serial.println("Display dispo pour notif : "+String(_maxDisplayMsg));
if (_maxDisplayMsg > 0 && hardConfig.maxZonesMsg>0) {
    WZN = floor(_maxDisplayMsg/hardConfig.maxZonesMsg );
    for (int i=xl;i<hardConfig.maxZonesMsg+xl-1;i++)
        {
     ZonesWide[i]=WZN;

     _maxZones++;
     Serial.println("Zone dans boucle index : "+String(i));
      }
  // la derniére zone prend ce qu'il reste
  if (hardConfig.maxZonesMsg>=1)  {
    ZonesWide[_maxZones]=hardConfig.maxDisplay-(hardConfig.maxZonesMsg-1)*WZN-xl*( hardConfig.zoneTime);
     _maxZones++;
     Serial.println("Zone finale "+String(_maxZones)+" taille = "+String(ZonesWide[_maxZones-1]));
    }
  }

  //Largeur des zones enregistré par défaut
  for (int i=0;i<MAX;i++)
  {
    Zones[i]=i;
    ZWP[i]=ZonesWide[i];
    Serial.println("Zone "+String(i+1)+" taille = "+String(ZonesWide[i]));
  }
  Serial.println("Nombre Zones à créé  ="+String(_maxZones));
// Zones perso si existe
if (hardConfig.perso) {
  Serial.println("mode perso");
  for (int i=0;i<MAX;i++) {
    Zones[i]=hardConfig.ZP[i];
     Serial.println("zones perso "+String(Zones[i]));
  }
}
// ajustement des zones avant création
  zoneTime=Zones[Time_Lo];
  zoneXL_L=Zones[Time_Lo];
  zoneXL_H=Zones[Time_Up];
  if (hardConfig.XL) zoneMsg=Zones[ZN_1];
  else zoneMsg=Zones[Time_Up];
  // test zones
 Serial.println("zone time "+String( zoneTime));
 Serial.println("zone XL L "+String( zoneXL_L));
 Serial.println("zone XL H "+String( zoneXL_H));
 Serial.println("zone msg "+String( zoneMsg));
 //test acces internet
testnet();
//****************************
// Etape finale
// ajustement matrices
infoSetup(3," 0");
// Selon modele des matrices - inversion de l'affichage ( true si Generic ou Parola )
invertUpperZone = (HARDWARE_TYPE == MD_MAX72XX::GENERIC_HW || HARDWARE_TYPE == MD_MAX72XX::PAROLA_HW);
//init Zones
for (int i=0;i<MAX;i++) {
  ZWP[i]=ZonesWide[Zones[i]];
}
// initialisation des Zones pour affichage zones et horloge
  P.begin(_maxZones);
  configSys.intensity=lumAuto();
  P.setIntensity(configSys.intensity);
  if (_maxZones==1) {
      P.setZone(zoneXL_L , 0, hardConfig.maxDisplay-1);
  }
  else {
    //Largeur zone perso enregistré
  for (int i=0;i<7;i++)
  {
     if (configSys.DEBUG) Serial.println("Zone Perso creation "+String(i)+" taille = "+String(ZWP[i]));
  }
  //
  Start=0;
  for (int i=0;i<_maxZones;i++)
      {
    End=Start+ZWP[i];
    if (configSys.DEBUG) Serial.println("Creation Zone : "+String(i)+" de "+String(Start)+" a "+String(End));
    P.setZone( i, Start, End - 1);
    Start=End;
    }

  }
  uint16_t S,E;
  // verification Zone
  if (configSys.DEBUG) {
  for (int j=0;j<_maxZones;j++)
  {
  P.getDisplayExtent(j,S,E);
  Serial.println("Zone : "+String(j)+" colonne : "+String(S)+" à "+String(E));
  }
  }
  // police de caractére à afficher
  //P.setFont(numeric7SegXL);
  P.setCharSpacing(_spaceChar); // espacement caractére
  // Selon type module inversion ou non de la zone supérieur
  if (invertUpperZone && hardConfig.XL)
  {
    P.setZoneEffect(zoneXL_L , true, PA_FLIP_UD);
    P.setZoneEffect(zoneXL_H , true, PA_FLIP_LR);
  }
  /*
   * inversion
  P.setZoneEffect(zoneXL_L , true, PA_FLIP_LR);
  P.setZoneEffect(zoneXL_L , true, PA_FLIP_UD);
  P.setZoneEffect(zoneXL_H , true, PA_FLIP_UD);
  P.setZoneEffect(zoneXL_H, true, PA_FLIP_LR);
  zoneTime=Zones[Time_Up];
  zoneXL_L=Zones[Time_Up];
  zoneXL_H=Zones[Time_Lo];
*/
//animation par defaut PAC
P.setSpriteData(pacman1, W_PMAN1, F_PMAN1,pacman2, W_PMAN2, F_PMAN2);   // chargement animation en memoire

//prepa zone horloge
configSys.intensity=lumAuto();
LuN=configSys.intensity;
if (_maxZones==xl) zoneMsg=zoneTime;
displayClock();
//fin setup
infoSys +=" IP : ";
infoSys += WiFi.localIP().toString();
displayNotif(infoSys);
//lecture temp et init capteur
GetTemp();
}
//********* Fin SETUP


void finNotif() {
  if (AUDIONOTIF) {
    audio('S');
    Serial.println("fin music");
  }
  cmdLED(configSys.LED);
}




void loop() {
  static bool XLZoneTest=true;
  static bool disClock=false;
  static uint32_t  lastTime = 0; // millis() memory
  static uint32_t  lastTimeLumAuto = 0; // millis() memory
  static uint32_t  lastTimePrintDHT= 0; // millis() system / synchro
  static uint32_t  lastTimeMqtt= 0; // millis() system / synchro
  static uint32_t  tempoBox= 0; // tempo pour envoie update vers box
  static uint32_t  tempoCheck= 0; // tempo pour envoie update vers box
  static bool flasher = false;  // seconds passing flasher
  static uint32_t  startAudio;
  static bool dispNotif=false;
  static bool dispNotiffx=false;
   //  ****** Page WEb :   traite les requetes http et ws
  server.handleClient();
  //webSocket.loop();  // ecoute websocket
  // if (configSys.broker) MQTTclient.loop();  // ecoute mqtt client
  ArduinoOTA.handle(); // ecoute OTA

if (ntpOK) {
    if (timeClient.update() && (_lastSynchro > _refTime))
    {
    _lastSynchro=now();
    }
}
  //MDNS.update();

  //tmpo
   if (millis() - lastTime >= 1000)
    {
      lastTime = millis();
      flasher = !flasher;
      if (minuteur>0 && !CRStop) {
              minuteur--;
              if (minuteur>=0 && minuteur<5) CR=true;
     }

    }
    if (millis() - lastTimeLumAuto >= 20000)
     {
       lastTimeLumAuto = millis();
       if (configSys.LUM) configSys.intensity=lumAuto();
     }
     if (millis() - lastTimePrintDHT >= (configSys.tempDDHT*60*1000))
      {
        lastTimePrintDHT = millis();
        if (configSys.DHT) displayDHT();
      }

     /*
  if (millis() - lastTimeSystem >= synchroNTP+500)
     {
       lastTimeSystem =millis();
       checkTime();
     }
     */
     // ***********  Gestion bouton
   if (hardConfig.btn1) {
     // etat bouton
      bouton1.Update();
      // Sauvegarde de la valeur dans la variable click
     if (bouton1.clicks != 0) {
       clic = bouton1.clicks;
        if (configSys.DEBUG) Serial.println("Valeur bouton 1 ="+String(configSys.btn1)+" , Valeur du clic ="+String(clic));
     BoutonAction(1,clic);
     clic=0;
     }
   }
   if (hardConfig.btn2) {
     // etat bouton
      bouton2.Update();
      // Sauvegarde de la valeur dans la variable click
     if (bouton2.clicks != 0) {
       clic = bouton2.clicks;
        if (configSys.DEBUG) Serial.println("Valeur bouton 2 ="+String(configSys.btn2)+" , Valeur du clic ="+String(clic));
     BoutonAction(2,clic);
     clic=0;
     }
   }

//********************************


//********************************
// Affichage display *************
// *******************************
P.displayAnimate();
 //*************************
 // gestion des zones
 //*************************
 for (int i=0;i<_maxZones;i++) {
   //test si zone xl et zone haute
   if (!hardConfig.XL || i!=zoneXL_H ) {
  // debut traitement zone libre
   if (P.getZoneStatus(i)) {
      if (!Notification[i].Alert)
            {
           if (i==zoneTime) {
             //**********************
             // Mode affichage horloge
             //**********************
             dispNotif=false;
           // affichage horloge si different de fix
           if (Notification[zoneTime].type!=2 ) {
           P.setIntensity(zoneTime,configSys.intensity);
           if (hardConfig.XL) P.setIntensity(zoneXL_H,configSys.intensity);
           displayClock();
           getFormatClock(msgL, flasher);
           if (hardConfig.XL) createHStringXL(msgH, msgL);
          }
        } // fin test zone horloge
      } // fin display horloge
else
    {
      // affichage des notifications
      Serial.println("notif boucle zone ="+String(i)+" msg ="+String(Notification[i].Notif));
      dispNotif=true;
      if (Notification[i].fxIn==14 || Notification[i].fxOut==14) {
            P.setSpriteData(sprite[Notification[i].AnIn].data, sprite[Notification[i].AnIn].width, sprite[Notification[i].AnIn].frames,  //intro sprite
              sprite[Notification[i].AnOut].data, sprite[Notification[i].AnOut].width, sprite[Notification[i].AnOut].frames); // exit sprite
            }
      P.displayClear(i);
      P.setIntensity(i,Notification[i].intensity);
    // Si mode XL
    if (hardConfig.XL && i==zoneTime) {
        Serial.println("mode XL");
        //prepa zone superieur
        P.displayClear(zoneXL_H);
        P.setIntensity(zoneXL_H,Notification[zoneTime].intensity);
        // Chargement police Double
        P.setFont(zoneXL_L, BigFontLower);
        P.setFont(zoneXL_H, BigFontUpper);
        //chargement et preparation de l'affichage notification
        if (invertUpperZone)
          {
            textPosition_t position;
            P.displayZoneText(zoneXL_L, Notification[zoneTime].Notif, Notification[zoneTime].pos, Notification[zoneTime].speed, Notification[zoneTime].pause, effect[Notification[zoneTime].fxIn],effect[Notification[zoneTime].fxIn]);
            if (Notification[zoneTime].pos==PA_LEFT) position=PA_RIGHT;
            else position=PA_LEFT;
            P.displayZoneText(zoneXL_H,Notification[zoneTime].Notif, position, Notification[zoneTime].speed,Notification[zoneTime].pause, effect[28-Notification[zoneTime].fxIn], effect[28-Notification[zoneTime].fxIn]);
          }
          else
          {
            byte fi,fo;
            //inversion effet pour type INFO
            if (Notification[zoneTime].type==1 ) {
                fi=Notification[zoneTime].fxOut;
                fo=Notification[zoneTime].fxIn;
            } else {
                fo=Notification[zoneTime].fxOut;
                fi=Notification[zoneTime].fxIn;
            }
           P.displayZoneText(zoneXL_H,Notification[zoneTime].Notif,Notification[zoneTime].pos,Notification[zoneTime].speed, Notification[zoneTime].pause,effect[fi], effect[fo]);
           P.displayZoneText(zoneXL_L,Notification[zoneTime].Notif,Notification[zoneTime].pos,Notification[zoneTime].speed, Notification[zoneTime].pause,effect[Notification[zoneTime].fxIn], effect[Notification[zoneTime].fxOut]);
          }
          // Affichage synchronisé
          P.synchZoneStart();
          P.displayReset(zoneXL_H);
        }   // fin notif XL
        else {
        // affichage simple
          P.setFont(i, ExtASCII);
          P.displayZoneText(i,Notification[i].Notif,Notification[i].pos, Notification[i].speed,Notification[i].pause, effect[Notification[i].fxIn], effect[Notification[i].fxOut]);
          }
      // fin de préparation pour affichage suivant
        P.displayReset(i);
        if ( Notification[i].cycle > 1 ) Notification[i].cycle--;
        else Notification[i].Alert=false;
     } // fin display notif
    //************ fin notif -sur zone fx led OFF si ON
     if ( i==nzofx && dispNotiffx && notifLed.fx==1 && !configSys.LED) {
          cmdLED(false,0);
          notifLed.fx=0;
          dispNotiffx=false;
          if (configSys.DEBUG) Serial.println("fin notif fx");
     }
   } // fin zone innactif
  else  // zone en cours
    {
      //*************************
      //***** Gestion notif Audio
      //*************************
      if (i==notifAudio.nzo && notifAudio.fx>0 && hardConfig.typeAudio==1)
      {
        if (i==zoneTime && dispNotif ) rtttl::play();
        else if (i!=zoneTime) rtttl::play();
      }
      // debut message avec effet
      if (i==nzofx && notifLed.fx==1 && !dispNotiffx) {
        if (configSys.DEBUG) Serial.println("debut notif fx");
        dispNotiffx=true;}
    } // fin zone en lecture
  } // if XL
  } // fin boucle for
//********************************
// service MQTT
// Verifie connexion serveur MQTT
if (configSys.broker) {
    if (!MQTTclient.connected()) {
        long nowmqtt = millis();
        if (nowmqtt - lastReconnectAttempt >= 10000) {
        lastReconnectAttempt = nowmqtt;
        // Attempt to reconnect
        if (MQTTconnect()) lastReconnectAttempt = 0;

      }
  } else {
      // Client connected  - ecoute MQTT active
      MQTTclient.loop();
      }
    }
    // si broker actif
    if (configSys.broker && MQTTclient.connected())  {
         if (millis() - lastTimeMqtt >= (configSys.tempobroker*1000))
          {
            Serial.println("tempo broker : "+String(configSys.tempobroker*1000));
            lastTimeMqtt = millis();
            MQTTsend();
          }
    }

    // Alarme
    if (configSys.REV) {
      if (hour()==configSys.timeREV[0]) {
        if (minute()==configSys.timeREV[1]) {
          if (second() <5 && second() >0 ) {  alarme(); }
        }
      }
    }

    if (configSys.box && sendBox) {
      long nowbox = millis();
      if (tempoBox==0) tempoBox = nowbox;
      if ((nowbox - tempoBox) >=750) {
        ToBox(configSys.URL_Update);
        tempoBox=0;
        sendBox=false;
      }
    }

    if (checkntp) {
      long nowCheck = millis();
      if (tempoCheck==0) tempoCheck = nowCheck;
      if ((nowCheck - tempoCheck) >=1000) {
        checkTime();
        tempoCheck=0;
        checkntp=false;
      }
    }

} // fin loop
//chargement
void handleFileUpload() {
  if (configSys.DEBUG)  Serial.println("fonction file upload");
  HTTPUpload& upload = server.upload();
  if (configSys.DEBUG)  Serial.println(upload.status);
  if (upload.status == UPLOAD_FILE_START) {
    String filecfg = upload.filename;
    if (filecfg.endsWith(".json")) {
      filecfg = fileconfig;
      if (configSys.DEBUG)  Serial.print("handleFileUpload Name: "); Serial.println(filecfg);
      fsUploadFile = SPIFFS.open(filecfg, "w");
      }
      filecfg = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    //  Reboot = true;
    }
    if (configSys.DEBUG)  Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

// Affichage des pages WEB
//********************* Page web
String getContentType(String filename){
  if (server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".png")) return "image/png";
  return "text/plain";
}

bool handleFileRead(String path){  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += _pagehtml;           // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    File file = SPIFFS.open(path, "r");                    // Open the file
    //size_t sent =
    server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;                                          // If the file doesn't exist, return false
}

String getPage() {
  String page = "<html lang=fr-FR><head><meta http-equiv='refresh' content='10'/>";
  page += "<title>upload</title><meta charset='utf-8'>";
  page += "<style> body { background-color: #fffff; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>";
  page += "</head><body><h3>Le fichier à bien été enregistré</h3>";
  page += "<p>Le notif'heure va rebooter d'ici une trentaine de secondes - Merci d'atendre le reboot pour la prise en compte </p>";
  page += "<p>Vous pouvez cliquer sur ce lien , pour retourner sur l' <a href='/'>Accueil</a> .</p>";
  page += "</body></html>";
  return page;
}
