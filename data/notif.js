// fichier javascript pour Notifheure XL
// init variables
var init=1;
var debug=false;
var immp=false;
var boutons=["Aucune","Afficher / Masquer les secondes","Afficher / Masquer l'horloge","Mode luminosite Mini / Maxi / Automatique","On / Off Veilleuse","Historique Message","Afficher / masquer Minuteur","lancer Minuteur","Action 1","Action 2","Action 3"];
var Actions = ['Aucune','Afficher / Masquer les Secondes', 'Activer / desactiver Horloge','Mode Manuel ( Mini ) - Manuel ( Maxi ) - Automatique','On / Off LED','Action 1','Action 2','Action 3','Action 4','Action 5','Action 6','Afficher Historique message'];
var couleurs=["Blanc","Rouge","Bleu","Vert","Jaune","Orange","Violet","Rose"];
var jours = ["Dimanche","Lundi","Mardi","Mercredi","Jeudi","Vendredi","Samedi"];
var mois = ["Janvier","Février","Mars","Avril","Mai","Juin","Juillet","Août","Septembre","Octobre","Novembre","Décembre"];
var typeAudio = ["Absent","HP / Buzzer","MP3Player","Autre"];
var typeLed= ["Absent","LED Interne","Commande Relais","Neopixel Ring","Sortie Digitale"];
var buzMusic=["Mission Impossible","Star Wars","Indiana Jones","Panthere Rose","Famille Adam's","l'exorciste","The simpsons","Tetris","Arkanoid","Super Mario","Xfiles","AxelF","PacMan","dambuste","Muppet show","James Bond","Take On Me","Agence tout risque","Top Gun","les Schtroumpfs","l'arnaque","looney Tunes","20 century fox","Le bon, la brute ...","Retour vers le futur"];
var ZXL=["Zone XL","Zone XL haut","Zone Message","Zone Notif 2","Zone notif 3","Zone notif 4","Zone Notif 5","Zone notif 6"];
var Z=["Zone Horloge","Zone Message","Zone Notif 2","Zone notif 3","Zone notif 4","Zone Notif 5","Zone Notif 6","Zone Notif 7"];
var fx=[ 'PRINT','SCROLL_LEFT','SCROLL_UP_LEFT','SCROLL_DOWN_LEFT','SCROLL_UP','GROW_UP','SCAN_HORIZ','BLINDS','WIPE','SCAN_VERTX','SLICE','FADE','OPENING_CURSOR','NO_EFFECT','SPRITE','CLOSING','SCAN_VERT','WIPE_CURSOR','SCAN_HORIZX','DISSOLVE','MESH','OPENING','CLOSING_CURSOR','GROW_DOWN','SCROLL_DOWN','SCROLL_DOWN_RIGHT','SCROLL_UP_RIGHT','SCROLL_RIGHT','RANDOM'];
var animation=['PACMAN','fleche 1',"Roll 1","Marcheur","space invader","chevron","Coeur","Bateau vapeur","Voilier","boule de feu","rocket","ligne","vague","fantome pacman","fleche 2","roll 2"];
var fxLed3=['on','flash','breath','rainbow','colorWipe','colorWipeFill','chaseColor'];
var fxLed2=['on'];
var fxLed1=['on','flash','breath'];
var typMat=['Parola','Genric','ICStation','FC16'];
var cr=false;
var crstp=false;
var tA,tL;
var ajaxload=false;
var ver;
var pause;
var color;

function IsJsonString(str) {
try {
    JSON.parse(str);
} catch (e) {
    return false;
}
return true;
}
function notifheure(n,i,v,u,s) {
this.nom=n;
this.ip=i;
this.version=v;
this.up=u;
this.signal=s;
}

function unixToDate(unixStamp) {
var dt=eval(unixStamp*1000);
var myDate = new Date(dt);
return(myDate.toLocaleString( 'fr-FR',{ timeZone: 'UTC' }));
}

