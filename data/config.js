

var debug=true;
var maxdisplay;
var boutons=["Aucune","Afficher / Masquer les secondes","Afficher / Masquer l'horloge","Mode luminosite Mini / Maxi / Automatique","On / Off Veilleuse","Historique Message","Afficher / masquer Minuteur","lancer Minuteur","Action 1","Action 2","Action 3"];
var couleurs=["Blanc","Rouge","Bleue","Vert","Jaune","Orange","Violet","Rose"];
var optLed=["Aucune","Led","Relais","Neopixel Ring/Strip"];
var fx=['Aucun','on','flash','breath','raimbow','colorWipe ( Single LED )','colorWipe ( Full LED )','color chase'];
var fx1=['Aucun','on','flash','breath'];
var fx2=['Aucun','on'];
var flag={"I":"Important","F":"Fix ou Info","N":"Notifications","Q":"Notification MQTT","J":"Notification Jeedom / nextDom","A":"Avec Animation","X":"Avec effets","S":"notification systéme","D":"notification défaut"};
var buzMusic=["Aucun","Mission Impossible","Star Wars","Indiana Jones","Panthere Rose","Famille Adam's","l'exorciste","The simpsons","Tetris","Arkanoid","Super Mario","Xfiles","AxelF","PacMan","dambuste","Muppet show","James Bond","Take On Me","Agence tout risque","Top Gun","les Schtroumpfs","l'arnaque","looney Tunes","20 century fox","Le bon, la brute ...","Retour vers le futur"];
var nbClic=3;
var tabFlag=[];
var validflag;
var tL;
var fxcr;
var fxal;

function IsJsonString(str) {
    try {
    JSON.parse(str);
    } catch (e) {
    return false;
    }
    return true;
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
        $("#IP").text(jinfo.IP);
        $("#nom").text(jinfo.NOM);
        $("#inputhost").val(jinfo.SUFFIXEHOST);
        $('#DEBUG').prop('checked',jinfo.DEBUG);
        $('#AutoMsg').prop('checked',jinfo.ALN);
        $('#bouton1').prop('checked',jinfo.BTN1).change();
        $('#bouton2').prop('checked',jinfo.BTN2).change();
        $('#dst').prop('checked',jinfo.DST);
        $("#inputNTP").val(jinfo.NTPSERVER);
        $("#CRTIME").val(jinfo.CRTIME);
        $("#CRTEXT").val(jinfo.MSGMINUT);
        $("#ALTEXT").val(jinfo.MSGALARM);
        $("#btn1_1").val(jinfo.btnclic[0]);
        $("#btn1_2").val(jinfo.btnclic[1]);
        $("#btn1_3").val(jinfo.btnclic[2]);
        $("#btn2_1").val(jinfo.btnclic[3]);
        $("#btn2_2").val(jinfo.btnclic[4]);
        $("#btn2_3").val(jinfo.btnclic[5]);
        tL=jinfo.TYPELED;
        $("#selectNotifLum").val(jinfo.TYPELED).change();
        $("#selectNotifAud").val(jinfo.TYPEAUDIO).change();
        $('#btnMQTT').prop('checked',jinfo.BROKER).change();
        $('#btnDOM').prop('checked',jinfo.BOX).change();
        $('#URL_action1').val(jinfo.URL_ACT1);
        $('#URL_action2').val(jinfo.URL_ACT2);
        $('#URL_action3').val(jinfo.URL_ACT3);
        $("#ipMQTT").val(jinfo.SRVBROKER);
        $("#userMQTT").val(jinfo.UBROKER);
        $("#passMQTT").val(jinfo.PBROKER);
        $("#portMQTT").val(jinfo.PORTBROKER);
        $("#tempoMQTT").val(jinfo.TEMPOBROKER);
        $('#DispOff').val(String.fromCharCode(jinfo.CHAROFF));
        $('#intled').val(jinfo.LEDINT).trigger("input");
        $('#intled').rangeslider('update', true);
        $('#fxintled').val(jinfo.FXINT).trigger("input");
        $('#fxintled').rangeslider('update', true);
        $('#speed').val(jinfo.SPEED).trigger("input");
        $('#speed').rangeslider('update', true);
        $('#pause').val(jinfo.PAUSE).trigger("input");
        $('#pause').rangeslider('update', true);
        $('#selectled3_color').val(jinfo.FXCOLOR);
        fxcr=jinfo.CRFX;
        fxal=jinfo.ALFX;
        $('#CRAudio').val(jinfo.CRFXSOUND);
        $('#ALAudio').val(jinfo.ALFXSOUND);
        $('#CRACT').val(jinfo.ACTION[0]);
        $('#ALACT').val(jinfo.ACTION[1]);
        validflag=jinfo.HFLAG;
        var charoff =jinfo.CHAROFF;
        if ( charoff == 32 ) $('#DispOff').attr("placeholder", "Espace");
        else if (cahroff == 0 ) $('#DispOff').attr("placeholder", "Vide");

    }
    } ,
      error: function(resultat,statut) {
         // init();
      },
      complete: function(resultat, statut){
        cpt=0;
        $.each( flag, function( key, value ) {
        if (validflag.indexOf(key)>=0) {
          tabFlag[cpt]=key;
          cpt++;
        }
        });
        if (tL==1) fx=fx1;
        if (tL==2) fx=fx2;
        $.each(fx, function (value, text) {
          $('#selectled3_fxAL').append($('<option>', {
                  value: value,
                  text : text
              }));
        });
        $.each(fx, function (value, text) {
          $('#selectled3_fx').append($('<option>', {
                  value: value,
                  text : text
              }));
        });
        console.log(tabFlag);
      $("#selectFlag").val(tabFlag);
      $('#selectled3_fx').val(fxcr);
      $('#selectled3_fxAL').val(fxal);
  }
  }); //fin ajax
}

