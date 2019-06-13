
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
const String ver = "0.8.1";
// Bibliotheque à inclure
//***** Gestion reseau
#include <ESP8266mDNS.h>
//********** eeprom / SPIFFS
#include <EEPROM.h>
#include <FS.h>   //Include SPIFFS
#include <ArduinoJson.h>
//*************************
// matrix
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "Font_Clock.h"
#include "Font_Byfeel.h"
#include "Parola_Fonts_data.h"
#include "bigfontext.h"
//NTP
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// wifimanager
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
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
//#define CS_PIN    12  // SS ( D10 sur D1R1  ou D8 sur Mini )

#define MAX_DEVICES 80
// Max zone à gerer
#define MAX 8
// PIN Analogique
#define PINAUTO_LUM A0


const unsigned long MAGICEEP=12345678;
int eeAddress = 64;

// gestion des effets
#define PAUSE_TIME 0
#define SCROLL_SPEED 40


//Hard Config
struct sHardConfig {
  unsigned long magic;
  byte maxDisplay;
  byte zoneTime;
  byte maxZonesMsg;
  bool XL;
  bool perso;
  byte ZP[MAX];
};
sHardConfig hardConfig;

// software config
struct sConfigSys {
  int pauseTime;
  int scrollSpeed;
  byte intensity;
  char NTPSERVER[30];
  int timeZone;                //
  bool DLS;                   // Le fuseau utilise les horaires été / hiver
  bool DEBUG;
  byte posClock;
  char hostName[20];
  bool SEC;
  bool HOR;
  bool LUM;
  char charOff;
};
sConfigSys configSys;
const size_t capacityConfig = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(41) + 500;
const char *fileconfig = "/config/config.json";  // fichier config

WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7200, 60000);

//variable pour inversion zone superieur selon module
bool invertUpperZone = false;  // Type ICS ou FC16

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);


// définition des chaines de stockage pour les zones inf et sup
char msgH[15];
char msgL[15]; // un buffer de 15 caractères au max

// Message a afficher
#define  BUF_SIZE  100       // Taille max des notification ( nb de caractéres max )
//char Notifs[BUF_SIZE];
// variable notif
bool Notif=false;

//variable affichage Zone
byte _spaceChar=1;
int xl=0;
byte _maxDisplayMsg;
byte _maxZones;
byte ZonesWide[MAX];
byte ZWP[MAX];   // zone Wide perso
byte Zones[MAX];
enum {Time_Lo,Time_Up,ZN_1,ZN_2,ZN_3,ZN_4,ZN_5};
byte zoneXL_L,zoneXL_H,zoneTime,zoneMsg;

//variable sys
byte mem=0;


//************* Structure notif
struct sNotif {
  char Notif[BUF_SIZE];
  textPosition_t pos;
  byte fxIn;
  byte fxOut;
  uint16_t speed;
  uint16_t pause;
  byte zone;
  bool Alert;
};

sNotif Notification[7];
byte tab_notif=1;

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
  PA_NO_EFFECT,
  PA_WIPE,
  PA_SCAN_VERTX,
  PA_SLICE,
  PA_FADE,
  PA_OPENING_CURSOR,
  PA_BLINDS,
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


// service WEB
ESP8266WebServer server(80);         // serveur WEB sur port 80


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

/*************************
* * *lecture eeprom * * *
*************************/