function up(seconds) {
seconds = Number(seconds);
var d = Math.floor(seconds / (3600*24));
var h = Math.floor(seconds % (3600*24) / 3600);
var m = Math.floor(seconds % 3600 / 60);
var s = Math.floor(seconds % 60);

var dDisplay = d > 0 ? d + " J, " : "";
var hDisplay = h > 0 ? h + " H, " : "";
var mDisplay = m > 0 ? m + " m, " : "";
var sDisplay = s > 0 ? s + " s" : "";
return dDisplay + hDisplay + mDisplay + sDisplay;
}
function pad (str, max) {
  str = str.toString();
  return str.length < max ? pad("0" + str, max) : str;
}
// Example functionality to demonstrate a value feedback
function valueOutput(element) {
var value = element.value;
var output = element.parentNode.getElementsByTagName('output')[0] || element.parentNode.parentNode.getElementsByTagName('output')[0];
output['innerText'] = value;
}

var DEBUG=true;

function checktime() {
  $.get('/Config?checktime');
  update_Info();
}

function checknet() {
  $.get('/Config?checknet',function(data){
  console.log(data);
  update_Info();
  });
}

function sendConfigHA() {
  $.get('/Config?mqttconfig',function(data){
  console.log(data);
  //update_Info();
  });
}

function goConfig() {
  window.location.href='/editconfig.html';
}

