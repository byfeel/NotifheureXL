

var debug=true;
var maxdisplay;
var boutons=["Aucune","Afficher / Masquer les secondes","Afficher / Masquer l'horloge","Mode luminosite Mini / Maxi / Automatique","On / Off Veilleuse","Historique Message","Afficher / masquer Minuteur","lancer Minuteur","Action 1","Action 2"];
var nbClic=3;

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
        $("#inputhost").val(jinfo.SUFFIXEHOST);
        $('#DEBUG').prop('checked',jinfo.DEBUG);
        $('#AutoMsg').prop('checked',jinfo.AUTOLARGEURNOTIF);
        $('#bouton1').prop('checked',jinfo.BTN1);
        $('#bouton2').prop('checked',jinfo.BTN2);
        $('#dst').prop('checked',jinfo.DST);
        $("#inputNTP").val(jinfo.NTPSERVER);
        $("#CRTIME").val(jinfo.CRTIME);
        $("#btn1_1").val(jinfo.btnclic[0]);
        $("#btn1_2").val(jinfo.btnclic[1]);
        $("#btn1_3").val(jinfo.btnclic[2]);
        $("#btn2_1").val(jinfo.btnclic[3]);
        $("#btn2_2").val(jinfo.btnclic[4]);
        $("#btn2_3").val(jinfo.btnclic[5]);
    }
    } ,
      error: function(resultat,statut) {
         // init();
      },
      complete: function(resultat, statut){
        // init();
  }
  }); //fin ajax
}

function Reboot() {
var r = confirm("Etes vous sur de vouloir effectué un Reboot ?");
if (r == true) {
  Socket.send("REBOOT");
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
  $.post('/Config',
       {
  hostname:$("#inputhost").val(),
  debug:$('#DEBUG').prop('checked'),
  automsg:$('#AutoMsg').prop('checked'),
  btn1:$('#bouton1').prop('checked'),
  btn2:$('#bouton2').prop('checked'),
  dst:$('#dst').prop('checked')
//  nzo:$("#selectZone option:selected").val()
   }, function(data) {
  console.log(data);
   });
return false;
});

$( document ).ready(function() {
// init champs
//select Boutons
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

});
