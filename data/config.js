

var debug=true;
var maxdisplay;
var boutons=["Aucune","Afficher / Masquer les secondes","Afficher / Masquer l'horloge","Mode luminosite Mini / Maxi / Automatique","On / Off Veilleuse","Historique Message","Afficher / masquer Minuteur","lancer Minuteur","Action 1","Action 2"];
var couleurs=["Blanc","Rouge","Bleue","Vert","Jaune","Orange","Violet","Rose"];
var optLed=["Aucune","Led","Relais","Neopixel Ring/Strip"];
var fx=['flash','fade','raimbow','colorWipe ( Single LED )','colorWipe ( Full LED )','color chase'];
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
        $('#AutoMsg').prop('checked',jinfo.ALN);
        $('#bouton1').prop('checked',jinfo.BTN1).change();
        $('#bouton2').prop('checked',jinfo.BTN2).change();
        $('#dst').prop('checked',jinfo.DST);
        $("#inputNTP").val(jinfo.NTPSERVER);
        $("#CRTIME").val(jinfo.CRTIME);
        $("#CRTEXT").val(jinfo.MSGMINUT);
        $("#btn1_1").val(jinfo.btnclic[0]);
        $("#btn1_2").val(jinfo.btnclic[1]);
        $("#btn1_3").val(jinfo.btnclic[2]);
        $("#btn2_1").val(jinfo.btnclic[3]);
        $("#btn2_2").val(jinfo.btnclic[4]);
        $("#btn2_3").val(jinfo.btnclic[5]);
        $("#selectNotifLum").val(jinfo.TYPELED).change();

        $('#DispOff').val(String.fromCharCode(jinfo.CHAROFF));
        $('#intled').val(jinfo.FXINT).trigger("input");
        $('#intled').rangeslider('update', true);
        $('#selectled3_color').val(jinfo.FXCOLOR);
        $('#selectled3_fx').val(jinfo.CRFX);
        var charoff =jinfo.CHAROFF;
        if ( charoff == 32 ) $('#DispOff').attr("placeholder", "Espace");
        else if (cahroff == 0 ) $('#DispOff').attr("placeholder", "Vide");

    }
    } ,
      error: function(resultat,statut) {
         // init();
      },
      complete: function(resultat, statut){

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
  typeled:$('#selectNotifLum').val(),
  ntpserver:$('#inputNTP').val(),
  charoff:$('#DispOff').val().charCodeAt(0),
  crtext:$('#CRTEXT').val(),
  crtime:$('#CRTIME').val(),
  clicbtn1 : $('#btn1_1').val()+","+$('#btn1_2').val()+","+$('#btn1_3').val()+",",
  clicbtn2 : $('#btn2_1').val()+","+$('#btn2_2').val()+","+$('#btn2_3').val()+",",
  intled : $("#intled").val(),
  color : $("#selectled3_color").val(),
  fxcr :$("#selectled3_fx").val()
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
//visuelle
$('#selectNotifLum').change(function () {
          var check = $(this).val();

          if (check>0)  {
            $('#led3').removeClass("d-none");
            $("#infovisuel").text(optLed[check]);
              if (check==1 || check==3 ) $('#groupLED1').removeClass("d-none");
              else $('#groupLED1').addClass("d-none");
              if (check==3) $('#groupLED3').removeClass("d-none");
              else $('#groupLED3').addClass("d-none");
              if (check==2) $("#groupled0").removeClass("d-none");
              else $("#groupled0").addClass("d-none");
          }
          else $('#led3').addClass("d-none");
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

$.each(fx, function (value, text) {
  $('#selectled3_fx').append($('<option>', {
          value: value,
          text : text
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
