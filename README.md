# Notifheure XL
******************************
Nouvelle Interface ( vue.js )
******************************
* Version 1.1.1 
    * Correction bug sur offset décimal
    * Ajout etiquette FIX et INFO pour commande MQTT
    * ajout Upload , pour charger configuration
    * Correction pb DHT
    * Correction bug interface
    * et toujours des amliorations du code , afin d'apporter une meilleure expérience...

* Version 1.1.0 
    * Prise en charge nouvelle interface Web 2.x ( ancienne version 1.x toujours supporté )
    * Gestion des langues ( anglais , français pour l'instant )
    * Prise en charge amélioré de MQTT ( avec home assistant )
    * et toujours des amliorations du code , afin d'apporter une meilleure expérience...


******************************
Utilisation de LittleFS
******************************

  * Version 1.0.1  
    * correction pb intensité avec mode fix et auto activé
    * correctifs divers 

  * Version 1.0.0  - Il faut utiliser la version 3.5.2 au minimum de la bibliothéque MD_Parola
    * Transfert de SPIFFS à LittleFS . Cette nouvelle version à un nouveau systéme de fichier. Il faudra uploader le dossier data au format LittleFS
    * Ajout de la fonction MQTT device trigger pour HA ( automation sur action btn)
    * ajout fonctionalité extinction auto
    * correctifs divers 

******************************
Utilisation de SPIFFS
******************************

* Version 0.9.7
  * Ajout Offset température / humidité
  * Suppression affichage DHT lorsque l'horloge est off
  * Mofication interface Web pour support nouvelles fonctions
  * ajout d'un commutateur HA , pour valider envoie config MQTT
  * correctifs divers / amélioration de la fonction Intensité auto

* Version 0.9.6
  * Ajout fonction cycle : Pour répétition message
  * Ajout de la création auto dans Home Assistant
  * Modification interface Web ( champ MQTT et cycle )
  * Correctifs bug intensité
  * Mise a jour librairies
  * correctifs divers


* Version 0.9.5
  * Amélioration des commandes MQTT  
  * Ajout du topic pour les commandes Options
  * Correctifs / amélioration code


* Version 0.9.2
  * Correction erreur NTP
  * Nouvelle interface Configuration / ajout fonction synchro et test Internet
  * Prise en charge de la nouvelle version du plugin Notifheure
  * mise en place fonction import / export fichier config
  * Correction divers bug


* Version 0.8.6
  * Corrections bugs divers  
  * Prise en charge Led / relais et Neopixel pour notif visuelle



* Version 0.8.5.3
  * Corrections bugs divers  
  * Amélioration de l'Affichage  
  * Prise en charge buzzer ou HP   


* Version 0.8.5  
  * Gestion multi Zone et double Affichage  
  * Information nouvelle version  
  * ajout de MQTT  
  * correction nombreux bugs  
  * quelques fonctions sont encore inactives  
