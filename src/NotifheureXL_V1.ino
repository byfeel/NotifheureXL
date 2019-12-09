
// Tuto affichage double taille Horloge
// Byfeel 2019
// ****************************
// Exemple affichage  horloge synchronisé via NTP
//  sur 4 module de 4 Matrices Led 72xx
//
// Selon Module , il faudra brancher les matrices de deux façon differentes
// * Modules ( type FC16 ou ICSstation ) - Type ZigZag
//  n n-1 n-2 ... n/2+1   <- Matrice haute
//  n/2 ... 3  2  1  0    <- Matrice basse
//
// * Modules (Parola ou Generic )  - type S
//  n/2+1 ... n-2 n-1 n   -> Matrice haute
//  n/2 ... 3  2  1  0    <- Matrice basse
// ***************************************
#include <ArduinoJson.h>  // compatibilité Autom
const String ver = "0.7.0";
const String hardware = "Notifheure XL";
// Bibliotheque à inclure
//***** Gestion reseau
#include <ESP8266mDNS.h>
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

//NTP
#include <NTPClient.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// wifimanager - Network
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
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
//***************************
// admin page config
//***************************
const char* www_username = "admin";
const char* www_password = "notif";
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
#define CS_PIN    15  // SS ( D10 sur D1R1  ou D8 sur Mini )
//#define CS_PIN    12  // SS ( D10 sur D1R1  ou D8 sur Mini )  ---- Pour NOtifheure 1
// Pour info ancienne version NotifHeure
//#define CLK_PIN   14
//#define DATA_PIN  13
//#define CS_PIN    12
// Options matériels
#define LED_OUT 3 // 0 aucune , 1 Led interne , 2 relais ,3 Ring neopixel ,4 Digital output
#define AUDIO_OUT 2 // 0 aucun , 1 buzzer , 2 MP3player , 3 autres ( sortie relais ou digital)
#define BOUTON1 true
#define BOUTON2 true

//option NeoPixel
// Nombre de led si Ring ou strip neopixel en place
#define LED_COUNT  12


// ********** DHT
#define DHTTYPE DHTesp::DHT11  // si DHT11
// **************
// *** PIN ******
// ***************
#define dhtpin 16 // GPIO16  egale a D2 sur WEMOS D1R1  ou D0 pour mini ( a verifier selon esp )
// Entree analogique pour auto luminosité
// PIN Analogique
#define PINAUTO_LUM A0
// PIN AUDIO ( AUDIOPINTX = MP3PLAYER TX ou Sortie Buzzer ou sortie RElais ou sortie Digital)
// PINAUDIORX : uniquement our MP3player
// PIN qui serviront pour la communication série sur le WEMOS
#define AUDIOPINRX 12   // Entree pour DFP audio uniquement
#define AUDIOPINTX 4    // Sortie principale pour AUdio
// LED PIN ou Relais Lum
#define LEDPIN 5    // Sortie dédié notification lumineuse
//Boutons
#define PIN_BTN1 2
#define PIN_BTN2 0
// ************************
// *********** FIN PIN ****
// ************************

// Config valeur max sytéme ( ne pas modifier )
// matrices max gérés par systémes
#define MAX_DEVICES 80
// Max zone à gerer
#define MAX 8

//***************************
// Valeur par defaut config :
//***************************
#define TAILLENOTIF 4
#define TAILLECLOCK 4
#define NBZONE_SUP 0
#define MODEXL false
#define NTPSERV "pool.ntp.org"
#define UTC +1    // fuseau france
#define DST_DEF false  // Ajustement heure ETE/hivee ( Daylight saving Time )
#define DEBUG_DEF true
#define VOLUME 4 // volume audio de 0 à 10
#define _MP3START 1
#define _MP3NOTIF 1
#define TXTALARM "Alarme !!!"
// gestion des effets
#define PAUSE_TIME 0
#define SCROLL_SPEED 40
// luminosité Led
#define BRIGHTNESS 30
//**************************

// Message a afficher
#define  BUF_SIZE  160      // Taille max des notification ( nb de caractéres max )
#define MAX_HIST 10       // Nombres de message dans historique

//****************************************
//********** AUDIO OUT BUZZER ************
//****************************************
int frequence[] = {262, 294, 330, 349, 370, 392, 440, 494};      // (moi) tableau de fréquence des notes
const char* The_Simpsons = "The_Simpsons:d=4,o=5,b=160:c.6,e6,f#6,8a6,g.6,e6,c6,8a,8f#,8f#,8f#,2g,8p,8p,8f#,8f#,8f#,8g,a#.,8c6,8c6,8c6,c6";
const char* tetris = "tetris:d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
const char* arkanoid = "Arkanoid:d=4,o=5,b=140:8g6,16p,16g.6,2a#6,32p,8a6,8g6,8f6,8a6,2g6";
const char* mario = "mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6";
const char* Xfiles  = "Xfiles:d=4,o=5,b=125:e,b,a,b,d6,2b.,1p,e,b,a,b,e6,2b.,1p,g6,f#6,e6,d6,e6,2b.,1p,g6,f#6,e6,d6,f#6,2b.,1p,e,b,a,b,d6,2b.,1p,e,b,a,b,e6,2b.,1p,e6,2b.";
//***************************************

//******************
// variable systemes
//******************

// Constante systémes
const unsigned long synchroNTP=3600000;
const unsigned long MAGICEEP=13579025;
const byte EP_VERSION = 2;
int eeAddress = 32;
const unsigned long _refTime=1514800000;
const unsigned long _refTimeHigh=1514800000;