function getInfo() {
$.ajax({
     url: "/getInfo",
     type: "GET",
     dataType:"html",
     success: function(data) {
       console.log(data);
       if (IsJsonString(data)){
         var jinfo = JSON.parse(data);
         $("#display").text(jinfo.MAXDISPLAY);
         ver=jinfo.VERSION;
          $('#version').text(ver);
          $('#lieu').text(jinfo.NOM);
         WZT=jinfo.ZONETIME;
         $('#WZT').text(WZT);
         MZM=jinfo.MAXZONEMSG;
         TZ=jinfo.TOTALZONE;
         XL=jinfo.XL;
         ZP=jinfo.ZP;
         tA=jinfo.TYPEAUDIO;
         tL=parseInt(jinfo.TYPELED);
         pause=parseInt(jinfo.PAUSE);
         $('#MZM').text(MZM);
         $('#TZ').text(TZ);
         if (MZM>0) {
        $("#groupZone").removeClass("d-none");
         if (XL)  Zones=ZXL;
         else Zones=Z;
         numero="<ul class='list-group list-group-flush'>";
         //remplissage select
         for (i=0;i<=(XL?MZM+1:MZM);i++) {
           if ((!XL) || (XL && i!=1)) {
           $('#selectZone').append($('<option>', {
                   value: ZP[i],
                   text : Zones[i]
               }));
         }
         numero=numero+"<li class='list-group-item d-flex justify-content-between align-items-center'>"+Zones[i]+"  <span class='badge badge-info badge-pill'>"+ZP[i]+"</span></li>";
       }
       numero=numero+"</ul>";
       $("#selectZone option:contains(Zone Message)").attr("selected", "selected");
       $("#num").html(numero);
     } else $("#num").text("Zone Horloge et Msg unique");
         $("#INT").val(jinfo.INTENSITY);
         $("#MINUT").val(jinfo.CRTIME);
         $("#groupLum output").val(jinfo.INTENSITY);
         $('#LUM').prop('checked',jinfo.LUM);
         $('#typMat').text(typMat[jinfo.TYPEMATRICE]);

         //$("#groupLed output").val(jinfo.LEDINT);
         $('#LUM').prop('checked',jinfo.LUM);
         $('#infoLUM').text(jinfo.INTENSITY);
         $('#SEC').prop('checked',jinfo.SEC);
         $('#HOR').prop('checked',jinfo.HOR);
         $('#tzname').text(jinfo.TZNAME);
         $('#hostname').text(jinfo.HOSTNAME);
         $('#mdns').text(jinfo.MDNS);
         $('#uptime').text(up(jinfo.UPTIME));
         $('#first').text(unixToDate(jinfo.STARTTIME));
         $('#last').text(unixToDate(jinfo.LASTSYNCHRO));
         $('#ddj').text(unixToDate(jinfo.DATE));
         $('#NTP').text(jinfo.NTPSERVER);
         if (jinfo.NTPOK) {
           $('#blocInfoNTP').addClass("bg-dark");
           $('#blocInfoNTP').removeClass("bg-danger");
           $('#NTP').addClass("text-dark");
           $('#NTP').removeClass("text-danger");
         }
         else {
            $('#blocInfoNTP').addClass("bg-danger");
            $('#blocInfoNTP').removeClass("bg-dark");
            $('#NTP').removeClass("text-dark");
            $('#NTP').addClass("text-danger");
          }
          if (jinfo.NETOK) {
            $("#net").text("En ligne");
            $('#net').addClass("text-success");
            $('#net').removeClass("text-danger");
            $('#blocInfoNet').addClass("bg-success");
            $('#blocInfoNet').removeClass("bg-danger");
          }
          else {
            $('#net').addClass("text-danger");
            $('#net').removeClass("text-success");
            $("#net").text("Hors Ligne");
            $('#blocInfoNet').addClass("bg-danger");
            $('#blocInfoNet').removeClass("bg-success");
          }
         $('#MAC').text(jinfo.MAC);
          $('#IP').text(jinfo.IP);
          $('#SSID').text(jinfo.SSID);
          $('#RSSI').text(jinfo.RSSI);
          $('#BSSID').text(jinfo.BSSID);
          $('#channel').text(jinfo.CHANNEL);
          $('#masque').text(jinfo.MASQUE);
          $('#passerelle').text(jinfo.PASSERELLE);
          $('#DNS').text(jinfo.DNS);
          $('#DNS2').text(jinfo.DNS2);
          if (jinfo.DHT) {
            $("#navdht").removeClass("disabled");
            $('#dht_box').removeClass('d-none');
            $('#Temp').text(jinfo.TEMP);
            $('#Hum').text(jinfo.HUM);
            $('#Modele').text(jinfo.DHTMODEL);
            $('#Status').text(jinfo.DHTSTATUS);
            $('#dhtdate').text(unixToDate(jinfo.STAMPDHT));
            $('#dht_t').text(jinfo.TEMP);
            $('#dht_h').text(jinfo.HUM);
            $('#dht_hi').text(parseInt(jinfo.HI).toFixed(2));
            $('#dht_p').text(parseInt(jinfo.ROSE.toFixed(2)));
            $('#DDHT').prop('checked',jinfo.DDHT);
          }
          else $('#dht_box').addClass('d-none');

          $('#isLED').text(typeLed[tL]);
          $('#isAUDIO').text(typeAudio[tA]);
          // si LED
          if (tL > 0)  {
            $('#LED').prop('disabled', false);
            $('#LED').prop('checked',jinfo.LED);
            $('#groupLED').removeClass('d-none');
            $('#BRI').val(jinfo.LEDINT);
            $('#infoTypeLed').text(" ("+typeLed[tL]+")");
            // pour fonctions
            if (jinfo.CRFX>0) $('#INVCR').text(fxLed3[jinfo.CRFX-1]);
            if (jinfo.ALFX>0) $('#INVAL').text(fxLed3[jinfo.ALFX-1]);
            if (tL>1) {
                if (tL !=2 ) $('#intLEDFX').val(jinfo.FXINT);
            }
            if (tL==3) {
              color=jinfo.COLOR;
              $('#COLOR').val(jinfo.COLOR);
              $('#loopLED').val(2);
                    }

          }
            else {
              $('#groupLED').addClass('d-none');
            }
          if (tA > 0)  {
            $('#groupAUDIO').removeClass('d-none');
            $('#infoTypeAudio').text(" ("+typeAudio[tA]+")");
            $('#volAUDIO').val(jinfo.VOLUME);
              if (jinfo.CRFXSOUND>0) $('#INACR').text(buzMusic[jinfo.CRFXSOUND-1]);
              if (jinfo.ALFXSOUND>0) $('#INAAL').text(buzMusic[jinfo.ALFXSOUND-1]);
            if (tA==2) {
              $('#cardMP3').removeClass('d-none');
              $('#MP3list').text(jinfo.TOTALMP3-jinfo.MP3_1);
              $('#MP3sound').text(jinfo.MP3_1);
              $('#MP3notif').text(jinfo.MP3NOTIF);
            }

        } else $('#groupAUDIO').addClass('d-none');
          if (XL) $('#XL').text('Activé');
          else $('#XL').text('Désactivé');
          if(jinfo.DEBUG) {
            $('#debug').text("Actif");
            DEBUG=true;
          }
                 else {
                   DEBUG=false;
                   $('#debug').text("inactif");
                 }
          if(jinfo.PHOTOCELL) $('#photocell').text("Présent");
                  else $('#photocell').text("Absent");
          if(jinfo.DHT) { $('#dht').text("Présent");

        }
          else $('#dht').text("Absent");
          if(jinfo.BTN1) { $('#bouton1').text("Présent");
                        $("#cardbtn1").removeClass('d-none');
                        $('#btn1clic1').text(boutons[jinfo.btnclic[0]]);
                        $('#btn1clic2').text(boutons[jinfo.btnclic[1]]);
                        $('#btn1clic3').text(boutons[jinfo.btnclic[2]]);
                      }
                  else $('#bouton1').text("Absent");
          if(jinfo.BTN2) { $('#bouton2').text("Présent");
                  $("#cardbtn2").removeClass('d-none');
                  $('#btn2clic1').text(boutons[jinfo.btnclic[3]]);
                  $('#btn2clic2').text(boutons[jinfo.btnclic[4]]);
                  $('#btn2clic3').text(boutons[jinfo.btnclic[5]]);
        }
                  else $('#bouton2').text("Absent");
          if(jinfo.LED) $('#StatutLED').text("Présent");
                  else $('#StatutLED').text("Absent");
          if(jinfo.BOX) {
              $('#boxinfo').text("Activé");
              $("#cardbox").removeClass("d-none");
              $('#IUACR').text(Actions[jinfo.ACTION[0]]);
              $('#IUAAL').text(Actions[jinfo.ACTION[1]]);
              $('#infoaction1').text(jinfo.URL_ACT1);
              $('#infoaction2').text(jinfo.URL_ACT2);
              $('#infoaction3').text(jinfo.URL_ACT3);
              $('#infoupdate').text(jinfo.URL_UPD);
            }
              else {
                $('#boxinfo').text("Désactivée");
                $("#cardbox").addClass("d-none");
              }
            $("#Alarme input").val(pad(jinfo.TIMEREV[0],2)+":"+pad(jinfo.TIMEREV[1],2));

          if (jinfo.REV) {
            $("#blocAl").addClass("text-danger");
            $("#blocAl").removeClass("text-secondary");
            $("#blocAl h3").text("Alarme Active");
            $("#blocAl h1").text($("#Alarme input").val());
          }
          else {
            $("#blocAl").addClass("text-secondary");
            $("#blocAl").removeClass("text-danger");
            $("#blocAl h3").text("Alarme Inactive");
          }
          cr=jinfo.CR;
          crstp=jinfo.CRSTOP;

          if (jinfo.BROKER) {
            $("#cardmqtt").removeClass("d-none");
            $('#mqttState').text("activé");
            if (jinfo.STATEBROKER) {
              $("#infostatemqtt").text("Connexion OK");
              $("#infostatemqtt").removeClass("text-danger");
            }
            else {
              $("#infostatemqtt").addClass("text-danger");
              $("#infostatemqtt").text("Erreur Connexion");
            }
            $("#infoipmqtt").text(jinfo.SRVBROKER);
            if (jinfo.UBROKER !="") $("#infousermqtt").text(jinfo.UBROKER);
            else $("#infousermqtt").text("Serveur MQTT anonyme");
            $("#infotempomqtt").text(jinfo.TEMPOBROKER);
            $("#infotopicS").text(jinfo.TOPIC);
            $("#infotopicO").text(jinfo.TOPICOPT);
            $("#infotopicP").text(jinfo.TOPICSTATE);
            $("#predisco").text(jinfo.PREFIXHA);
          }
          else {
              $("#cardmqtt").addClass("d-none");
          }
          //Jours
          for (i=1;i<8;i++) {
            $("#alday"+i).prop('checked',jinfo.ALDAY[i]);
          }
        }
   },
     error: function(resultat,statut) {
        // init();
     },
     complete: function(resultat, statut){
      ajaxload=true;
      if (cr) {
        $('#CR').text('Afficher');
      }
      else $('#CR').text('Masquer');
      checkLum();
      checkLed();
      setInterval('update_Info();',20000); /* rappel après 20 secondes  */
      checkGithub();
      if (tL==1) fxLed3=fxLed1;
      else if (tL==2) fxLed3=fxLed2;
      if (tL>0) {
        $.each(fxLed3, function (value, text) {
          $('#selectfxled').append($('<option>', {
                value: value+1,
                text : (value+1)+" - "+text
            }));
          });
        }
       if (tL==3) {
        $('#scoled').removeClass("d-none");
        $('#cycle').removeClass("d-none");
        $.each(couleurs, function (value, text) {
          $('#selectcolorled').append($('<option>', {
                value: value,
                text : value+" - "+text
            }));
          });
      }
 } // fin complete

 }); //fin ajax
}

