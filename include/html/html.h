#pragma once

#ifndef __HTML_H_
#define __HTML_H_

const char html_setting[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1" charset="utf-8">
  <title>WIFI</title>
  <style>
    body { background-color:#f1f1f1; margin:0; padding:0; }
    .body_container { max-width:500px; margin:auto; padding:20px; background:#fff; border-radius:4px; box-shadow:0 2px 10px rgba(0,0,0,.1); }
    h1 { margin-top:0; text-align:center; }
    hr { border:none; border-top:1px solid #EAEAEA; }
    label { font-weight:bold; color:black; }
    table { width:100%; border-collapse:collapse; }
    th, td { text-align:left; padding:8px; }
    tr:nth-child(even){ background:#f2f2f2; }
    th { background:#00ABB3; color:#fff; }
    #wifiListTable th, #wifiListTable td { text-align:left; padding:8px; }
    #wifiListTable td:nth-child(1){ width:60%; } /* SSID */
    #wifiListTable td:nth-child(2){ width:20%; } /* dBm  */
    #wifiListTable td:nth-child(3){ width:20%; } /* Select */
    .current_value { color:#00ABB3; }
    .input_text { width:100%; padding:12px 20px; margin:8px 0; border:1px solid #ccc; border-radius:4px; box-sizing:border-box; }
    .button-container { display:flex; flex-direction:row; justify-content:space-between; width:100%; }
    .button { width:100%; padding:10px 20px; margin:8px 0; border:none; border-radius:4px; cursor:pointer; font-weight:bold; text-transform:uppercase; font-size:medium; }
    .button-mode { flex:1; background:#fff; color:#000; border:solid #EAEAEA .5px; padding:10px 0; border-radius:4px; text-align:center; text-decoration:none; font-weight:bold; text-transform:uppercase; cursor:pointer; transition:background-color .3s ease; }
    .button-green { background:#00ABB3; color:#fff; }
    .button-gray { background:#3C4048; color:#fff; }
    .button-mode:hover { background:#EAEAEA; }
    .box-2-left { margin-right:5px; }
    .box-2-right { margin-left:5px; }
    .info_container { display:flex; flex-direction:column; justify-content:space-between; }
    .info_box { background:#EAEAEA; padding:20px; border-radius:4px; box-sizing:border-box; flex:1; display:flex; flex-direction:column; align-items:center; text-align:center; box-shadow:0 2px 6px rgba(0,0,0,.1); }
    .info_box strong { font-weight:bold; font-size:medium; margin-bottom:10px; }
    .info_box .value { color:#333; font-size:x-large; }
    .info_box::before { content:''; display:block; width:50px; height:5px; background:#00ABB3; margin-bottom:10px; border-radius:4px; }
    #loadingMessage { display:block; text-align:center; }
    .modal { display:none; align-items:center; justify-content:center; position:fixed; z-index:1; left:0; top:0; width:100%; height:100%; overflow:auto; background:rgba(0,0,0,.4); }
    .modal-content { background:#fefefe; padding:20px; border:1px solid #888; width:80%; box-shadow:0 4px 8px rgba(0,0,0,.1); transform:translate(-50%,-50%); position:relative; top:50%; left:50%; }
    .close { color:#aaa; float:right; font-size:x-large; font-weight:bold; }
    .close:hover, .close:focus { color:black; text-decoration:none; cursor:pointer; }
    @media (max-width:320px){
      .info_container,.button-container,.select-container{ flex-direction:column; }
      .box-2-left{ margin-right:0; }
      .box-2-right{ margin-left:0; margin-top:10px; }
      .modal-content{ width:100%; }
    }
    @media screen and (min-width:600px){ .modal-content{ max-width:500px; } }
  </style>

  <script>
    var newSSID = "";
    var newPassword = "";

    function getColorForSignalStrength(dBm){
      if (dBm >= -50) return '#00C851';
      if (dBm >= -60) return '#ffbb33';
      if (dBm >= -70) return '#ff8800';
      return '#CC0000';
    }

    function getData(){
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function(){
        if (this.readyState == 4 && this.status == 200){
          var response = this.responseText.split(",");
          document.getElementById("ssid_wifi").innerHTML = response[0];
        }
      };
      xhttp.open("GET","/data",true);
      xhttp.send();
    }

    function postWiFi(ssid,password){
      var xhr = new XMLHttpRequest();
      xhr.open("POST","/updateWiFi",true);
      xhr.setRequestHeader("Content-type","application/x-www-form-urlencoded");
      var data = "new_ssid="+encodeURIComponent(ssid)+"&new_password="+encodeURIComponent(password);
      xhr.onreadystatechange = function(){
        if (xhr.readyState === 4){
          if (xhr.status === 200){ alert("Update WiFi successfully!"); }
          else { alert("Failed to update WiFi. Please try again."); }
        }
      };
      xhr.send(data);
    }

    function updateWiFi(){
      newPassword = document.getElementById("new_password").value;
      if (newSSID === "" || newPassword === ""){ alert("SSID or Password is empty!"); return; }
      postWiFi(newSSID,newPassword);
      getData();
      newSSID=""; newPassword="";
    }

    function updateWiFiManually(){
      newSSID = document.getElementById("new_ssid_manual").value;
      newPassword = document.getElementById("new_password_manual").value;
      if (newSSID === "" || newPassword === ""){ alert("SSID or Password is empty!"); return; }
      postWiFi(newSSID,newPassword);
      getData();
      newSSID=""; newPassword="";
    }

    function backNormalMode(){
      alert("Back to normal mode successfully!");
      var xhr = new XMLHttpRequest();
      xhr.open("POST","/normal",true);
      xhr.setRequestHeader("Content-type","application/x-www-form-urlencoded");
      xhr.onreadystatechange = function(){
        if (xhr.readyState == 4){ window.location.href = "about:blank"; }
      };
      xhr.send();
    }

    // --------- Rendering & Polling ----------
    function renderWifiList(networks){
      // Sort by SSID (stable)
      networks.sort(function(a,b){
        var A = (a.ssid || "").toUpperCase();
        var B = (b.ssid || "").toUpperCase();
        return A < B ? -1 : A > B ? 1 : 0;
      });

      var table = document.getElementById("wifiListTable");
      table.innerHTML = table.rows[0].outerHTML;

      for (var i=0;i<networks.length;i++){
        var row  = table.insertRow(-1);
        var c1   = row.insertCell(0);
        var c2   = row.insertCell(1);
        var c3   = row.insertCell(2);
        var ssid = networks[i].ssid || "";
        var rssi = networks[i].rssi;

        c1.textContent = ssid;
        c2.textContent = rssi;
        c2.style.color = getColorForSignalStrength(rssi);

        // SAFE: no inline quotes issues
        c3.innerHTML =
          '<button class="button button-gray" ' +
          'data-ssid="'+ encodeURIComponent(ssid) +'" ' +
          'onclick="selectNetwork(decodeURIComponent(this.dataset.ssid))">Select</button>';
      }
    }

    function pollScanWifi(maxTries, intervalMs){
      return new Promise(function(resolve,reject){
        var tries = 0;
        function tick(){
          var xhr = new XMLHttpRequest();
          xhr.open("GET","/scan_wifi",true);
          xhr.onreadystatechange = function(){
            if (xhr.readyState !== 4) return;

            if (xhr.status === 200){
              try { resolve(JSON.parse(xhr.responseText)); }
              catch(e){ reject(new Error("Invalid JSON")); }
              return;
            }
            if (xhr.status === 202){
              tries++;
              if (tries >= maxTries) reject(new Error("Scan timeout"));
              else setTimeout(tick, intervalMs);
              return;
            }
            reject(new Error("HTTP "+xhr.status));
          };
          xhr.send();
        }
        tick();
      });
    }

    function updateWiFiList(){
      var msg = document.getElementById("loadingMessage");
      msg.style.color = 'black';
      msg.innerText = "Scanning WiFi...";
      pollScanWifi(8, 1000)
        .then(function(networks){
          renderWifiList(networks);
          msg.style.color = networks.length > 0 ? 'green' : 'orange';
          msg.innerText   = networks.length > 0 ? "WiFi list updated." : "No networks found.";
        })
        .catch(function(err){
          console.error("scan error:", err);
          msg.style.color = 'red';
          msg.innerText   = "WiFi List Update Failed. Please Try Again.";
        });
    }

    function selectNetwork(ssid){
      newSSID = ssid;
      document.getElementById("modalSSID").innerText = ssid;
      var modal = document.getElementById("wifiPasswordModal");
      var span  = document.getElementsByClassName("close")[0];
      modal.style.display = "block";
      span.onclick = function(){ modal.style.display = "none"; };
      window.onclick = function(e){ if (e.target == modal) modal.style.display = "none"; };
    }
  </script>
</head>

<body>
  <div class="body_container">
    <h1>WIFI</h1>
    <hr>

    <div class="button-container">
      <a class="button-mode box-2-left" href="/">MONITOR</a>
      <a class="button-mode box-2-right" href="/wifi">WIFI</a>
    </div>
    <hr>

    <div class="info_container">
      <div class="info_box">
        <strong>WiFi SSID</strong>
        <span id="ssid_wifi" class="value">#NULL</span>
      </div>
    </div>
    <hr>

    <table id="wifiListTable">
      <tr>
        <th>SSID</th>
        <th>dBm</th>
        <th>Select</th>
      </tr>
    </table>

    <div id="wifiPasswordModal" class="modal">
      <div class="modal-content">
        <span class="close">&times;</span>
        <label for="new_password">Enter Password for <span id="modalSSID" class="current_value"></span></label><br>
        <input type="password" id="new_password" name="new_password" class="input_text" required maxlength="64">
        <button type="button" class="button button-green" onclick="updateWiFi()">Update WiFi</button>
      </div>
    </div>

    <button type="button" class="button button-green" onclick="updateWiFiList()">Scan WiFi</button>
    <div id="loadingMessage">Press button to scan WiFi.</div>
    <hr>

    <form onsubmit="return false;">
      <label for="new_ssid_manual">WiFi SSID:</label><br>
      <input type="text" id="new_ssid_manual" name="new_ssid_manual" class="input_text" required maxlength="32"><br>
      <label for="new_password_manual">WiFi Password:</label><br>
      <input type="password" id="new_password_manual" name="new_password_manual" class="input_text" required maxlength="64">
      <button type="button" class="button button-green" onclick="updateWiFiManually()">Update WiFi</button>
    </form>
    <hr>

    <button type="button" class="button button-gray" onclick="backNormalMode()">Back to Normal Mode</button>
  </div>

  <script>
    document.addEventListener('DOMContentLoaded', function () {
      getData();
    });
  </script>
</body>
</html>

)rawliteral";

const char html_monitor[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">

<head>
  <meta name="viewport" content="width=device-width, initial-scale=1" charset="utf-8">
  <title>MONITOR</title>

  <style>
    body {
      background-color: #f1f1f1;
      margin: 0;
      padding: 0;
    }

    .body_container {
      max-width: 500px;
      margin: auto;
      padding: 20px;
      background-color: #fff;
      border-radius: 4px;
      box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
    }

    h1 {
      margin-top: 0;
      text-align: center;
    }

    hr {
      border: none;
      border-top: 1px solid #EAEAEA;
    }

    label {
      font-weight: bold;
      color: black;
    }

    /* Minimalist button styles */
    .button-container {
      display: flex;
      flex-direction: row;
      justify-content: space-between;
      width: 100%;
    }

    .button {
      width: 100%;
      padding: 10px 20px;
      margin: 8px 0;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-weight: bold;
      text-transform: uppercase;

      font-size: medium;
    }

    .button-mode {
      flex: 1;
      background-color: white;
      color: black;

      border: solid #EAEAEA 0.5px;
      padding: 10px 0;
      border-radius: 4px;

      text-align: center;
      text-decoration: none;
      font-weight: bold;
      text-transform: uppercase;

      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    .button-mode:hover {
      background-color: #EAEAEA;
    }

    .box-2-left {
      margin-right: 5px;
    }

    .box-2-right {
      margin-left: 5px;
    }

    .box-3-left {
      margin-right: 5px;
    }

    .box-3-center {
      margin-left: 5px;
      margin-right: 5px;
    }

    .box-3-right {
      margin-left: 5px;
    }

    .button-gray {
      background-color: #3C4048;
      color: #fff;
    }

    .info_container {
      display: flex;
      flex-direction: row;
      justify-content: space-between;
    }

    .info_box {
      background-color: #EAEAEA;
      padding: 20px;
      border-radius: 4px;
      box-sizing: border-box;
      flex: 1;
      display: flex;
      flex-direction: column;
      align-items: center;
      text-align: center;
      box-shadow: 0 2px 6px rgba(0, 0, 0, 0.1);
    }

    .info_box strong {
      font-weight: bold;
      font-size: medium;
      margin-bottom: 10px;
    }

    .info_box .value {
      color: #333;
      font-size: x-large;
    }

    .info_box::before {
      content: '';
      display: block;
      width: 50px;
      height: 5px;
      background-color: #00ABB3;
      margin-bottom: 10px;
      border-radius: 4px;
    }

    .status_container {
      display: flex;
      flex-direction: row;
      justify-content: space-between;
    }

    .status_box {
      background-color: #00ABB3;
      color: white;

      padding: 20px;
      border-radius: 10px;
      box-sizing: border-box;
      flex: 1;
      display: flex;
      flex-direction: column;
      align-items: center;
      text-align: center;
      box-shadow: 0 2px 6px rgba(0, 0, 0, 0.1);
    }


    .status_box .value {
      color: white;

      font-weight: bold;
      font-size: 18px;
      font-size: x-large;

      margin-top: 10px;
      margin-bottom: 10px;
    }

    @media (max-width: 320px) {

      .button-container,
      .info_container {
        flex-direction: column;
      }

      .box-2-left {
        margin-right: 0;
      }

      .box-2-right {
        margin-left: 0;
        margin-top: 10px;
      }

      .box-3-left {
        margin-right: 0px;
      }

      .box-3-center {
        margin-left: 0px;
        margin-right: 0px;
        margin-top: 10px;
      }

      .box-3-right {
        margin-left: 0px;
        margin-top: 10px;
      }
    }

    @media screen and (min-width: 600px) {
      .modal-content {
        max-width: 500px;
      }
    }
  </style>

  <script>
    var fail = 0;

    function formatDuration(seconds) {
      var hours = Math.floor(seconds / 3600);
      var minutes = Math.floor((seconds % 3600) / 60);
      var remainingSeconds = seconds % 60;

      return (
        (hours < 10 ? "0" : "") + hours + ":" +
        (minutes < 10 ? "0" : "") + minutes + ":" +
        (remainingSeconds < 10 ? "0" : "") + remainingSeconds
      );
    }

    function getPowerMeter() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
          fail = 0;

          var response = this.responseText.split(",");
          document.getElementById("voltage").innerHTML = response[0] + " V";
          document.getElementById("current").innerHTML = response[1] + " A";
          document.getElementById("activePower").innerHTML = response[2] + " W";
          document.getElementById("activeEnergy").innerHTML = response[3] + " kWh";
          document.getElementById("powerFactor").innerHTML = response[4] + " %";

          var durationInSeconds = response[5];
          var formattedDuration = formatDuration(durationInSeconds);
          document.getElementById("duration").innerHTML = formattedDuration;
        }
        else {
          fail = fail + 1;

          if (fail == 10) {
            document.getElementById("voltage").innerHTML = "#FAIL";
            document.getElementById("current").innerHTML = "#FAIL";
            document.getElementById("activePower").innerHTML = "#FAIL";
            document.getElementById("activeEnergy").innerHTML = "#FAIL";
            document.getElementById("powerFactor").innerHTML = "#FAIL";
            document.getElementById("duration").innerHTML = "#FAIL";
          }
        }
      };
      xhttp.open("GET", "/PowerMeter", true);
      xhttp.send();
    }

    function backNormalMode() {
      alert("Back to normal mode successfully!");

      var xhr = new XMLHttpRequest();
      xhr.open("POST", "/normal", true);
      xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

      xhr.onreadystatechange = function () {
        if (xhr.readyState == 4) {
          if (xhr.status == 200) {
            console.log(xhr.responseText);
          }
          else {
            console.log("Error: [backNormalMode] " + xhr.status);
          }

          window.location.href = "about:blank";
        }
      };

      xhr.send();
    }
  </script>
</head>

<body>
  <div class="body_container">
    <h1>MONITOR</h1>
    <hr>

    <div class="button-container">
      <a class="button-mode box-2-left" href="/">MONITOR</a>
      <a class="button-mode box-2-right" href="/wifi">WIFI</a>
    </div>
    <hr>

    <div class="info_container">
      <div class="info_box box-2-left">
        <strong>Voltage</strong>
        <span id="voltage" class="value">#NULL</span>
      </div>

      <div class="info_box box-2-right">
        <strong>Current</strong>
        <span id="current" class="value">#NULL</span>
      </div>
    </div>

    <div class="info_container" style="margin-top: 10px;">
      <div class="info_box box-2-left">
        <strong>kW</strong>
        <span id="activePower" class="value">#NULL</span>
      </div>
      <div class="info_box box-2-right">
        <strong>kWh</strong>
        <span id="activeEnergy" class="value">#NULL</span>
      </div>
    </div>

    <div class="info_container" style="margin-top: 10px;">
      <div class="info_box box-2-left">
        <strong>PF</strong>
        <span id="powerFactor" class="value">#NULL</span>
      </div>

      <div class="info_box box-2-right">
        <strong>Duration</strong>
        <span id="duration" class="value">#NULL</span>
      </div>
    </div>

    <hr>

    <button type="button" class="button button-gray http-request-button" onclick="backNormalMode()">Back to Normal
      Mode</button>
  </div>

  <script>
    getPowerMeter();
    setInterval(getPowerMeter, 1000);
  </script>
</body>

</html>
)rawliteral";

const char html_normal[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1" charset="utf-8">

    <title>SETTING</title>

    <style>
        body {
            background-color: #f1f1f1;
            margin: 0;
            padding: 0;
            font-family: Arial;
        }

        .container {
            max-width: 500px;
            margin: auto;
            padding: 20px;
            background-color: #fff;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
        }

        h1 {
            margin-top: 0;
            text-align: center;
        }

        hr {
            border: none;
            border-top: 1px solid #EAEAEA;
        }
    </style>
</head>

<body>
    <div class="container">
        <h1>SETTING</h1>
        <hr>

        <p style="text-align: center;">
            Device is running normally!
            <br>
            Please <strong style="color: red;">disconnect</strong> this WiFi,
            and <strong style="color: red;">close</strong> this tab.
        </p>
    </div>
</body>

</html>
)rawliteral";

const char html_admin[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1" charset="utf-8">
    <title>ADMIN</title>

    <style>
        body {
            background-color: #f1f1f1;
            margin: 0;
            padding: 0;
            font-family: Arial;
        }

        .body_container {
            max-width: 500px;
            margin: auto;
            padding: 20px;
            background-color: #fff;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
        }

        h1 {
            margin-top: 0;
            text-align: center;
        }

        hr {
            border: none;
            border-top: 1px solid #EAEAEA;
        }

        label {
            font-weight: bold;
            color: black;
        }

        .current_value {
            color: #00ABB3;
            font-weight: bold;
        }

        .input_text {
            box-sizing: border-box;
            resize: vertical;
            width: 100%;
            padding: 12px 20px;
            margin: 8px 0;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
        }

        /* Minimalist button styles */
        .button-container {
            display: flex;
            flex-direction: row;
            justify-content: space-between;
            width: 100%;
        }

        .button-mode {
            flex: 1;
            background-color: white;
            color: black;

            border: solid #EAEAEA 1px;
            padding: 14px 0;
            /* margin: 0px; */
            border-radius: 5px;

            text-align: center;
            text-decoration: none;
            font-weight: bold;
            text-transform: uppercase;

            cursor: pointer;
            transition: background-color 0.3s ease;
        }

        .button-mode:hover {
            background-color: #EAEAEA;
        }

        .button {
            width: 100%;
            padding: 14px 20px;
            margin: 8px 0;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
            text-transform: uppercase;

            font-size: medium;
        }

        .button-green {
            background-color: #00ABB3;
            color: #fff;
        }

        .button-gray {
            background-color: #3C4048;
            color: #fff;
        }

        .info_container {
            display: flex;
            flex-direction: column;
            justify-content: space-between;
        }

        @media (max-width: 320px) {

            .button-container,
            .select-container {
                flex-direction: column;
            }
        }
    </style>

    <script>
        var id = "NULL";
        var station_code = "NULL";

        function getAdmin() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    var response = this.responseText.split(",");
                    console.log(response);

                    id = response[0];
                    station_code = response[1];

                    console.log(id);
                    console.log(station_code);

                    // Update HTML elements
                    document.getElementById("id").innerHTML = id;
                    document.getElementById("station_code").innerHTML = station_code;
                } else {
                    // Handle error or other status codes
                }
            };
            xhttp.open("GET", "/getAdmin", true);
            xhttp.send();
        }

        function postAdmin() {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/updateAdmin", true);
            xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

            // Prepare the data to send
            var data = "id=" + encodeURIComponent(id) + "&station_code=" + encodeURIComponent(station_code);

            console.log(data);

            xhr.onreadystatechange = function () {
                if (xhr.readyState === 4) {
                    if (xhr.status === 200) {
                        var response = xhr.responseText;
                        console.log(response); // Handle the response here if needed
                    } else {
                        console.error("Error: " + xhr.status + " - " + xhr.statusText);
                    }
                }
            };

            xhr.send(data);
        }

        function update() {
            // Get the values from input fields
            id = document.getElementById("new_id").value;
            station_code = document.getElementById("new_station_code").value;

            // Check if the values are valid floats or NULL
            var errorMessage = "";

            if (!/^\d{10}$/.test(id) && id.trim() !== "") {
                errorMessage += "ID must be a 10-digit number.";
            }

            if (station_code.trim() === "") {
                station_code = "NULL"
            }
            if (id.trim() === "") {
                id = "NULL";
            }
            if (id === "NULL" && station_code === "NULL") {
                errorMessage += "Please enter something!"
            }

            if (errorMessage !== "") {
                alert(errorMessage);
                return;
            }

            postAdmin();

            getAdmin(); // Refresh data after updating
        }
    </script>
</head>

<body>
    <div class="body_container">
        <h1>ADMIN</h1>
        <hr>

        <form onsubmit="event.preventDefault(); update();">
            <label for="new_id">ID: <span id="id" name="id" class="current_value">#NULL</span></label> <br>
            <input type="text" id="new_id" name="new_id" class="input_text" maxlength="10">
            <br>

            <label for="new_station_code">Station Code: <span id="station_code" name="station_code"
                    class="current_value">#NULL</span></label> <br>
            <input type="text" id="new_station_code" name="new_station_code" class="input_text" maxlength="16">
            <br>

            <button type="submit" class="button button-green">Update</button>
        </form>


        <hr>

        <form action="/normal" method="post">
            <button type="submit" class="button button-gray" onclick="backNormalMode()">Back to Normal Mode</button>
        </form>
    </div>

    <script>
        getAdmin();
    </script>
</body>

</html>
)rawliteral";

const char html_login[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1" charset="utf-8">
    <title>LOGIN</title>

    <style>
        body {
            background-color: #f1f1f1;
            margin: 0;
            padding: 0;
            font-family: Arial;
        }

        .body_container {
            max-width: 500px;
            margin: auto;
            padding: 20px;
            background-color: #fff;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
        }

        h1 {
            margin-top: 0;
            text-align: center;
        }

        hr {
            border: none;
            border-top: 1px solid #EAEAEA;
        }

        label {
            font-weight: bold;
            color: black;
        }

        .current_value {
            color: #00ABB3;
            font-size: 18px;
        }

        .input_text {
            box-sizing: border-box;
            resize: vertical;
            width: 100%;
            padding: 12px 20px;
            margin: 8px 0;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
        }

        /* Minimalist button styles */
        .button-container {
            display: flex;
            flex-direction: row;
            justify-content: space-between;
            width: 100%;
        }

        .button-mode {
            flex: 1;
            background-color: white;
            color: black;

            border: solid #EAEAEA 1px;
            padding: 14px 0;
            /* margin: 0px; */
            border-radius: 5px;

            text-align: center;
            text-decoration: none;
            font-weight: bold;
            text-transform: uppercase;

            cursor: pointer;
            transition: background-color 0.3s ease;
        }

        .button-mode:hover {
            background-color: #EAEAEA;
        }

        .button {
            width: 100%;
            padding: 14px 20px;
            margin: 8px 0;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
            text-transform: uppercase;

            font-size: medium;
        }

        .button-green {
            background-color: #00ABB3;
            color: #fff;
        }

        .button-gray {
            background-color: #3C4048;
            color: #fff;
        }

        .info_container {
            display: flex;
            flex-direction: column;
            justify-content: space-between;
        }

        .info_box {
            background-color: #EAEAEA;
            padding: 20px;
            border-radius: 10px;
            box-sizing: border-box;
            flex: 1;
            display: flex;
            flex-direction: column;
            align-items: center;
            text-align: center;
            box-shadow: 0 2px 6px rgba(0, 0, 0, 0.1);
        }

        .info_box strong {
            font-weight: bold;
            font-size: 18px;
            margin-bottom: 10px;
        }

        .info_box .value {
            font-size: 16px;
            color: #333;
            font-size: x-large;
        }

        .info_box::before {
            content: '';
            display: block;
            width: 50px;
            height: 5px;
            background-color: #00ABB3;
            margin-bottom: 10px;
            border-radius: 3px;
        }

        @media (max-width: 320px) {
            .info_container {
                flex-direction: column;
            }
        }
    </style>

    <script>
        function postLogin(url, username, password) {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", url, true);
            xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

            var data = "username=" + encodeURIComponent(username) + "&password=" + encodeURIComponent(password);

            xhr.onreadystatechange = function () {
                if (xhr.readyState === 4) {
                    if (xhr.status === 200) {
                        var response = xhr.responseText;
                        handleSuccess(response);
                    } else if (xhr.status === 401) {
                        handleFailure();
                    }
                }
            };

            xhr.send(data);
        }

        function handleSuccess(response) {
            console.log("Login successful");
            // Move to index.html
            window.location.href = '/admin';
        }

        function handleFailure() {
            console.log("Login failed");
            // You can display an error message or perform other actions here
            alert("Incorrect username or password");
        }

        function login() {
            var username = document.getElementById("username").value;
            var password = document.getElementById("password").value;

            postLogin("/checkLogin", username, password);
        }
    </script>

</head>

<body>
    <div class="body_container">
        <h1>LOGIN</h1>
        <hr>

        <form onsubmit="event.preventDefault(); login();">
            <label for="username">Username:
                <input type="text" id="username" name="username" class="input_text" required maxlength="32">
                <br>

                <label for="password">Password:</label><br>
                <input type="password" id="password" name="password" class="input_text" required maxlength="64" autocomplete="current-password">

                <button type="submit" class="button button-green">LOGIN</button>
        </form>

        <hr>

        <form action="/normal" method="post">
            <button type="submit" class="button button-gray" onclick="backNormalMode()">Back to Normal Mode</button>
        </form>
    </div>
</body>

</html>
)rawliteral";

#endif