//Hard Config  ( sauvé en EEPROM )
struct sHardConfig {
  unsigned long magic;
  byte EPvers;
  byte maxDisplay;
  byte zoneTime;
  byte maxZonesMsg;
  bool XL;
  bool perso;
  byte ZP[MAX];
  bool setup;
};
sHardConfig hardConfig;

// software config  ( sauvegarde SPIFFS )
struct sConfigSys {
  bool setup;                // verifie si setup initial ok
  char nom[20];              // Nom notifheure
  int pauseTime;             // Temps de pause entre chaque animation
  int scrollSpeed;           // vitesse de defliement
  byte intensity;            // Itensité par defaut matrice
  char NTPSERVER[30];        // Adresse NTPserveur
  int timeZone;
  bool DST;                   // Le fuseau utilise les horaires été / hiver
  bool DEBUG;                 // si debug actif
  byte posClock;              //
  char hostName[20];          // nom reseau
  bool SEC;
  bool HOR;
  bool LUM;
  bool REV;
  byte timeREV[2];            // Heure reveil
  char msgAlarme[30];         // Message a affiché pour alarme
  bool LED;
  int LEDINT;                 // Intensité de la veilleuse
  byte color;                 // couleur par défaut veilleuse
  char charOff;               // caractére / symbole à afficher sur horloge eteinte
  char textnotif[20];         // texte prénotif
  bool autoLargeurNotif;
  int CrTime;
  int MP3Start;               // numero MP3 à jouer au démarrage
  int MP3Notif;               // numero MP2 a jouer pour notification
  int typeAudio;
  int typeLED;
  int volumeAudio;             // volume audio par defaut
  bool btn1;
  bool btn2;
  byte btnclic[3][4];          // valeur par defaut des boutons
};
sConfigSys configSys;

const size_t capacityConfig = JSON_ARRAY_SIZE(8) + JSON_OBJECT_SIZE(60) + 800;
const char *fileconfig = "/config/config.json";  // fichier config
const char *fileHist = "/config/Historique.json";  // fichier config

WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset

NTPClient timeClient(ntpUDP, NTPSERV,0, synchroNTP);

//variable pour inversion zone superieur selon module
bool invertUpperZone = false;  // Type ICS ou FC16

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Declare votre neopixel Ring
Adafruit_NeoPixel ring(LED_COUNT, LEDPIN, NEO_GRB + NEO_KHZ800);

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
String stamp_DHT;
String Modele;

// définition des chaines de stockage pour les zones inf et sup
char msgH[15];
char msgL[15]; // un buffer de 15 caractères au max


//char Notifs[BUF_SIZE];
// variable notif
bool AUDIONOTIF = false;
bool Notif=false;
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
byte mem=0;
String msgDebug="";
String _pagehtml="index.html";
bool _photocell=false;
bool _dht=false;
String  hostname;
long int _lastSynchro = 0;
long int _startTime=0;
long int _upTime=0;
String mdnsName;
// boutons
byte clic=0;

//************* Structure notif
struct sNotif {
  char Notif[BUF_SIZE];
  textPosition_t pos;
  byte fxIn;
  byte fxOut;
  uint16_t speed;
  uint16_t pause;
//  byte zone;
  bool Alert;
  int type;
};

sNotif Notification[7];
byte tab_notif=1;
sNotif fxNotif;

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
  ring.gamma32(ring.ColorHSV(52000, 255, 255)), // viloet
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
const struct snotifLed flash={1,100,White,40,4};
const struct snotifLed breath={2,100,White,50,2};

struct snotifAudio {
  int fx;
  int piste;

};

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
WebSocketsServer webSocket = WebSocketsServer(81);

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

const uint8_t F_PMAN1 = 6;
 const uint8_t W_PMAN1 = 8;
static  uint8_t pacman1[F_PMAN1 * W_PMAN1] =  // gobbling pacman animation
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
static uint8_t pacman2[F_PMAN2 * W_PMAN2] =  // ghost pursued by a pacman
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
};
/*************************
* * *lecture eeprom * * *
*************************/
void writeEEPROM() {
 hardConfig.magic=MAGICEEP;
 hardConfig.EPvers=EP_VERSION;
 EEPROM.put(eeAddress,hardConfig);
 EEPROM.commit();
}


void readEConfig(bool reset=false) {
  EEPROM.begin(512);
bool err=false,writeOK=false;
EEPROM.get( eeAddress, hardConfig);
err = hardConfig.magic != MAGICEEP;
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
}
if (hardConfig.EPvers <2 || err) {
  writeOK=true;
  hardConfig.setup=false;
}
if (writeOK) writeEEPROM();