function getHisto() {
    $('#histoList').html("");
  $.getJSON( "/getInfo?HISTO", function( json ,status) {
    console.log(json);
    var n=1
    $("#badgehist").text(json.index);
    $.each(json.notif,function( key, val ) {
      $('#histoList').append( "<tr>"
        +"<td style='width:5%;'>"+n+"</td>"
        +"<td>"+unixToDate(json.date[key])+"</td>"
        +"<td style='width:60%;'>"+val+"</td>"
        +"<td>"+String.fromCharCode(json.flag[key])+"</td>"

      +"</tr>" );
      n++;
  });
});
}

function infoJson() {
  $.getJSON( "/info.json", function(json) {
    console.log(json.version);
    $('#vspiffs').text(json.version);
});
}



function update_Info() {
$.ajax({
     url: "/getInfo",
     type: "GET",
     dataType:"html",
     success: function(data) {
       if (IsJsonString(data)){
         var jinfo = JSON.parse(data);
        cr=jinfo.CR;
        $('#ddj').text(unixToDate(jinfo.DATE));
         $('#uptime').text(up(jinfo.UPTIME));
         $('#dhtdate').text(unixToDate(jinfo.STAMPDHT));
         $('#dht_t').text(jinfo.TEMP);
         $('#dht_h').text(jinfo.HUM);
         $('#dht_hi').text(parseInt(jinfo.HI).toFixed(2));
         $('#dht_p').text(parseInt(jinfo.ROSE.toFixed(2)));
         $('#RSSI').text(jinfo.RSSI);
         $('#last').text(unixToDate(jinfo.LASTSYNCHRO));
         $('#Temp').text(jinfo.TEMP);
         $('#Hum').text(jinfo.HUM);
         $('#infoLUM').text(jinfo.INTENSITY);
         if (jinfo.BROKER) {
         if (jinfo.STATEBROKER) {
           $('#blocInfoMQTT').removeClass("bg-danger");
           $('#blocInfoMQTT').removeClass("bg-dark");
           $('#blocInfoMQTT').addClass("bg-success");
           $("#infostatemqtt").text("Connexion OK");
           $("#infostatemqtt").removeClass("text-danger");
         }
         else {
           $("#infostatemqtt").addClass("text-danger");
           $("#infostatemqtt").text("Erreur Connexion");
           $('#blocInfoMQTT').removeClass("bg-dark");
           $('#blocInfoMQTT').removeClass("bg-success");
           $('#blocInfoMQTT').addClass("bg-danger");
         }
       }
       else {
         $('#blocInfoMQTT').removeClass("bg-danger");
         $('#blocInfoMQTT').removeClass("bg-success");
         $('#blocInfoMQTT').addClass("bg-dark");
         $('#mqttState').text("désactivé");
       }
         if (jinfo.NTPOK) {
           $('#blocInfoNTP').addClass("bg-dark");
           $('#blocInfoNTP').removeClass("bg-danger");
           $('#NTP').addClass("text-dark");
           $('#NTP').removeClass("text-danger");
         }
         else {
            $('#blocInfoNTP').addClass("bg-danger");
            $('#blocInfoNTP').removeClass("bg-dark");
            $('#NTP').removeClass("text-dark");
            $('#NTP').addClass("text-danger");
          }
          if (jinfo.NETOK) {
            $("#net").text("En ligne");
            $('#net').addClass("text-success");
            $('#net').removeClass("text-danger");
            $('#blocInfoNet').addClass("bg-success");
            $('#blocInfoNet').removeClass("bg-danger");
          }
          else {
            $('#net').addClass("text-danger");
            $('#net').removeClass("text-success");
            $("#net").text("Hors Ligne");
            $('#blocInfoNet').addClass("bg-danger");
            $('#blocInfoNet').removeClass("bg-success");
          }
         var httpcode=parseInt(jinfo.INFO);
         if (httpcode==200) {
         $('#infoEtatURL').text("derniére requéte URL : OK");
       }
         else {
            $('#infoEtatURL').text("derniére requéte URL : Erreur ( code  "+jinfo.INFO+" )");
         }
       }
      }
  });
}

