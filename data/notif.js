// fichier javascript pour Notifheure XL
// init variables
var init=1;
var debug=false;
var immp=false;
var Actions = ['Aucune','Afficher / Masquer les Secondes', 'Activer / desactiver Horloge','Mode Manuel ( Mini ) - Manuel ( Maxi ) - Automatique','On / Off LED','Action 1','Action 2','Action 3','Action 4','Action 5','Action 6','Afficher Historique message'];
var couleurs=["Blanc","Rouge","Bleue","Vert","Jaune","Orange","Violet","Rose"];
var jours = ["Dimanche","Lundi","Mardi","Mercredi","Jeudi","Vendredi","Samedi"];
var mois = ["Janvier","Février","Mars","Avril","Mai","Juin","Juillet","Août","Septembre","Octobre","Novembre","Décembre"];
var typeAudio = ["Absent","Buzzer","MP3Player","Autre"];
var typeLed= ["Absent","LED Interne","Commande Relais","Neopixel Ring","Sortie Digitale"];
var buzzer=["The simpsons","Tetris","Arkanoid","Super Mario","Xfiles"];
var ZXL=["Zone XL","Zone XL haut","Zone Message","Zone Notif 2","Zone notif 3","Zone notif 4","Zone Notif 5","Zone notif 6"];
var Z=["Zone Horloge","Zone Message","Zone Notif 2","Zone notif 3","Zone notif 4","Zone Notif 5","Zone Notif 6","Zone Notif 7"];
var cr=true;
var tA,tL;
var ajaxload=false;

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
          $('#version').text(jinfo.VERSION);
          $('#lieu').text(jinfo.NOM);
         WZT=jinfo.ZONETIME;
         $('#WZT').text(WZT);
         MZM=jinfo.MAXZONEMSG;
         TZ=jinfo.TOTALZONE;
         XL=jinfo.XL;
         ZP=jinfo.ZP;
         cr=jinfo.CR;
         $('#MZM').text(MZM);
         $('#TZ').text(TZ);
         if (MZM>0) { $("#groupZone").removeClass("d-none");
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
       $("#num").html(numero);
     } else $("#num").text("Zone Horloge et Msg unique");
         $("#INT").val(jinfo.INTENSITY);
         $("#MINUT").val(jinfo.CRTIME);
         $("#groupLum output").val(jinfo.INTENSITY);
         $('#LUM').prop('checked',jinfo.LUM);

         $('#BRI').val(jinfo.LEDINT);
         $("#groupLed output").val(jinfo.LEDINT);
         $('#LUM').prop('checked',jinfo.LUM);
         $('#SEC').prop('checked',jinfo.SEC);
         $('#HOR').prop('checked',jinfo.HOR);
         $('#LED').prop('checked',jinfo.LED);
         //checkLed();
         $('#hostname').text(jinfo.HOSTNAME);
         $('#mdns').text(jinfo.MDNS);
         $('#uptime').text(up(jinfo.UPTIME));
         $('#first').text(unixToDate(jinfo.STARTTIME));
         $('#last').text(unixToDate(jinfo.LASTSYNCHRO));
         $('#ddj').text(unixToDate(jinfo.DATE));
         $('#NTP').text(jinfo.NTPSERVER);
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
            $('#dht_box').removeClass('d-none');
            $('#Temp').text(jinfo.TEMP);
            $('#Hum').text(jinfo.HUM);
            $('#Modele').text(jinfo.DHTMODEL);
            $('#Status').text(jinfo.DHTSTATUS);
          }
          else $('#dht_box').addClass('d-none');
          tA=jinfo.TYPEAUDIO;
          tL=parseInt(jinfo.TYPELED);
          $('#isLED').text(typeLed[tL]);
          $('#isAUDIO').text(typeAudio[tA]);
          if (tL > 0)  {
            $('#groupLED').removeClass('d-none');

            $('#infoTypeLed').text(" ("+typeLed[tL]+")");
            $('#intLED').val(jinfo.LEDINT);
            $('#COLOR').val(jinfo.COLOR);
                  }
          if (tA > 0)  {
            $('#groupAUDIO').removeClass('d-none');
            $('#infoTypeAudio').text(" ("+typeAudio[tA]+")");
            $('#volAUDIO').val(jinfo.VOLUME);

        }
          if (XL) $('#XL').text('Activé');
          else $('#XL').text('Désactivé');
          if(jinfo.DEBUG) $('#debug').text("Actif");
                 else $('#debug').text("inactif");
          if(jinfo.PHOTOCELL) $('#photocell').text("Présent");
                  else $('#photocell').text("Absent");
          if(jinfo.DHT) { $('#dht').text("Présent");
                $("#cardbtn1").removeClass('d-none');
        }
                  else $('#dht').text("Absent");
          if(jinfo.BTN1) $('#bouton1').text("Présent");
                  else $('#bouton1').text("Absent");
          if(jinfo.BTN2) $('#bouton2').text("Présent");
                  else $('#bouton2').text("Absent");
          if(jinfo.LED) $('#StatutLED').text("Présent");
                  else $('#StatutLED').text("Absent");
          if(jinfo.BOX) $('#boxinfo').text("Présent");
                  else $('#boxinfo').text("Absent");
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


        }
   },
     error: function(resultat,statut) {
        // init();
     },
     complete: function(resultat, statut){
      ajaxload=true;
      checkLum();
      checkLed();
 }

 }); //fin ajax
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
       }
      }
  });
}