void readEConfig() {
//bool err=false;
EEPROM.get( eeAddress, hardConfig);
Serial.println("hardConfig.config="+String(hardConfig.magic));
if (hardConfig.magic != MAGICEEP) {
  Serial.println("Pas de configuration enregistrement par défaut");
  hardConfig.magic=MAGICEEP;
 hardConfig.maxDisplay=4;
 hardConfig.zoneTime=4;
 hardConfig.maxZonesMsg=0;
 hardConfig.XL=false;
 hardConfig.perso=false;
 for (int i=0;i<MAX;i++) {
 hardConfig.ZP[i]=i;
 }
EEPROM.put(eeAddress,hardConfig);
EEPROM.commit();
}
Serial.println("Configuration EEprom trouvé");
Serial.println("Mode XL  ="+String( hardConfig.XL));
Serial.println("Mode maxDisplay  ="+String( hardConfig.maxDisplay));
Serial.println("Mode perso  ="+String( hardConfig.perso));
Serial.println("Mode zoneTime  ="+String( hardConfig.zoneTime));
Serial.println("Mode maxZonesMsg  ="+String( hardConfig.maxZonesMsg));
for (int i=0;i<MAX;i++) {
  Serial.println("Mode ZP-"+String(hardConfig.ZP[i]));
}

}


// JSON
// load config JSON Suystem
void loadConfigSys(const char *fileconfig, sConfigSys  &config) {
   //Init buffer json config
//const size_t capacityConfig = 2*JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(41) + 1500;
  // Open file config
  File file = SPIFFS.open(fileconfig, "r");
   if (!file) {
     //InfoDebugtInit=InfoDebugtInit+" Fichier config Jeedom absent -";
     if (configSys.DEBUG) Serial.println("Fichier Config absent");
    }
  DynamicJsonDocument docConfig(capacityConfig);
  DeserializationError err = deserializeJson(docConfig, file);
  if (err) {
    if (configSys.DEBUG) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(err.c_str());
    }
  }
  // Initialisation des variables systémes pour configuration , si non présente valeur par defaut affecté
  strlcpy(config.NTPSERVER,docConfig["NTPSERVER"] | "pool.ntp.org",sizeof(config.NTPSERVER));
  strlcpy(config.hostName,docConfig["HOSTNAME"] | "NotifeureXL",sizeof(config.hostName));
  config.timeZone = docConfig["TIMEZONE"] | 1;
  config.DLS = docConfig["DLS"] | true;
  config.DEBUG = docConfig["DEBUG"] | false;
  config.pauseTime=docConfig["PAUSE"] | 1000;
  config.scrollSpeed=docConfig["SPEED"] | 30;
  config.intensity=docConfig["INTENSITY"] | 2;
  config.SEC=docConfig["SEC"] | true;
  config.HOR=docConfig["HOR"] | true;
  config.LUM=docConfig["LUM"] | true;
  config.charOff=docConfig["CHAROFF"] | ' ';
  file.close();

} // fin fonction loadconfig

String createJson(sConfigSys  &config) {
  String json;
  //const size_t capacityConfig = 2*JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(41) + 1500;
  DynamicJsonDocument docConfig(capacityConfig);
  docConfig["SEC"]=config.SEC;
  docConfig["HOR"]=config.HOR;
  docConfig["LUM"]=config.LUM;
  docConfig["NTPSERVER"]=config.NTPSERVER;
  docConfig["HOSTNAME"]=config.hostName;
  docConfig["TIMEZONE"]=config.timeZone;
  docConfig["DLS"]=config.DLS;
  docConfig["DEBUG"]=config.pauseTime;
  docConfig["SPEED"] =config.scrollSpeed;
  docConfig["PAUSE"] =config.pauseTime;
  docConfig["INTENSITY"]=config.intensity;
  docConfig["CHAROFF"]=config.charOff;
  docConfig["MEMOJSON"]=docConfig.memoryUsage();
  //donnee hardConfig
  docConfig["XL"]=hardConfig.XL;
  docConfig["MAXDISPLAY"]=hardConfig.maxDisplay;
  docConfig["MAXZONEMSG"]=hardConfig.maxZonesMsg;
  docConfig["ZONETIME"]=hardConfig.zoneTime;
  docConfig["PERSO"]=hardConfig.perso;
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
   }
   f.print(json);  // sauvegarde de la chaine
   f.close();
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
  if (configSys.SEC) sprintf(Sec,"%02d",timeClient.getSeconds());
  //else {Sec[1]=' ';Sec[0]=' ';}
  else Sec[0]='\0';
  createSecondes(Sec);
  sprintf(psz, "%02d%c%02d%s", timeClient.getHours(), (f ? ':' : ' '), timeClient.getMinutes(),Sec);
}
else sprintf(psz,"%c",configSys.charOff);
//sprintf(psz, "%02d%c%02d", timeClient.getHours(), (f ? ':' : ' '), timeClient.getMinutes());
}