function mdns_Info(IP,n) {
  console.log("recherche info pour ip : "+IP);
  $.getJSON( "http://"+IP+"/getInfo", function( data ) {
    console.log(data);
    if(data.hasOwnProperty('Options')){
      nom=data.system.lieu;
      version=data.system.version;
      uptime=data.system.uptime;
      signal=data.system.RSSI+" dBm";
          }
      else {
        nom=data.NOM;
        version=data.VERSION;
        uptime=up(data.UPTIME);
        signal=data.RSSI;
      }
        $('#mdnslist').append( "<tr>"
          +"<td >"+n+"</td>"
          +"<td>"+nom+"</td>"
          +"<td>"+IP+"</td>"
          +"<td>"+version+"</td>"
          +"<td>"+uptime+"</td>"
          +"<td>"+signal+"</td>"
        +"</tr>" );
  });

}



function getMdns() {
  $('#inforafraichir').text(" ... Scan du reseau en cours ...");
  $('#mdnslist').html("");
  $.ajax({
       url: "/getInfo?MDNS",
       type: "GET",
       dataType:"html",
       success: function(data) {
         console.log("rep " + data);
         var reponse = data.split(":");
                   if (reponse[1]=="0") $('#inforafraichir').text(" ... Aucun notifheure trouvé.");
                   else {
                       listnotif=reponse[1].split(",");
                       var n=listnotif.length;
                       n=n-1;
                       console.log("il y a "+n+" notifheure");
                       for (i=0;i<n;i++) {
                          $('#inforafraichir').text(" ... Récupération ip notifheure "+listnotif[i]+"...");
                          if (listnotif[i].length != 0 ) { mdns_Info(listnotif[i],i+1);}
                       }
                        $('#inforafraichir').text("fin recherche");

                      }

      }
});
}