Serial.println("Configuration EEprom");
Serial.println("Version EEProm  ="+String( hardConfig.EPvers));
Serial.println("Mode XL  ="+String( hardConfig.XL));
Serial.println("Mode maxDisplay  ="+String( hardConfig.maxDisplay));
Serial.println("Mode perso  ="+String( hardConfig.perso));
Serial.println("Mode zoneTime  ="+String( hardConfig.zoneTime));
Serial.println("Mode maxZonesMsg  ="+String( hardConfig.maxZonesMsg));
Serial.println("Mode Setup  ="+String( hardConfig.setup));
for (int i=0;i<MAX;i++) {
  Serial.println("Mode ZP-"+String(hardConfig.ZP[i]));
}
EEPROM.end();
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
  strlcpy(config.hostName,docConfig["SUFFIXE-HOST"] | "notifXL",sizeof(config.hostName));
  strlcpy(config.nom,docConfig["NOM"] | "New",sizeof(config.nom));
  strlcpy(config.msgAlarme,docConfig["MSGALARM"] | TXTALARM,sizeof(config.msgAlarme));
  config.timeZone = docConfig["TIMEZONE"] | UTC;
  config.DST = docConfig["DST"] | DST_DEF;
  config.DEBUG = docConfig["DEBUG"] | DEBUG_DEF;
  config.pauseTime=docConfig["PAUSE"] | 3;
  config.scrollSpeed=docConfig["SPEED"] | 30;
  config.intensity=docConfig["INTENSITY"] | 2;
  config.SEC=docConfig["SEC"] | true;
  config.HOR=docConfig["HOR"] | true;
  config.LUM=docConfig["LUM"] | true;
  config.REV=docConfig["REV"] | false;
  config.timeREV[0]=docConfig["TIMEREV"][0] | 7;
  config.timeREV[1]=docConfig["TIMEREV"][1] | 0;
  config.charOff=docConfig["CHAROFF"] | ' ';
  strlcpy(config.textnotif,docConfig["TEXTNOTIF"] | "Notif",sizeof(config.textnotif));
  config.autoLargeurNotif=docConfig["AUTOLARGEURNOTIF"] | true;
  config.CrTime=docConfig["CRTIME"] | 5;
  config.MP3Start=docConfig["MP3START"] | _MP3START;
  config.MP3Notif=docConfig["MP3NOTIF"] | _MP3NOTIF;
  config.typeAudio=docConfig["TYPEAUDIO"] | AUDIO_OUT ;
  config.volumeAudio=docConfig["VOLUME"] | VOLUME ;
  //config.typeLED=docConfig["TYPELED"] | LED_OUT ;
  config.typeLED=LED_OUT ;
  config.LED=docConfig["LED"] | false;
  config.LEDINT=docConfig["LEDINT"]|BRIGHTNESS;
  config.color=docConfig["COLOR"]| 0;
  config.btn1=docConfig["BTN1"] | BOUTON1;
  config.btn2=docConfig["BTN2"] | BOUTON2;
  config.btnclic[1][1] = docConfig["btn1"][0] | 1;
  config.btnclic[1][2] = docConfig["btn1"][1] | 2;
  config.btnclic[1][3] = docConfig["btn1"][2] | 3;
  config.btnclic[2][1] = docConfig["btn2"][0] | 13;
  config.btnclic[2][2] = docConfig["btn2"][1] | 12;
  config.btnclic[2][3] = docConfig["btn2"][2] | 4;
  file.close();

} // fin fonction loadconfig

