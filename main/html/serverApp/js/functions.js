$('#navbar a').click(function (e) {
	e.preventDefault();
	$(this).tab('show');
});

$('#settings_basic_testconnections').on('click', function() {
  var btn = $(this).button('loading');
  var ip = $("#settings_basic_esp32IP_input").val();
  var ajaxreq = ESP32_GET(ip, "status",
    function(data) {
      //console.log(ajaxreq);
      if (ajaxreq.status == 200) { //OK
        addWarningBefore($('#settings_basic_testconnection_div'), "success", "connection verified, status: " + data);
      }
      //console.log(ajaxreq.status);
      btn.button('reset');
    },
    function(err) {
      console.error("settings_basic_testconnections error");
      console.log(err);
      addWarningBefore($('#settings_basic_testconnection_div'), "warning", "connection failed, error code: " + ajaxreq.status);
      btn.button('reset');
    });
  setCookie('esp32IP', ip);
});

$("[href='#settings']").on('click', function() {
  //loading data of settings page
  var ip = getCookie('esp32IP');
  $("#settings_basic_esp32IP_input").val(ip);
});

function setCookie(c_name,value,expiredays)
{
  var exdate = new Date();
  exdate.setDate(exdate.getDate()+expiredays);
  document.cookie=c_name+ "=" +escape(value)+
    ((expiredays==null) ? "" : ";expires="+exdate.toGMTString());
};

function getCookie(c_name) {
  if (document.cookie.length>0) {
    c_start=document.cookie.indexOf(c_name + "=");
    if (c_start!=-1)
    {
      c_start=c_start + c_name.length+1;
      c_end=document.cookie.indexOf(";",c_start);
      if (c_end==-1) c_end=document.cookie.length;
      return unescape(document.cookie.substring(c_start,c_end));
    }
  }
  return "";
};

$(document).ready(function(){
    $("#turnOnLight").click(function() {
      turnOn = $.ajax({url:"/util/turnOnLight",async:false});
      $("#turnonoffreturn").html(turnOn.responseText);
    });
    $("#turnOffLight").click(function() {
      turnOff = $.ajax({url:"/util/turnOffLight",async:false});
      $("#turnonoffreturn").html(turnOff.responseText);
    });
});

function ESP32_GET(ip, cmd, success, error) {
  var myurl = "http://" + ip + "/util/" + cmd;
  var ajaxobj = $.ajax({
    url: myurl,
    type: "GET",
    timeout: 3000,
    success: success,
    error: error,
  });
  return ajaxobj;
};

function addWarningBefore(obj, type, str) {
  //type could be "success", "info", "warning", "danger"
  obj.before("<div id='settings_basic_testconnectionstatus' " +
  "class='alert alert-" + type + " alert-dismissible' role='alert'><button type='button' class='close' " +
  "data-dismiss='alert' aria-label='Close'><span aria-hidden='true'>&times;</span></button><strong>" +
  type + "!</strong> " + str + " </div>");
}