$("#FormMsg").submit(function(){
Message=$('#inputMsg').val();
// valeur par défaut
audioValue=0;
ledValue=0;
numeroPiste=0;
animation=0;
P=pause;
fxled=0;
loop=1;
colorled=color;
//Si notif led
if ($('#notifLed').prop('checked')) {
  if (tL>0) fxled=$("#selectfxled option:selected").val();
  if (tL==1 || tL==3) {
    ledValue=$('#intLEDFX').val();
    loop=$("#loopLED").val();
  }
  if ( tL==3) {
    colorled=$("#selectcolorled option:selected").val();
  }
  if (tL==2) ledValue=100;
}

if ($('#notifAudio').prop('checked')) {
  if (tA==1) {
    numeroPiste=$("#selectTheme option:selected").val();
  }
  else if (tA==2) {
    audioValue=$("#volAUDIO").val();
  }
}

typ=$("#type option:selected").val();
if (typ==6) fio=$('#FX').val();
else if (typ==7) {
  fio=14;
  animation=$('#Anim').val();
}
else fio=1;
if (typ==1) P=parseInt($('#pauseInfo').val());

// Message = escape($('#msg').val()).replace(/\+/g, "%2B");
//Msg = Msg.replace("%u20AC", "%80");  // pour Euro


$.post('/Notification',
     {
msg:Message,
ledlum:ledValue,
audio:audioValue,
num:numeroPiste,
nzo:$("#selectZone option:selected").val(),
fio:fio,
anim:animation,
cycle:$("#cycle").val(),
type:typ,
pause:P,
ledfx:fxled,
color:colorled,
loop:loop
   }, function(data) {
     //$("#infoSubmit").text(data);
     console.log(data);
 }).done(function() {
     getHisto();
  });

return false;
});