// feedback slider value
function valueOutput(element) {
var value = element.value;
var output = element.parentNode.getElementsByTagName('output')[0] || element.parentNode.parentNode.getElementsByTagName('output')[0];
output['innerText'] = value;
}

function Reboot() {
var r = confirm("Etes vous sur de vouloir effectué un Reboot ?");
if (r == true) {
  $.post( "/Config?reboot=true", function( data ) {
    $("#infoconfig").text("le systéme reboot , retour vers page d'accueil dans quelques secondes");
  setTimeout( function() {window.location.href='/';}, 10000);
  });
}
};

function Reset() {
var r = confirm("Confirmez vous votre demande de Remise à zéro !");
if (r == true) {
$("#raz").html("<h1>Remise à Zero du Notif'Heure en cours ....</h1>");
 Socket.send("RESET");
setTimeout( function() {window.location.href='http://byfeel.info';}, 5000);
}
};

function Purge() {
var r = confirm("Etes vous sur de vouloir effacer l'historique");
if (r == true) {
  Socket.send("PURGE");
}
};

$("#Config").submit(function(){
  TFlag=$('#selectFlag').val();
  strFlag="";
  TFlag.forEach(function(item){
  strFlag+=item;
  });
  console.log($("#btnMQTT").prop('checked'));

  $.post('/Config',
       {
  hostname:$("#inputhost").val(),
  debug:$('#DEBUG').prop('checked'),
  automsg:$('#AutoMsg').prop('checked'),
  pause:$('#pause').val(),
  speed:$('#speed').val(),
  box:$('#btnDOM').prop('checked'),
  url1:$('#URL_action1').val(),
  url2:$('#URL_action2').val(),
  url3:$('#URL_action3').val(),
  ntpserver:$('#inputNTP').val(),
  charoff:$('#DispOff').val().charCodeAt(0),
  crtext:$('#CRTEXT').val(),
  altext:$('#ALTEXT').val(),
  crtime:$('#CRTIME').val(),
  clicbtn1 : $('#btn1_1').val()+","+$('#btn1_2').val()+","+$('#btn1_3').val()+",",
  clicbtn2 : $('#btn2_1').val()+","+$('#btn2_2').val()+","+$('#btn2_3').val()+",",
  intled : $("#intled").val(),
  fxint : $("#fxintled").val(),
  color : $("#selectled3_color").val(),
  fxcr :$("#selectled3_fx").val(),
  fxal : $("#selectled3_fxAL").val(),
  aucr :$("#CRAudio").val(),
  aual : $("#ALAudio").val(),
  broker:$("#btnMQTT").prop('checked'),
  ipbroker:$("#ipMQTT").val(),
  ubroker:$("#userMQTT").val(),
  pbroker:$("#passMQTT").val(),
  portbroker:$("#portMQTT").val(),
  tempobroker:$("#tempoMQTT").val(),
  action:$('#CRACT').val()+","+$('#ALACT').val()+",",
  flag:strFlag
//  nzo:$("#selectZone option:selected").val()
   }, function(data) {
    $.notify(data);
    $('html, body').animate({scrollTop: '0px'}, 1500);
   });
return false;
});