function mdns_Info(IP,n) {
  $.getJSON( "http://192.168.8.129/getInfo", function( data ) {
    var items = [];
    $.each( data, function( key, val ) {
      items.push( "<li id='" + key + "'>" + val + "</li>" );
    });

    $( "<ul/>", {
      "class": "my-new-list",
      html: items.join( "" )
    }).appendTo( "body" );
  });
  /*
$.ajax({
     url: "http://"+IP+"/getInfo",
     xhrFields: {
        withCredentials: true
    },
     type: "GET",
     dataType:"html",
     success: function(data) {
       if (IsJsonString(data)){
         var jinfo = JSON.parse(data);
         if (jinfo.HARDWARE=="Notifheure XL") XL=jinfo.HARDAWRE;
         else XL="Notifheure";
         $('#mdnslist').append( "<tr>"
           +"<td >"+n+"</th>"
           +"<td>"+jinfo.NOM+"</th>"
           +"<td>"+IP+"</td>"
           +"<td>"+XL+" : "+jinfo.VERSION+"</td>"
           +"<td>"+jinfo.UPTIME+"</td>"
           +"<td>"+jinfo.RSSI+"</td>"
         +"</tr>" );
       }
      }
  });
  */
}



function getMdns() {
  $('#inforafraichir').text(" ... Scan du reseau en cours ...");
  $('#mdnslist').html("");
  $.ajax({
       url: "/getInfo?MDNS",
       type: "GET",
       dataType:"html",
       success: function(data) {
         var reponse = data.split(":");
                   if (reponse[1]=="0") $('#inforafraichir').text(" ... Aucun notifheure trouvé.");
                   else {
                       listnotif=reponse[1].split(",");
                       var n=listnotif.length;

                       for (i=0;i<n;i++) {
                          $('#inforafraichir').text(" ... Récupération ip notifheure "+listnotif[i]+"...");
                          mdns_Info(listnotif[i],i);
                       }
                        $('#inforafraichir').text("");
                      }

      }
});
}

$("#FormMsg").submit(function(){
Msg=$('#msg').val();
// valeur par défaut
ledType=0;
audioValue=0;
ledValue=0;
numeroPiste=0;
//Si notif led
if ($('#notifLed').prop('checked')) {
  ledType=1;
  if (tL==1 || tL==3) {
    ledValue=$('#intLED').val();
  }
}
else ledType=0;
if ($('#notifAudio').prop('checked')) {
  if (tA==1) {
    numeroPiste=$("#selectTheme option:selected").val();
  }
  else if (tA==2) {
    audioValue=$("#volAUDIO").val();
  }
}
// Msg = escape($('#msg').val()).replace(/\+/g, "%2B");
//Msg = Msg.replace("%u20AC", "%80");  // pour Euro

//type = $('#type').val();
//  int=$('#intensity').val();
//  if ($('#checkInt').prop('checked')) {
//      postInt='';
//    } else postInt='&intnotif='+int;

$.post('/Notification',
     {
msg:Msg,
ledfx:ledType,
ledlum:ledValue,
audio:audioValue,
num:numeroPiste,
type:$("#type").val(),
nzo:$("#selectZone option:selected").val()
   }, function(data) {
$("#infoSubmit").text(data);
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

  $('#groupLed').removeClass("d-none");
if (tL==3)  $('#groupled3').removeClass("d-none");
// Options();
    }
else {
   $('#groupLed').addClass("d-none");
if (tL==3)  $('#groupled3').addClass("d-none");
}
}


function Options(cle = "") {

key = ((typeof cle != 'object') ? cle : key=$(this).attr('id'));
console.log ( "valeur de key :"+$(this).attr('id')+" et type cle "+typeof cle);
if (key=="LUM") checkLum();
if (key=="LED") checkLed();
if (key=="INT" || key=="COLOR")   val=$(this).val();
else if (key=="BRI")   { val=$(this).val()+"&COLOR="+$("#COLOR").val();key="LEDINT";}
else if (key=="TIMEREV")  { val=$("#blocAl h1").text()+":";}
else if (key=="REV")   val=false;
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
 }
 else if (key=="STP") {
   val=0;
   key="MIN";
 }
 else if (key=="CR") {
   cr=!cr;
   val=cr;
 }
console.log("info minuteur :"+key+" = "+val);
url="/Options?"+key+"="+val;
$.get(url);
}

function majMinut() {
console.log("maj minut");
valueOutput(document.getElementById('MINUT'));
$('#MINUT').rangeslider('update', true);
}

$('#notifLed').change(function() {
 if ($(this).prop('checked'))  {
  if (tL==1 || tL==3)  {
        $('#groupLED1').removeClass("d-none");
        valueOutput(document.getElementById('intLED'));
        $('#intLED').rangeslider('update', true);
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

//var rangeSlider = document.getElementById("range");
//var rangeBullet = document.getElementById("rs-bullet");

//rangeSlider.addEventListener("input", showSliderValue, false);

//function showSliderValue() {
//  rangeBullet.innerHTML = rangeSlider.value;
//  var bulletPosition = (rangeSlider.value /rangeSlider.max);
//  rangeBullet.style.left = (bulletPosition * 250) + "px";
//}

$( document ).ready(function() {
getInfo();
$('#LUM').on('change',Options);
$('#SEC').on('change',Options);
$('#HOR').on('change',Options);
$('#INT').on('change',Options);
$('#REV').on('change',Options);
$('#LED').on('change',Options);
$('#BRI').on('change',Options);
$('#COLOR').on('change',Options);
$('#btn_upAl').on('click',upAl);
$('#btn_stopAl').on('click',stopAl);
$('input[type="range"]').rangeslider({
 polyfill: false
});
//remplissage select
$.each(buzzer, function (value, text) {
  $('#selectTheme').append($('<option>', {
          value: value+1,
          text : text
      }));
});

$.each(couleurs, function (value, text) {
  $('#COLOR').append($('<option>', {
          value: value,
          text : text
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

});