function checkLum()
{
if ($('#LUM').prop('checked'))  {
 $('#groupLum').addClass("d-none");
 $('label[for="checkInt"]').text('Luminosité Auto');
// Options();
    }

else {
$('#groupLum').removeClass("d-none");
$('label[for="checkInt"]').text('Luminosité Manuelle');
$('#INT').rangeslider('update', true);
//$('#groupLum output').val();
}
}

function checkLed()
{
  $('#BRI').rangeslider('update', true);
  if ($('#LED').prop('checked'))  {
      if (tL!=2) $('#groupLed').removeClass("d-none");
      if (tL==3)  $('#groupled3').removeClass("d-none");
      // Options();
  }
  else {
   $('#groupLed').addClass("d-none");
   if (tL==3)  $('#groupled3').addClass("d-none");
}

}

function checkDay() {
  ald="false,";
  for (i=1;i<8;i++) {
    ald+=$("#alday"+i).prop('checked')+",";
  }
Options('ALD',ald);
}

function Options(cle = "" , valeur = "") {

key = ((typeof cle != 'object') ? cle : key=$(this).attr('id'));
console.log ( "valeur de key :"+$(this).attr('id')+" et type cle "+typeof cle);
if (key=="LUM") checkLum();
if (key=="LED") checkLed();
if (key=="INT" || key=="COLOR")   val=$(this).val();
else if (key=="BRI")   { val=$(this).val()+"&COLOR="+$("#COLOR").val();key="LEDINT";}
else if (key=="TIMEREV")  { val=$("#blocAl h1").text();}
else if (key=="REV")   val=false;
else if (key=="ALD") val=valeur;
else val=$(this).prop('checked');

console.log ("Envoie des valeur "+key+" et "+val);
url="/Options?"+key+"="+val;
$.get(url, function( data ) {
var res = data.split(":");
console.log("retour serveur : "+data);
if (res[0]=="INT") {
  $('#INT').val(res[1]);
  $('#INT').rangeslider('update', true);
  }

});
}


function upAl() {
  $("#blocAl").addClass("text-danger");
  $("#blocAl").removeClass("text-secondary");
  $("#blocAl h3").text("Alarme Active");
  $("#blocAl h1").text($("#Alarme input").val());
 Options("TIMEREV");
}

function stopAl() {
  $("#blocAl").addClass("text-secondary");
  $("#blocAl").removeClass("text-danger");
  $("#blocAl h3").text("Alarme Inactive");
  Options("REV");
}

/*
$('#Alarme').on('change.datetimepicker', function(e) {
        // `e` here contains the extra attributes
        //if ("#REV".prop('checked'))
        var formatedValue = e.date.format("HHmm");
        alert(formatedValue);
    });


/*
$("#Alarme").change(function() {
     if ($("#REV").prop('checked'))  {
    Options("REV");
     }
});
*/

function Minuteur(key) {
if (key=="MIN") {
  $('#MINUT').rangeslider('update', true);
   val=$('#MINUT').val();
   val=val*60;
   cr=true;
 }
 else if (key=="STP") {
   val=0;
   key="MIN";
 }
 else if (key=="CR") {
   cr=!cr;
   val=cr;
 }
 else if (key=="CRSTP") {
   crstp=!crstp;
   val=crstp;
 }
console.log("info minuteur :"+key+" = "+val);
url="/Options?"+key+"="+val;
$.get(url);
if (cr) {
  $('#CR').text('Masquer');
}
else $('#CR').text('Afficher');
}

function majMinut() {
console.log("maj minut");
valueOutput(document.getElementById('MINUT'));
$('#MINUT').rangeslider('update', true);
}