$( document ).ready(function() {
// init champs
//select Boutons
$('#bouton1').change(function () {
            var check = $(this).prop('checked');
            if (check) $('#btn1').removeClass("d-none");
            else $('#btn1').addClass("d-none");

        });
$('#bouton2').change(function () {
          var check = $(this).prop('checked');
          if (check) $('#btn2').removeClass("d-none");
          else $('#btn2').addClass("d-none");
                });
$('#btnMQTT').change(function () {
          var check = $(this).prop('checked');
          if (check) $('#MQTT').removeClass("d-none");
          else $('#MQTT').addClass("d-none");
                });
$('#btnDOM').change(function () {
          var check = $(this).prop('checked');
          if (check) $('#box').removeClass("d-none");
          else $('#box').addClass("d-none");
                });

$('#selectFlag').change(function() {
  console.log($(this).val());
});
//visuelle
$('#selectNotifLum').change(function () {
          var check = $(this).val();

          if (check>0)  {
            $('#led3').removeClass("d-none");
            $('#groupLEDFonctions').removeClass("d-none");
            $("#infovisuel").text(optLed[check]);
              if (check==1 || check==3 ) $('#groupLED1').removeClass("d-none");
              else $('#groupLED1').addClass("d-none");
              if (check==3) $('#groupLED3').removeClass("d-none");
              else $('#groupLED3').addClass("d-none");
              if (check==2) $("#groupled0").removeClass("d-none");
              else $("#groupled0").addClass("d-none");
          }
          else {
            $('#led3').addClass("d-none");
            $('#groupLEDFonctions').addClass("d-none");
          }
                });

  $('#selectNotifAud').change(function () {
            var check = $(this).val();
            if (check>0)  {
              if (check==1) $('#groupAUDIO1').removeClass("d-none");
            }
            else $('#groupAUDIO1').addClass("d-none");
});
//remplissage select
$.each(boutons, function (value, text) {
//  console.log("valeur :"+text);
  for (i=1;i<(nbClic+1);i++) {
  $('#btn1_'+i).append($('<option>', {
          value: value,
          text : text
      }));
  }
  for (i=1;i<(nbClic+1);i++) {
  $('#btn2_'+i).append($('<option>', {
          value: value,
          text : text
      }));
  }
});

$.each(couleurs, function (value, text) {
  $('#selectled3_color').append($('<option>', {
          value: value,
          text : text
      }));
});





$.each(buzMusic, function (value, text) {
  $('#CRAudio').append($('<option>', {
          value: value,
          text : text
      }));
});

$.each(buzMusic, function (value, text) {
  $('#ALAudio').append($('<option>', {
          value: value,
          text : text
      }));
});

$.each( flag, function( key, value ) {
  $('#selectFlag').append($('<option>', {
          value: key,
          text : value
      }));

});



// activation range
$('input[type="range"]').rangeslider({
 polyfill: false
});
$(document).on('input', 'input[type="range"]', function(e) {
        valueOutput(e.target);
    });
// map timeZone
/*$('#map').timezonePicker({
  quickLink: [{
      "PST": "PST",
      "MST": "MST",
      "CST": "CST",
      "EST": "EST",
      "GMT": "GMT",
      "PARIS": "Europe/Paris",
      "IST": "IST"
  }],
  width: 800,
  height: 400,
  hoverColor: '#666666',
  selectedColor: '#333333',
  mapColor: '#BBB',
  mapHover: null,
  defaultCss: true,
  localStore: true,
  selectBox: true,
  showHoverText: true,
  hoverText: null,
  dayLightSaving: ((typeof moment == "function") ? (true) : (false))
});*/
//$("#map").data('timezonePicker').setValue('Europe/Paris');
// récuperation info
getInfo();
$.notify.defaults({ className: "success" });
$("#BoxInit").addClass("d-none");
});