String createJson(sConfigSys  &config) {
  String json;
  //const size_t capacityConfig = 2*JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(41) + 1500;
  DynamicJsonDocument docConfig(capacityConfig);
  docConfig["HARDWARE"]=hardware;
  docConfig["NOM"]=config.nom;
  docConfig["SEC"]=config.SEC;
  docConfig["HOR"]=config.HOR;
  docConfig["LUM"]=config.LUM;
  docConfig["REV"]=config.REV;
  docConfig["MSGALARM"]=config.msgAlarme;
  JsonArray doctime = docConfig.createNestedArray("TIMEREV");
  doctime.add(config.timeREV[0]);
  doctime.add(config.timeREV[1]);
  docConfig["LED"]=config.LED;
  docConfig["LEDINT"]=config.LEDINT;
  docConfig["COLOR"]=config.color;
  docConfig["NTPSERVER"]=config.NTPSERVER;
  docConfig["SUFFIXE-HOST"]=config.hostName;
  docConfig["TIMEZONE"]=config.timeZone;
  docConfig["DLS"]=config.DST;
  docConfig["DEBUG"]=config.DEBUG;
  docConfig["SPEED"] =config.scrollSpeed;
  docConfig["PAUSE"] =config.pauseTime;
  docConfig["INTENSITY"]=config.intensity;
  docConfig["CHAROFF"]=config.charOff;
  docConfig["TEXTNOTIF"]=config.textnotif;
  docConfig["AUTOLARGEURNOTIF"]=config.autoLargeurNotif;
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
  docConfig["TYPEAUDIO"] = config.typeAudio;
  docConfig["VOLUME"] = config.volumeAudio;
  docConfig["TYPELED"] = config.typeLED;
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
  docConfig["DHTMODEL"]=Modele;
  docConfig["DHTSTATUS"]= dht.getStatusString();
//minuteur
  docConfig["CR"]=CR;
  docConfig["CRSTOP"]=CRStop;
  docConfig["CRTIME"]=config.CrTime;
  //boutons
  //boutons
  docConfig["BTN1"]=config.btn1;
  docConfig["BTN2"]=config.btn2;
    JsonArray btnclic = docConfig.createNestedArray("btnclic");
    btnclic.add(config.btnclic[1][1]);
    btnclic.add(config.btnclic[1][2]);
    btnclic.add(config.btnclic[1][3]);
    btnclic.add(config.btnclic[2][1]);
    btnclic.add(config.btnclic[2][2]);
    btnclic.add(config.btnclic[2][3]);
  //donnee hardConfig
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
    //stamp_DHT=DateFormat(timeClient.getEpochTime());

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
void audio(char action,int val1=1,int val2=1)
{
switch (configSys.typeAudio) {
  case 1 : // Buzzer
          {
            switch (val1) {
              case 1 :rtttl::begin(AUDIOPINTX, The_Simpsons);
                break;
                case 2:rtttl::begin(AUDIOPINTX, tetris);
                break;
                case 3 :rtttl::begin(AUDIOPINTX, arkanoid);
                  break;
                case 4:rtttl::begin(AUDIOPINTX, mario);
                break;
              case 5:rtttl::begin(AUDIOPINTX, Xfiles);
                break;

            }
            while( !rtttl::done() )
             {
                  rtttl::play();
               }
        } // fin 1
        break;
      case 2: // MP3 player
      {
       mySoftwareSerial.begin(9600);
        if (action=='P') {
          val1=floor(val1*30/100);
          Serial.println("Morceau : "+String(configSys.MP3Notif)+"  volume :"+String(val1));

          //myDFPlayer.begin(mySoftwareSerial);

          myDFPlayer.volume(val1);
          //myDFPlayer.playFolder(1,configSys.MP3Start);
          myDFPlayer.playMp3Folder(configSys.MP3Notif);
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
  if (configSys.LEDINT<=0) val=false;
  switch (configSys.typeLED) {
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
configSys.LED=val;
}

// Fonction LED **************
// ***************************
//
void flashLED(int Lo=3,int In=configSys.LEDINT,int Sp=50) {
  for (int i=0;i<Lo;i++) {
    cmdLED(true,In,notifLed.color);
    delay(Sp);
    cmdLED(false,0);
    delay(Sp);
      }
}

void fadeLED(int Lo=1,int In=configSys.LEDINT) {
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
void colorWipe(byte C, int speed=50,bool single=false,int loops=1) {
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

void rainbow(int rainbowLoops,int F=100,int speed=3, int S=255) {
  int fadeVal=0, fadeMax;
  fadeMax=constrain(F, 1, 100);
  for(uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops*65536;
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

void fxLED() {
  if (configSys.typeLED==1) constrain(notifLed.fx, 1, 2);
  if (configSys.typeLED==2) {notifLed.fx=1;notifLed.speed=100;}
  switch (notifLed.fx) {
    case 1 : flashLED(notifLed.loop,notifLed.lum,notifLed.speed);
    break;
    case 2 : fadeLED(notifLed.loop,notifLed.lum);
    break;
    case 3 :
    ring.clear();
    ring.show();
    rainbow(notifLed.loop,notifLed.lum);
    ring.clear();
    ring.show();
    break;
    case 4:
    //ring.clear();
    //ring.show();
     colorWipe(notifLed.color,notifLed.speed,true);
     ring.clear();
     ring.show();
    break;

} //fin switch
notifLed.fx=0;
}

void displayNotif(String Msg,int NZO=zoneMsg,byte type=0,textPosition_t pos=PA_LEFT, uint16_t S=configSys.scrollSpeed ,uint16_t P=configSys.pauseTime , byte fi=1,byte fo=1) {
  if (configSys.DEBUG) {
      Serial.println("**********************");
      Serial.println("valeur Notification : ");
      Serial.println("Message : "+Msg);
      Serial.println("Zone : "+String(NZO)+" Type = "+String(type));
      Serial.println("Position: "+String(pos)+" Speed = "+String(S)+" Pause : "+String(P));
      Serial.println("Effet fxIn : "+String(fi)+"  fxOut : "+String(fo));
      Serial.println("**********************");
    }
 // Verification Zones
 if (NZO >_maxZones-1 || _maxZones==xl) NZO=zoneMsg;
 // calcul largeur notif
 int maxLed,L,largeurMsg;
 largeurMsg=Msg.length();
 if (NZO==zoneTime) L=xl*5;
 else L=5;
 maxLed=ZWP[NZO]*8;
 if (largeurMsg>floor(maxLed/L)) type=0;
 // adapte la pause au millieme
 P=P*1000;
 //construction notif
  Notification[NZO].Alert=true;
  Notification[NZO].type=type;
  Msg.toCharArray( Notification[NZO].Notif,BUF_SIZE);
  if (!hardConfig.XL)  utf8Ascii(Notification[NZO].Notif);
  Notification[NZO].pos=pos;
  Notification[NZO].speed=S;
  Notification[NZO].pause=P;
  Notification[NZO].fxIn=fi;
  Notification[NZO].fxOut=fo;
  //if (configSys.typeLED>0 && notifLED>0) fxLED(1,notifLED);
  if (configSys.typeLED>0 && notifLed.fx>0 ) fxLED();
  //if (music>0 && configSys.typeAudio>0 ) audio('P',music);
  switch (type) {
    case 0 : // type message scroll
     //Notification[NZO].fxIn=1;
     //Notification[NZO].fxOut=1;
     break;
    case 1 : // type info
    Serial.println("mode info");
    //Notification[NZO].speed=40;
    if (P<1000) Notification[NZO].pause=1000;
    Notification[NZO].pos=PA_CENTER;
    Notification[NZO].fxIn=24;
    Notification[NZO].fxOut=4;
    break;
    case 2 : // type fix
    Serial.println("mode fix");
    //Notification[NZO].speed=40;
    Notification[NZO].pause=0;
    Notification[NZO].pos=PA_CENTER;
    Notification[NZO].fxIn=0;
    Notification[NZO].fxOut=13;
    break;
    case 3: // animation Pac
    Serial.println("mode fix");
    //Notification[NZO].speed=40;
    //Notification[NZO].pause=0;
    Notification[NZO].pos=PA_LEFT;
    Notification[NZO].fxIn=14;
    Notification[NZO].fxOut=14;
    break;
    case 9 : // type infoSys
    Serial.println("mode infoSys");
    Notification[NZO].speed=30;
    Notification[NZO].pause=1000;
    Notification[NZO].pos=PA_CENTER;
    Notification[NZO].fxIn=24;
    Notification[NZO].fxOut=4;
    break;
  }

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
        notifLed=flash;
        displayNotif("Fin Minuteur !");
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
  if (configSys.SEC) sprintf(Sec,"%02d",second());

  createSecondes(Sec);
  //sprintf(psz, "%02d%c%02d%s", timeClient.getHours(), (f ? ':' : ' '), timeClient.getMinutes(),Sec);
sprintf(psz, "%c%02d%c%02d%s%c",(minuteur>0?(CRStop?92:93):' ') ,hour(), (f ? ':' : ' '), minute(),Sec,(configSys.REV?'.':' '));

}
else sprintf(psz,"%c",configSys.charOff);
if (CR) displayTimer(psz,f);
//sprintf(psz, "%02d%c%02d", timeClient.getHours(), (f ? ':' : ' '), timeClient.getMinutes());
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
   lum =round((sensorValue*1.5)/100);
   lum = constrain(lum,0,15);
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

  displayNotif(msgInfo,zoneMsg,9);
}

//websockets
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


//MDNS
void serverMDNS( String mdnsName) {
  // Set up mDNS responder:
 if (!MDNS.begin(mdnsName)) {
    if (configSys.DEBUG) Serial.println("Error setting up MDNS responder!");
  }

  if (configSys.DEBUG) Serial.println("mDNS responder started : "+mdnsName);

  //mdns.register("fishtank", { description="Top Fishtank", service="http", port=80, location='Living Room' })
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
  MDNS.addService("notifheure", "tcp", 8800); // Announce notifeur service port 8888 TCP
  MDNS.addServiceTxt("notifheure","tcp", "nom", configSys.hostName);
}

String queryMdns() {
//  MDNSResponder::hMDNSServiceQuery               hMDNSServiceQuery       = 0;
  String ReponseMdns="MDNSreponse:";
  int n = MDNS.queryService("notifheure", "tcp"); // Send out query for esp tcp services

  //if (n==0) ReponseMdns+="0";
for (int i = 0; i < n; ++i) {
    ReponseMdns+= MDNS.IP(i).toString();

    if (n-1>i) ReponseMdns+=",";
}
int m = MDNS.queryService("notifeur", "tcp"); // Pour les anciennes versions
if (n>0 && m>0 ) ReponseMdns+=",";
for (int i = 0; i < m; ++i) {
  ReponseMdns+= MDNS.IP(i).toString();
//  ReponseMdns+="_";
//  ReponseMdns+= MDNS.hostname(i);
  if (m-1>i) ReponseMdns+=",";
}
if (n==0 && m==0 ) ReponseMdns+="0";
return ReponseMdns;
}

// fonction Options
bool optionsBool( bool *pOpt, String val) {
  bool result=true;
  Serial.println("valeur ="+String(val));
  if (val == "true" || val =="1")   *pOpt=true;
  else if (val == "false" || val =="0")   *pOpt=false;
  else result=false;
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
void optionsSplit(byte *Opt,String Val,char split) {
  Serial.println("valeur de function split :  "+Val);
  int  r=0 , t=0;

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

void handleConfig() {
  String rep="Aucune modification";
  String key,info,value;
  int maxMsg;
  bool reboot=false;
  mem=0;
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
        if (key=="setup") if (optionsBool(&hardConfig.setup,server.arg(i))) mem=2;
        if (key=="zp") {optionsSplit(hardConfig.ZP,server.arg(i),',');mem=2;}
        if (key=="speed") {optionsNum(&configSys.scrollSpeed,server.arg(i),10,100);mem=1;}
        if (key=="pause") {optionsNum(&configSys.pauseTime,server.arg(i),0,180);mem=1;}
        if (key=="debug") if (optionsBool(&configSys.DEBUG,server.arg(i))) mem=1;
        if (key=="hostname") {
                if (validString(server.arg(i),4,20)) {
                          value=server.arg(i);
                          value.toLowerCase();
                          value.toCharArray(configSys.hostName,sizeof(configSys.hostName));
                          //toLowerCase();
                          mem=1;
                        }
                      } // fin hostname
        if (key=="name") {
                if (validString(server.arg(i),4,20)) {
                            value=server.arg(i);
                            value.toLowerCase();
                            value.toCharArray(configSys.nom,sizeof(configSys.nom));
                            //toLowerCase();
                            mem=1;
                            }
                        } // fin hostname
        if (key=="checktime" && server.arg(i)=="true") checkTime();rep="mise a jour horloge";
        if (key=="wifireset" && server.arg(i)=="true") wifiReset();
        if (key=="reboot" && server.arg(i)=="true") rep="reboot en cours";reboot=true;
        if (key=="matrixreset" && server.arg(i)=="true") {readEConfig(true);rep="Reset config Matrices";reboot=true;}

if (configSys.DEBUG) {
info = "Arg n"+ String(i) + "–>";
info += server.argName(i) + ": ";
info += server.arg(i) + "\n";
Serial.println(info);
                  }// fin debug

}
bool verif=true;
  switch (mem) {
    case 1 :
        rep=createJson(configSys);
        saveConfigSys(fileconfig,rep);
        rep="OK - Modification config systéme";
        break;
    case 2 :
        if (hardConfig.XL && hardConfig.zoneTime*2 > hardConfig.maxDisplay) verif=false;
        if (!hardConfig.XL && hardConfig.zoneTime > hardConfig.maxDisplay) verif=false;
        if (verif) {
        EEPROM.begin(512);
        EEPROM.put(eeAddress,hardConfig);
        EEPROM.commit();
        EEPROM.end();
        rep="OK - > Modification config matrix -- Systeme reboot - Mode perso : "+String(hardConfig.perso);
        reboot=true;
      }
        else rep="Erreur , les valeurs ne semblent pas coherentes";
        break;

  }

     server.send(200, "text/plane",rep);
     delay(2000);
     if (reboot) ESP.restart();
}

void handleOptions() {
  String key,value;
  String rep="No Options";
  mem=0;
  for (int i = 0; i < server.args(); i++) {
    key=server.argName(i);
    key.toUpperCase();
    if (key=="LED") if (optionsBool(&configSys.LED,server.arg(i))) {mem=1;rep="LED:"+String(configSys.LED);cmdLED(configSys.LED);}
    if (key=="COLOR") { optionsNum(&configSys.color,server.arg(i),0,7);mem=1;rep="COLOR :"+String(configSys.color);cmdLED(configSys.LED);}
    if (key=="LEDINT") {
      optionsNum(&configSys.LEDINT,server.arg(i),0,100);
      cmdLED(configSys.LED,configSys.LEDINT);
      rep="LEDINT : "+String(configSys.LEDINT);
    }
    if (key=="REV") if (optionsBool(&configSys.REV,server.arg(i))) {mem=1;rep="REV:"+String(configSys.REV);}
    if (key=="SEC") if (optionsBool(&configSys.SEC,server.arg(i))) {mem=1;rep="SEC:"+String(configSys.SEC);}
    if (key=="HOR") if (optionsBool(&configSys.HOR,server.arg(i))) {mem=1;rep="HOR:"+String(configSys.HOR);}
    if (key=="LUM") if (optionsBool(&configSys.LUM,server.arg(i))) {
        mem=1;
        cmdLum(configSys.LUM);
        rep="LUM : "+String(configSys.LUM)+" et INT: "+String(configSys.intensity);
      }
      if (key=="TIMEREV") {
        optionsSplit(configSys.timeREV,server.arg(i),':');
        mem=1;
        configSys.REV=true;
        rep="TIMEREV :"+String(configSys.timeREV[0])+"-"+String(configSys.timeREV[1]);
      }
    if (key=="INT") {
      optionsNum(&configSys.intensity,server.arg(i),0,15);
      cmdLum(false,configSys.intensity);
      mem=1;
      rep="INT:"+String(configSys.intensity);
    }
    if (key=="MIN") {
        optionsNum(&minuteur,server.arg(i),0,35999);
        CR=true;
        rep="MIN:"+String(minuteur/60)+" mn";

        P.displayReset(zoneTime);
        P.displayClear(zoneTime);
      if (hardConfig.XL)  P.displayReset(zoneXL_H);
    }
    if (key=="CR") if (optionsBool(&CR,server.arg(i))) {rep="CR:"+String(CR);}
    if (key=="CRSTP") if (optionsBool(&CRStop,server.arg(i))) {rep="CRSTOP:"+String(CRStop);}
  }
  if (mem==1) {
    String json="";
    json=createJson(configSys);
    saveConfigSys(fileconfig,json);
  }
  server.send(200,"text/plane",rep);
}

void handleGetInfo() {
  String rep;
  if ( server.hasArg("MDNS")) {
      rep=queryMdns();
    } else {
  rep=createJson(configSys);
}
  server.send(200, "application/json",rep);

}

void handleNotif() {
  String notif,rep,key,value;
  rep="Erreur";
  //displayNotif(String Msg,int NZO=zoneMsg,byte type=0,textPosition_t pos=PA_LEFT, uint16_t S=configSys.scrollSpeed ,uint16_t P=configSys.pauseTime , byte fi=1,byte fo=1)
  int NZO=zoneMsg;
  byte type=0;
  textPosition_t pos=PA_LEFT;
  int S=configSys.scrollSpeed ;
  int P=configSys.pauseTime;
  notifLed.fx=0;
  notifLed.lum=configSys.LEDINT;
  notifLed.loop=1;
  notifLed.color=configSys.color;
  notifLed.speed=50;
  int A=0;
  byte fi=1,fo=1;
  //parcours argument http
  for (int i = 0; i < server.args(); i++) {
    key=server.argName(i);
    key.toUpperCase();
    if (key=="MSG" && server.arg(i)!="") { notif=server.arg(i);rep="ok";}
    if (key=="AUDIO") optionsNum(&A,server.arg(i),0,10);
    if (key=="LEDFX") {
          optionsNum(&notifLed.fx,server.arg(i),0,10);
          if (notifLed.fx==1) notifLed.loop=3;
        }
    if (key=="LEDLUM") { optionsNum(&notifLed.lum,server.arg(i),0,100);}
    if (key=="FLASH") notifLed=flash;
    if (key=="BREATH") notifLed=breath;
    if (key=="NZO" ) { NZO=server.arg(i).toInt();}
    if (key=="TYPE" ) { type=server.arg(i).toInt();}
    if (key=="SPEED" ) { optionsNum(&S,server.arg(i),10,100);}
    if (key=="PAUSE" ) { optionsNum(&P,server.arg(i),0,180);}
    if (key=="FI" ) { optionsNum(&fi,server.arg(i),0,28);}
    if (key=="FO" ) { optionsNum(&fo,server.arg(i),0,28);}
  }
  /*if ( server.hasArg("msg")) {
         notif=server.arg("msg");
         if (notif!="") {
           rep="ok";
         }
       }*/
  if (rep=="ok") displayNotif(notif,NZO,type,pos,S,P,fi,fo);
  server.send(200, "text/plane",rep);
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
  while(!timeClient.update()) {
  timeClient.forceUpdate();
}
setTime(timeClient.getEpochTime());
_lastSynchro=now();
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
  if (configSys.DEBUG) Serial.println("setup led : "+String(configSys.typeLED));

  switch (configSys.typeLED) {
    case 1 : // Led interne
    {
    pinMode(LEDPIN,OUTPUT);
    digitalWrite(LEDPIN,!configSys.LED);
    }
    break;
    case 2 : // autres , relais
    {
    pinMode(LEDPIN,OUTPUT);
    }
    case 3 : // ring neopixel
    {
      int Br;
      Br=map(configSys.LEDINT,0,100,0,255);
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
if (btnclic < 4 ) actionClick=configSys.btnclic[btn][btnclic];
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
  break;
      case 12 : // affichage / masque Minuteur
              CR=!CR;
              break;
      case 13 : //Lancer Minuteur
              minuteur = configSys.CrTime*60;
              CR=true;
      break;
    default:

      break;
  }
  if (m==1) {
String json="";
json=createJson(configSys);
saveConfigSys(fileconfig,json);
}
}

void infoSetup(int step,String txt="") {
P.print(txt);
notifLed.fx=step;
notifLed.lum=BRIGHTNESS;
if (configSys.typeLED==1)  {
  notifLed.loop=3;
}
else if (configSys.typeLED==3)  {
notifLed.fx=notifLed.fx+2;
notifLed.color=Blue;
}
else notifLed.fx=0;
fxLED();
}

void alarme() {
  notifLed=flash;
  displayNotif("Alarme !");
}

// ****************************
/******************************
 ********* SETUP **************
 ******************************/
// ****************************

void setup() {
  // serial monitor pour debug
 Serial.begin(115200);
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
 //****************** Etpae 1
 infoSetup(1,"Init ...");
// Connexion WIFI
  wifiMan();
// **********
// config Sortie AUDIO Si present
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
    infoSetup(2,"DFplayer ...");
      // serial softawre (pour player )
     mySoftwareSerial.begin(115200);
     if (!myDFPlayer.begin(mySoftwareSerial,false)) {  //Utilisation de  softwareSerial pour communiquer
    // if (!myDFPlayer.begin(Serial1)) {  //Utilisation de  softwareSerial pour communiquer
     Serial.println("Pb communication:");
     Serial.println("1.SVP verifier connexion serie!");
     Serial.println("2.SVP verifier SDcard !");
     while(true){
          delay(0); // Code to compatible with ESP8266 watch dog.
        }
     }
      //Serial.println("DFPlayer Mini En ligne.");
      myDFPlayer.setTimeOut(500); // Définit un temps de time out sur la communication série à 500 ms
      infoSetup(2,"DFplayer ... OK");
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
// ************************ Etape 2
infoSetup(2,"Net ...");
//delay(300);
// Debug fsinfo
if (configSys.DEBUG) {
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
 // creation hostname wifi// Set Hostname.
   hostname = configSys.nom;
   hostname +=configSys.hostName;
   mdnsName = hostname;
   hostname +="-"+String(ESP.getChipId(), HEX);
   WiFi.hostname(hostname);
//
//************** Services MDNS
mdnsName.toLowerCase();
serverMDNS(mdnsName);
 // Serveur OTA
Ota(hostname);
Serial.println(msgDebug);
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
 server.onNotFound([]() {
  if (!handleFileRead(server.uri())) {
    server.send(404, "text/plain", "FileNotFound");
  }
});
 server.begin();

//WebSocketsServer
webSocket.begin();
webSocket.onEvent(webSocketEvent);

 // recuperation info temps
 // recuperation heure
   timeClient.begin();
   bool s=true;
   String MsgInfo;
   int Offset;
   Offset=(configSys.timeZone+(configSys.DST?1:0))*3600;
while(!timeClient.update()) {

   timeClient.forceUpdate();
   MsgInfo=(s ? "Time ..." : "Time .. ");
   infoSetup(2,MsgInfo);
   s=!s;
 }
 timeClient.setTimeOffset(Offset);
 setTime(timeClient.getEpochTime());
 _startTime=now();
 _lastSynchro=timeClient.getEpochTime();

if (configSys.DEBUG) Serial.println("time ok :"+String(now())+" offset :"+String(Offset));

infoSetup(2,"Ok ...");
 // init options
 // Initialize temperature sensor
  // dht.setup(dhtPin, DHTType);
  int pin(dhtpin);
  dht.setup(pin,DHTTYPE);
  delay(500);
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

// Selon modele des matrices - inversion de l'affichage ( true si Generic ou Parola )
invertUpperZone = (HARDWARE_TYPE == MD_MAX72XX::GENERIC_HW || HARDWARE_TYPE == MD_MAX72XX::PAROLA_HW);

//init Zones
for (int i=0;i<MAX;i++) {
  ZWP[i]=ZonesWide[Zones[i]];
}
//uint8_t max = 0;

  // initialisation des Zones
  P.begin(_maxZones);
  P.setIntensity(configSys.intensity);
  if (_maxZones==1) {
      P.setZone(zoneXL_L , 0, hardConfig.maxDisplay-1);
  }
  else {
    //Largeur zone enregistré
  for (int i=0;i<7;i++)
  {
     Serial.println("Zone Perso creation "+String(i)+" taille = "+String(ZWP[i]));
  }
      Start=0;
      for (int i=0;i<_maxZones;i++)
      {
         End=Start+ZWP[i];
         Serial.println("Creation Zone : "+String(i)+" de "+String(Start)+" a "+String(End));
        P.setZone( i, Start, End - 1);
         Start=End;
    }

  }
  uint16_t S,E;
  // verification Zone
  for (int j=0;j<_maxZones;j++)
  {

  P.getDisplayExtent(j,S,E);
  Serial.println("Zone : "+String(j)+" colonne : "+String(S)+" à "+String(E));
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
P.setSpriteData(pacman1, W_PMAN1, F_PMAN1,pacman2, W_PMAN2, F_PMAN2);   // chargement animation en memoire

//prepa zone horloge
if (_maxZones==xl) zoneMsg=zoneTime;
displayClock();

configSys.intensity=lumAuto();

//fin setup
String infoSys="OK IP : ";
infoSys += WiFi.localIP().toString();
displayNotif(infoSys);

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
  static uint32_t  lastTimeSystem= 0; // millis() system / synchro

  static bool flasher = false;  // seconds passing flasher
   //  ****** Page WEb :   traite les requetes http et ws
  server.handleClient();
  webSocket.loop();
  ArduinoOTA.handle();
  timeClient.update();
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
  if (millis() - lastTimeSystem >= synchroNTP+500)
     {
       lastTimeSystem =millis();
       checkTime();
     }
     // ***********  Gestion bouton
   if (configSys.btn1) {
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
   if (configSys.btn2) {
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
// Alarme
if (configSys.REV) {
  if (hour()==configSys.timeREV[0]) {
    Serial.println("Mode heure alarme");
    if (minute()==configSys.timeREV[1]) {
      if (second() <5 ) {
      alarme();
      }
    }
  }
}


//********************************
// Affichage display *************
// *******************************
P.setIntensity(configSys.intensity);
P.displayAnimate();
     if (hardConfig.XL) XLZoneTest=P.getZoneStatus(zoneXL_L) && P.getZoneStatus(zoneXL_H);
      else XLZoneTest=P.getZoneStatus(zoneTime);
// l'animation commence

if ( XLZoneTest) {

             if (Notification[zoneTime].Alert) {
                //if (zoneMsg == zoneTime ) {
                   Serial.println("mode notification");
                 //display notif
                P.displayClear(zoneTime);
              if (hardConfig.XL) {
                Serial.println("mode XL");
                P.displayClear(zoneXL_H);
                  // affichage double
                     P.setFont(zoneXL_L, BigFontLower);
                     P.setFont(zoneXL_H, BigFontUpper);
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
                        if (Notification[zoneTime].type==1 ) {
                          fi=Notification[zoneTime].fxOut;
                          fo=Notification[zoneTime].fxIn;
                        } else
                        {
                          fo=Notification[zoneTime].fxOut;
                          fi=Notification[zoneTime].fxIn;
                        }
                       P.displayZoneText(zoneXL_H,Notification[zoneTime].Notif,Notification[zoneTime].pos,Notification[zoneTime].speed, Notification[zoneTime].pause,effect[fi], effect[fo]);
                       P.displayZoneText(zoneXL_L,Notification[zoneTime].Notif,Notification[zoneTime].pos,Notification[zoneTime].speed, Notification[zoneTime].pause,effect[Notification[zoneTime].fxIn], effect[Notification[zoneTime].fxOut]);
                      }
                      P.synchZoneStart();
                      Serial.println("valeur de fx dans affichage : "+String(Notification[zoneTime].fxOut));
                 } else {
                  // affichage simple
                  Serial.println("mode simple");
                  P.setFont(zoneXL_L, ExtASCII);
                   P.displayZoneText(zoneXL_L,Notification[zoneTime].Notif,Notification[zoneTime].pos,Notification[zoneTime].speed, Notification[zoneTime].pause,effect[Notification[zoneTime].fxIn], effect[Notification[zoneTime].fxOut]);
                }
        //}
          Notification[zoneTime].Alert=false;
          finNotif();
        //  if (Notification[zoneTime].type==2) Notification[zoneTime].Alert=true;
          if (Notification[zoneTime].type!=2 ) disClock=true;

        }
      else {
       //Serial.println("clock");
       if ( !Notification[zoneTime].Alert && disClock) {
        displayClock();
        disClock=false;
       }
       // affichage horloge
       if (Notification[zoneTime].type!=2 ) {
       getFormatClock(msgL, flasher);
       if (hardConfig.XL) createHStringXL(msgH, msgL);
     }
  }
  P.displayReset(zoneXL_L);
 if (hardConfig.XL) P.displayReset(zoneXL_H);
 }

 //if (_maxZones >xl) {
 for (int i=0;i<_maxZones;i++) {
    if (i!=zoneTime) {
      if (P.getZoneStatus(i)) {
              if (Notification[i].Alert) {
                 P.displayClear(i);
      Serial.println("notif boucle zone ="+String(i)+" msg ="+String(Notification[i].Notif));
 // affichage simple
                  P.setFont(i, ExtASCII);
                 P.displayZoneText(i,Notification[i].Notif,Notification[i].pos, Notification[i].speed,Notification[i].pause, effect[Notification[i].fxIn], effect[Notification[i].fxOut]);
           P.displayReset(i);
       Notification[i].Alert=false;
     } // fin alert
   } // fin zonestatus
  } // fin if xl
} // fin boucle


} // fin loop


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