$('#notifLed').change(function() {
  console.log("test notif : "+tL);
 if ($(this).prop('checked'))  {
  if (tL==1 || tL==3)  {
        $('#groupLED1').removeClass("d-none");
        valueOutput(document.getElementById('intLEDFX'));
        $('#intLEDFX').rangeslider('update', true);
        valueOutput(document.getElementById('loopLED'));
        $('#loopLED').rangeslider('update', true);
      }
  if (tL==2) {
      $('#groupLED1').removeClass("d-none");
      $('#groupledintfx').addClass("d-none");
    }
 }
 else {
   $('#groupLED1').addClass("d-none");
 }

});

$('#notifAudio').change(function() {
 if ($(this).prop('checked'))  {
  if (tA==1 ) {
      $('#groupAUDIO1').removeClass("d-none");
    }
    else if (tA==2) {
        $('#groupAUDIO2').removeClass("d-none");
        valueOutput(document.getElementById('volAUDIO'));
        $('#volAUDIO').rangeslider('update', true);
      }
 }
 else {
   $('#groupAUDIO'+tA).addClass("d-none");
 }
});

$('#type').change(function() {
  if ($(this).val()==6) $('#blocFx').removeClass("d-none");
  else {
    $('#blocFx').addClass("d-none");
  }
  if ($(this).val()==7) $('#blocAn').removeClass("d-none");
  else $('#blocAn').addClass("d-none");
  if ($(this).val()==1) $('#pausefield').removeClass("d-none");
  else  {
    $('#pausefield').addClass("d-none");
    $('#pauseInfo').val('3').change();
  }
});

$('#selectfxled').change(function() {
  if (tL==3) {
  if ($(this).val()==4) $('#scoled').addClass('d-none');
    else $('#scoled').removeClass('d-none');
    if ($(this).val()==6) $('#cycle').addClass('d-none');
    else $('#cycle').removeClass('d-none');
  }
  if (tL==3 || tL==1) {
    if ($(this).val()==1) $('#cycle').addClass('d-none');
    else $('#cycle').removeClass('d-none');
  }
});

function checkGithub() {
  $.getJSON( "https://api.github.com/repos/byfeel/NotifheureXL/releases/latest", function( data ) {
    if (data.hasOwnProperty('tag_name')) {
      console.log(data);
      var lastver=data.tag_name;
      var urlV=data.html_url;
      console.log("version : "+ver+" et new : "+lastver);
      if (ver!=lastver) {
        $("#newversion").text("nouvelle version ");
        $("#newversion").removeClass("bg-dark");
        $("#newversion").addClass("bg-warning");
        $("#lastversion").addClass("text-warning");
      }
       $("#lastversion").text(data.tag_name);
       $("#urlversion").attr('href',urlV);
  }
  });
}

$( document ).ready(function() {
getInfo();
getHisto();
infoJson();
update_Info()
$('#LUM').on('change',Options);
$('#SEC').on('change',Options);
$('#HOR').on('change',Options);
$('#INT').on('change',Options);
$('#REV').on('change',Options);
$('#LED').on('change',Options);
$('#BRI').on('change',Options);
$('.alday').on('change',checkDay);
$('#COLOR').on('change',Options);
$('#DDHT').on('change',Options);
$('#btn_upAl').on('click',upAl);
$('#btn_stopAl').on('click',stopAl);
$('input[type="range"]').rangeslider({
 polyfill: false
});
//remplissage select
$.each(buzMusic, function (value, text) {
  $('#selectTheme').append($('<option>', {
          value: value+1,
          text : (value+1)+" - "+text
      }));
});



$.each(couleurs, function (value, text) {

  $('#COLOR').append($('<option>', {
          value: value,
          text : text
      }));
});


$.each(fx, function (value, text) {
  if (value<13 || value>14) {  //no sprite  no effect
  $('#FX').append($('<option>', {
          value: value,
          text : value+" - "+text
      }));
  }
});

$.each(animation, function (value, text) {
  $('#Anim').append($('<option>', {
          value: value,
          text : value+" - "+text
      }));

});
/*
$('#Alarme').datetimepicker({
                    locale: 'fr',
                    format: 'HH:mm',
                    inline: false,
                    widgetPositioning:{
                      horizontal: 'left',
                      vertical: 'bottom'
                                  }

});
*/
$(document).on('input', 'input[type="range"]', function(e) {
        valueOutput(e.target);
    });

//requete test version notifheure XL


}); // fin fonction document