void displayNotif(String Msg,int NZO=zoneMsg,byte type=0,textPosition_t pos=PA_LEFT, uint16_t S=configSys.scrollSpeed ,uint16_t P=configSys.pauseTime , byte fi=1,byte fo=1) {
 // Msg.toCharArray( Notifs,BUF_SIZE);
  Notification[NZO].Alert=true;
  Msg.toCharArray( Notification[NZO].Notif,BUF_SIZE);
  if (!hardConfig.XL)  utf8Ascii(Notification[NZO].Notif);
  Notification[NZO].pos=pos;
  Notification[NZO].speed=S;
  Notification[NZO].pause=P;
  Notification[NZO].fxIn=fi;
  Notification[NZO].fxOut=fo;
  switch (type) {
    case 0 : // type message scroll
     Notification[NZO].fxIn=1;
     Notification[NZO].fxOut=1;
     break;
    case 1 : // type info
    Serial.println("mode info");
    Notification[NZO].speed=40;
    Notification[NZO].pause=3000;
    Notification[NZO].pos=PA_CENTER;
    Notification[NZO].fxIn=24;
    Notification[NZO].fxOut=4;
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

// luminosite auto
// fonction reglage auto luminosite
int lumAuto() {
  int sensorValue,lum;
  sensorValue = analogRead(0); // read analog input pin 0
  if (configSys.DEBUG) Serial.println("valeur photocell dans boucle auto : "+String(sensorValue));
   lum =round((sensorValue*1.5)/100);
   lum = constrain(lum,0,15);
   return lum;
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
  Serial.println("valeur de zp2  "+Val);
  int  r=0 , t=0;

   for (int i=0; i < Val.length(); i++)
   {
    if(Val.charAt(i) == split)
     {
       Opt[t] = Val.substring(r, i).toInt();
       r=(i+1);
        Serial.println("valeur de zp"+String(t)+" - "+String(hardConfig.ZP[t]));
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
        if (key=="xl") if (optionsBool(&hardConfig.XL,server.arg(i))) mem=1;
        if (key=="maxdisplay") {optionsNum(&hardConfig.maxDisplay,server.arg(i),4,80);mem=1;}
        if (key=="zonetime") {optionsNum(&hardConfig.zoneTime,server.arg(i),4,80);mem=1;}
        if (key=="maxzonemsg") {
          maxMsg=floor((hardConfig.maxDisplay - hardConfig.zoneTime)/2);
          optionsNum(&hardConfig.maxZonesMsg,server.arg(i),0,maxMsg);
          mem=1;
        }
        if (key=="perso") if (optionsBool(&hardConfig.perso,server.arg(i))) mem=1;
        if (key=="zp") {optionsSplit(hardConfig.ZP,server.arg(i),',');mem=1;}
        if (key=="speed") {optionsNum(&configSys.scrollSpeed,server.arg(i),10,100);mem=2;}

        if (key=="debug") if (optionsBool(&configSys.DEBUG,server.arg(i))) mem=2;
        if (key=="hostname") { if (validString(server.arg(i),4,20)) {
                          value=server.arg(i);
                          value.toLowerCase();
                          value.toCharArray(configSys.hostName,sizeof(configSys.hostName));
                          //toLowerCase();
                          mem=2;
                        }
                      }
if (configSys.DEBUG) {
info = "Arg n"+ String(i) + "–>";
info += server.argName(i) + ": ";
info += server.arg(i) + "\n";
Serial.println(info);
}
}

  switch (mem) {
    case 1 :
        EEPROM.put(eeAddress,hardConfig);
        EEPROM.commit();
        rep="Modification config matrix - Sys reboot";
        reboot=true;
        break;
    case 2 :
        rep=createJson(configSys);
        saveConfigSys(fileconfig,rep);
        rep="Modification config systéme";
        break;
  }

     server.send(200, "text/plane",rep);
     delay(2000);
     if (reboot) ESP.restart();
}

void handleOptions() {
  String key,value;
  mem=0;
  for (int i = 0; i < server.args(); i++) {
    key=server.argName(i);
    key.toUpperCase();
    if (key=="SEC") if (optionsBool(&configSys.SEC,server.arg(i))) mem=1;
    if (key=="HOR") if (optionsBool(&configSys.HOR,server.arg(i))) mem=1;
    if (key=="LUM") if (optionsBool(&configSys.LUM,server.arg(i))) {
        mem=1;
        if (configSys.LUM) value="Auto";
          else value="L : "+String(configSys.intensity);
        displayNotif(value,zoneMsg,9);
      }
    if (key=="INT") {optionsNum(&configSys.intensity,server.arg(i),0,15);
      mem=1;
      configSys.LUM=false;
      value="L : "+String(configSys.intensity);
      displayNotif(value,zoneMsg,9);
    }
  }
  if (mem==1) {
      server.send(200, "text/plane","OK");
  }
  server.send(200, "text/plane","Pas de changement");
}

void handleGetInfo() {
  String rep;
  rep=createJson(configSys);
  server.send(200, "application/json",rep);
}

void handleNotif() {
  String notif,rep;
  rep="Erreur";
  if ( server.hasArg("msg")) {
         notif=server.arg("msg");
         if (notif!="") {
           rep="ok";
         }
       }
  if (rep=="ok") displayNotif(notif);
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
void wifiMan() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  //wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
if(!wifiManager.autoConnect("WIFI-Notifheure")) {
          P.print("erreur AP");
            delay(2000);
            //reset and try again, or maybe put it to deep sleep
            ESP.reset();
            delay(2000);
      }
}
// ************************
// ************************
void setup() {
  EEPROM.begin(512);
  // serial monitor
  SPIFFS.begin();
  // wifi manager
  wifiMan();
 Serial.begin(115200);
 // Initialisation des variables
 // Eeprom
 readEConfig();
 // lecture fichier json dans memoire SPIFFS
 loadConfigSys(fileconfig, configSys);
 // Serveur OTA
Ota(configSys.hostName);
//serveur WEB
//server.on("/Notification",handleNotif); //Gestion des Notifications
 server.on("/Notification",handleNotif); //Gestion des Notifications
 server.on("/Config",handleConfig); //page gestion Options
 server.on("/Options",handleOptions); //page gestion Options
 server.on("/GetInfo",handleGetInfo); //page gestion Options
 server.onNotFound([]() {
  if (!handleFileRead(server.uri())) {
    server.send(404, "text/plain", "FileNotFound");
  }
});
 server.begin();
 // init variable  - Assignation zone
 _spaceChar = 1;
// variable par defaut pour calcul des Zones
int Start,End,WZN;
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
// calcul zone dispo pour Notifications
_maxDisplayMsg=hardConfig.maxDisplay-xl* hardConfig.zoneTime;
Serial.println("Display dispo pour notif : "+String(_maxDisplayMsg));
if (_maxDisplayMsg > 0 ) {
    WZN = floor(_maxDisplayMsg/hardConfig.maxZonesMsg );
    for (int i=xl;i<hardConfig.maxZonesMsg+xl-1;i++)
        {
     ZonesWide[i]=WZN;
    // Serial.println("Zone "+String(i+1)+" taille = "+String(ZonesWide[i]));
     _maxZones++;
      }
  // la derniére zone prend ce qu'il reste
  if (hardConfig.maxZonesMsg>=1)  {
    ZonesWide[_maxZones]=hardConfig.maxDisplay-(hardConfig.maxZonesMsg-1)*WZN-xl*( hardConfig.zoneTime);
     _maxZones++;
    // Serial.println("Zone "+String(_maxZones)+" taille = "+String(ZonesWide[_maxZones-1]));
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
  Serial.println("mode paerso");
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
 Serial.println("zone SL H"+String( zoneXL_H));
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
  if (_maxZones==xl) zoneMsg=zoneTime;
  displayClock();
  if (_maxZones>xl) {
 // P.setFont(zoneMsg,ExtASCII);
 // P.displayZoneText(zoneMsg,Notifs,PA_LEFT,SCROLL_SPEED, PAUSE_TIME,PA_SCROLL_LEFT,PA_SCROLL_LEFT);
  }

  timeClient.begin();
  configSys.intensity=lumAuto();
  //fin setup
  String infoSys="OK IP : ";
 infoSys += WiFi.localIP().toString();
displayNotif(infoSys);

}

void loop() {

  static bool XLZoneTest=true;
  static bool disClock=false;
  timeClient.update();
  static uint32_t  lastTime = 0; // millis() memory
  static uint32_t  lastTimeLumAuto = 0; // millis() memory
  // static uint32_t  lastTimeMsg = 0; // millis() memory
  static bool flasher = false;  // seconds passing flasher
   //  ****** Page WEb :   traite les requetes http et ws
  server.handleClient();
  ArduinoOTA.handle();
  //tmpo
   if (millis() - lastTime >= 1000)
    {
      lastTime = millis();
      flasher = !flasher;
    }
    if (millis() - lastTimeLumAuto >= 20000)
     {
       lastTimeLumAuto = millis();
       if (configSys.LUM) configSys.intensity=lumAuto();
     }

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
                 P.displayClear();
              if (hardConfig.XL) {
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
                       P.displayZoneText(zoneXL_L,Notification[zoneTime].Notif,Notification[zoneTime].pos,Notification[zoneTime].speed, Notification[zoneTime].pause,effect[Notification[zoneTime].fxIn], effect[Notification[zoneTime].fxIn]);
                       P.displayZoneText(zoneXL_H,Notification[zoneTime].Notif,Notification[zoneTime].pos,Notification[zoneTime].speed, Notification[zoneTime].pause,effect[Notification[zoneTime].fxIn], effect[Notification[zoneTime].fxOut]);
                      }
                      P.synchZoneStart();
                 } else {
                  // affichage simple
                  P.setFont(zoneXL_L, ExtASCII);
                   P.displayZoneText(zoneXL_L,Notification[zoneTime].Notif,Notification[zoneTime].pos,Notification[zoneTime].speed, Notification[zoneTime].pause,effect[Notification[zoneTime].fxIn], effect[Notification[zoneTime].fxOut]);
                }
        //}

          Notification[zoneTime].Alert=false;
          disClock=true;
        }
      else {
       //Serial.println("clock");
       if ( !Notification[zoneTime].Alert && disClock) {
        displayClock();
        disClock=false;
       }
     getFormatClock(msgL, flasher);
     if (hardConfig.XL) createHStringXL(msgH, msgL);
     //displayClock();

  }
  P.displayReset(zoneXL_L);
  if (hardConfig.XL) P.displayReset(zoneXL_H);
 }

 //if (_maxZones >xl) {
 for (int i=0;i<_maxZones;i++) {
      if (P.getZoneStatus(i)) {
              if (Notification[i].Alert) {
      Serial.println("notif actif  zoneTime ="+String(i)+" zone msg ="+String(Notification[i].Notif));
 // affichage simple
                  P.setFont(i, ExtASCII);
                 P.displayZoneText(i,Notification[i].Notif,Notification[i].pos, Notification[i].speed,Notification[i].pause, effect[Notification[i].fxIn], effect[Notification[i].fxOut]);
           P.displayReset(i);
       Notification[i].Alert=false;
      }
    }
  }
 //}




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
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    File file = SPIFFS.open(path, "r");                    // Open the file
  //  size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;                                          // If the file doesn't exist, return false
}
