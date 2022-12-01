#include "Arduino.h"
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
  <link rel="preconnect" href="https://fonts.googleapis.com" />
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
  <link href="https://fonts.googleapis.com/css2?family=Lato:wght@900&display=swap" rel="stylesheet" />
  <style>
    body {
      text-align: center;
      -webkit-touch-callout: none;
      -webkit-user-select: none;
      -khtml-user-select: none;
      -moz-user-select: none;
      -ms-user-select: none;
      user-select: none;
      transform: scale(1);
    }

    .button {
      color: black;
      text-align: center;
      width: 4vw;
      height: 4vw;
      margin: 0.15vw;
      user-select: none;
      border: 2px solid black;
      border-radius: 8px;
      background-color: #d1d1d1;
    }

    #content {
      margin: 1.5em;
    }

    .button:hover {
      background-color: #c0c0c0;
      cursor: pointer;
    }

    #precSter {
      display: inline-block;
    }

    #sterringButtons {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
    }

    #sterringButtons>button {
      margin: 0.5vw;
      background-color: rgb(67, 73, 88);
      color: white;
      height: 1.7vw;
      width: 9vw;
      border-radius: 0.5em;
    }

    #sterringButtons>div>button {
      margin: 0.5vw;
      background-color: rgb(67, 73, 88);
      color: white;
      height: 1.7vw;
      width: 8vw;
      border-radius: 0.5em;
    }

    .sterButton {
      margin: 5px;
    }

    #pstatus {
      margin: auto;
      width: 5em;
      height: 1em;
      background-color: #d1d1d1;
    }

    #sterring {
      display: inline-flex;
      flex-direction: row;
      align-items: center;
      justify-content: center;
      gap: 5em;
    }

    #info {
      margin-top: 3em;
    }

    #measures {
      font-size: 1.8vw;
      text-align: center;
    }

    .column {
      float: left;
      width: 50%;
    }

    .row:after {
      content: "";
      display: table;
      clear: both;
    }

    #temp {
      margin-left: 43vw;
    }

    #humi {
      margin-right: 43vw;
    }
  </style>
</head>

<body>
  <header>
    <h1 id="h1" style="font-family: 'Lato', sans-serif">ER Janusz Control</h1>
  </header>
  <main>
    <article id="upper-buttons">
      <button type="button" id="gosleep">GOSLEEP</button>
      <button type="button" id="upload" class="sterButton">UPLOAD</button>
      <br />
      <button type="button" id="darkLight" style="
            cursor: pointer;
            clear: both;
            text-align: center;
            width: 30px;
            height: 30px;
            background-color: #2a2d36;
            color: white;
          "> &#9790; </button>
    </article>
    <div id="content">
      <img id="images" style="transform: rotate(175deg)" />
    </div>
    <br />
    <article>
      <div id="sterring">
        <div id="arrowSter">
          <button type="button" id="forward" class="button">&#8593;</button>
          <br />
          <div id="downArrows">
            <button type="button" id="left" class="button">&#8592;</button>
            <button type="button" id="back" class="button">&#8595;</button>
            <button type="button" id="right" class="button">&#8594;</button>
          </div>
        </div>
        <div id="sterringButtons">
          <button type="button" id="precSter">PREC-STER</button>
          <p id="pstatus">none</p>
          <div>
            <button type="button" id="stop" class="sterButton">STOP</button>
            <button type="button" id="start" class="sterButton">START</button>
          </div>
          <div>
            <button type="button" id="servoplus" class="sterButton"> SERVOPLUS </button>
            <button type="button" id="servominus" class="sterButton"> SERVOMINUS </button>
          </div>
          <button type="button" id="joystickState">JOYSTICK OFF</button>
          <button type="button" id="sendData">Send Data</button>
        </div>
      </div>
      <br>
      <div id="measures">
        <div class="column">
          <p class="measure" id="temp">23</p>
        </div>
        <div class="column">
          <p class="measure" id="humi">35</p>
        </div>
      </div>
    </article>
  </main>
  <article>
    <div id="info">Info</div>
  </article>
  <script>
    let sendDataToggle = true;
    let style = "dark";
    let darkLight = document.getElementById("darkLight");
    let h1 = document.getElementById("h1");
    let joystickStateBool = false;
    let precSter = 0;

    function dewajs() {
      const ua = navigator.userAgent;
      if (/(tablet|ipad|playbook|silk)|(android(?!.*mobi))/i.test(ua)) {
        return "tablet";
      } else if (
        /Mobile|Android|iP(hone|od)|IEMobile|BlackBerry|Kindle|Silk-Accelerated|(hpw|web)OS|Opera M(obi|ini)/.test(
          ua
        )
      ) {
        return "mobile";
      }
      return "desktop";
    }

    const deviceType = dewajs();
    window.onload = function () {

      console.log(deviceType);

      function streamOff() {
        document.getElementById("content").innerHTML = " ";
      }

      function streamOn() {
        setTimeout(function () { }, 350);
        document.getElementById("content").innerHTML =
          '<img id="images" style="transform: rotate(180deg); margin-left: 3px;" src="http://192.168.1.44/video">';
      }

      function change() {
        if (style == "dark") {
          style = "bright";
          document.body.style.backgroundColor = "#2a2d36";
          darkLight.style.backgroundColor = "white";
          darkLight.style.color = "#2a2d36";
          darkLight.innerHTML = "&#9788";
          h1.style.color = "white";
          document.querySelector('#measures').style.color = "black";
        } else if (style == "bright") {
          style = "dark";
          document.body.style.backgroundColor = "white";
          darkLight.style.backgroundColor = "#2a2d36";
          darkLight.style.color = "white";
          darkLight.innerHTML = "&#9790";
          h1.style.color = "black";
          document.querySelector('#measures').style.color = "white";
        }
      }

      function sendData(what) {
        if (sendDataToggle) {
          sendDataToggle = false;
          document.getElementById("info").innerHTML = what;
          console.log(what);
          fetch("http://192.168.1.44/" + what, {
            method: "GET",
            mode: "cors",
            headers: {
              "Access-Control-Request-Method": "*",
              "Access-Control-Allow-Origin": "*",
              "Vary": "*",
            },
          })
            .then((response) => response.json())
            .then((data) => {
              console.log(data.temperature,data.humidity);
              if (data.temperature > 0 && data.temperature < 50) {
                document.querySelector("#temp").innerHTML = `${data.temperature}&#176;C`;
              }
              if (data.humidity > 0 && data.temperature <= 100) {
                document.querySelector("#humi").innerHTML = `${data.humidity}%`;
              }
            });

          setTimeout(function () { }, 50);
          sendDataToggle = true;
        }
        if (what == "upload") {
          streamOff();
          location.reload();
        }
      }

      function changePrecSter() {
        console.log(precSter);
        if (precSter == 0) {
          precSter = 1;
          document.getElementById("pstatus").innerHTML = "800ms";
          setTimeout(function () { }, 150);
        } else if (precSter == 1) {
          precSter = 2;
          document.getElementById("pstatus").innerHTML = "500ms";
          setTimeout(function () { }, 150);
        } else if (precSter == 2) {
          precSter = 3;
          document.getElementById("pstatus").innerHTML = "150ms";
          setTimeout(function () { }, 150);
        } else if (precSter == 3) {
          precSter = 4;
          document.getElementById("pstatus").innerHTML = "85ms";
          setTimeout(function () { }, 150);
        } else if (precSter == 4) {
          precSter = 0;
          document.getElementById("pstatus").innerHTML = "none";
          setTimeout(function () { }, 150);
        }
      }

      function checkLoaded() {
        setTimeout(function () { }, 50);
        return (
          document.readyState === "complete" ||
          document.readyState === "interactive"
        );
      }
      while (checkLoaded() != 1) {
        console.log("not loaded");
        setTimeout(function () { }, 100);
      }
      if (checkLoaded()) {
        console.log("loaded");
        document.getElementById("images").style.marginLeft = "1px";
        setTimeout(function () { }, 50);
        if (deviceType == "desktop") {
          document
            .getElementById("stop")
            .addEventListener("mousedown", function () {
              sendData("stop");
              streamOff();
            });
          document
            .getElementById("start")
            .addEventListener("mousedown", function () {
              sendData("start");
              streamOn();
            });

          // document.getElementById("video").addEventListener("mousedown", function() {sendData("video"); streamOn()});

          document
            .getElementById("upload")
            .addEventListener("mousedown", function () {
              sendData("upload");
            });

          document
            .getElementById("gosleep")
            .addEventListener("mousedown", function () {
              sendData("gosleep");
            });

          document
            .getElementById("darkLight")
            .addEventListener("mousedown", function () {
              change();
            });

          document
            .getElementById("precSter")
            .addEventListener("mousedown", function () {
              changePrecSter();
              setTimeout(function () { }, 150);
            });

          document
            .getElementById("servoplus")
            .addEventListener("mousedown", function () {
              sendData("servoplus");
            });
          document
            .getElementById("servominus")
            .addEventListener("mousedown", function () {
              sendData("servominus");
            });

          document
            .getElementById("joystickState")
            .addEventListener("mousedown", function () {
              if (joystickStateBool == false) {
                sendData("joystickFalse");
                joystickStateBool = true;
                document.getElementById("joystickState").innerHTML =
                  "JOYSTICK ON";
              } else {
                sendData("joystickTrue");
                joystickStateBool = false;
                document.getElementById("joystickState").innerHTML =
                  "JOYSTICK OFF";
              }
            });

          document
            .getElementById("forward")
            .addEventListener("mousedown", function () {
              if (precSter);
              else {
                sendData(1);
              }
            });
          document
            .getElementById("forward")
            .addEventListener("mouseup", function () {
              if (precSter == 1) sendData("lprec1");
              else if (precSter == 2) sendData("prec1");
              else if (precSter == 3) sendData("sprec1");
              else if (precSter == 4) sendData("uprec1");
              else {
                sendData(0);
              }
            });

          document
            .getElementById("left")
            .addEventListener("mousedown", function () {
              if (precSter);
              else {
                sendData(2);
              }
            });
          document
            .getElementById("left")
            .addEventListener("mouseup", function () {
              if (precSter == 1) sendData("lprec2");
              else if (precSter == 2) sendData("prec2");
              else if (precSter == 3) sendData("sprec2");
              else if (precSter == 4) sendData("uprec2");
              else {
                sendData(0);
              }
            });

          document
            .getElementById("back")
            .addEventListener("mousedown", function () {
              if (precSter);
              else {
                sendData(3);
              }
            });
          document
            .getElementById("back")
            .addEventListener("mouseup", function () {
              if (precSter == 1) sendData("lprec3");
              else if (precSter == 2) sendData("prec3");
              else if (precSter == 3) sendData("sprec3");
              else if (precSter == 4) sendData("uprec3");
              else {
                sendData(0);
              }
            });

          document
            .getElementById("right")
            .addEventListener("mousedown", function () {
              if (precSter);
              else {
                sendData(4);
              }
            });
          document
            .getElementById("right")
            .addEventListener("mouseup", function () {
              if (precSter == 1) sendData("lprec4");
              else if (precSter == 2) sendData("prec4");
              else if (precSter == 3) sendData("sprec4");
              else if (precSter == 4) sendData("uprec4");
              else {
                sendData(0);
              }
            });
          document
            .getElementById("sendData")
            .addEventListener("mouseup", function () {
              sendData("data");
            });
        } else {
          document
            .getElementById("stop")
            .addEventListener("touchstart", function () {
              sendData("stop");
              streamOff();
            });
          document
            .getElementById("start")
            .addEventListener("touchstart", function () {
              sendData("start");
              streamOn();
            });

          // document.getElementById("video").addEventListener("touchstart", function() {sendData("video"); streamOn()});

          document
            .getElementById("upload")
            .addEventListener("touchstart", function () {
              sendData("upload");
            });

          document
            .getElementById("gosleep")
            .addEventListener("touchstart", function () {
              sendData("gosleep");
            });

          document
            .getElementById("darkLight")
            .addEventListener("touchstart", function () {
              change();
            });

          document
            .getElementById("precSter")
            .addEventListener("touchstart", function () {
              changePrecSter();
              setTimeout(function () { }, 150);
            });

          document
            .getElementById("servoplus")
            .addEventListener("touchstart", function () {
              sendData("servoplus");
            });
          document
            .getElementById("servominus")
            .addEventListener("touchstart", function () {
              sendData("servominus");
            });

          document
            .getElementById("joystickState")
            .addEventListener("touchstart", function () {
              if (joystickStateBool == false) {
                sendData("joystickFalse");
                joystickStateBool = true;
              } else {
                sendData("joystickTrue");
                joystickStateBool = false;
              }
            });

          document
            .getElementById("forward")
            .addEventListener("touchstart", function () {
              if (precSter);
              else {
                sendData(1);
              }
            });

          document
            .getElementById("forward")
            .addEventListener("touchend", function () {
              if (precSter == 1) sendData("lprec1");
              else if (precSter == 2) sendData("prec1");
              else if (precSter == 3) sendData("sprec1");
              else if (precSter == 4) sendData("uprec1");
              else {
                sendData(0);
              }
            });

          document
            .getElementById("left")
            .addEventListener("touchstart", function () {
              if (precSter);
              else {
                sendData(2);
              }
            });
          document
            .getElementById("left")
            .addEventListener("touchend", function () {
              if (precSter == 1) sendData("lprec2");
              else if (precSter == 2) sendData("prec2");
              else if (precSter == 3) sendData("sprec2");
              else if (precSter == 4) sendData("uprec2");
              else {
                sendData(0);
              }
            });

          document
            .getElementById("back")
            .addEventListener("touchstart", function () {
              if (precSter);
              else {
                sendData(3);
              }
            });
          document
            .getElementById("back")
            .addEventListener("touchend", function () {
              if (precSter == 1) sendData("lprec3");
              else if (precSter == 2) sendData("prec3");
              else if (precSter == 3) sendData("sprec3");
              else if (precSter == 4) sendData("uprec3");
              else {
                sendData(0);
              }
            });

          document
            .getElementById("right")
            .addEventListener("touchstart", function () {
              if (precSter);
              else {
                sendData(4);
              }
            });
          document
            .getElementById("right")
            .addEventListener("touchend", function () {
              if (precSter == 1) sendData("lprec4");
              else if (precSter == 2) sendData("prec4");
              else if (precSter == 3) sendData("sprec4");
              else if (precSter == 4) sendData("uprec4");
              else {
                sendData(0);
              }
            });
          document
            .getElementById("sendData")
            .addEventListener("touchstart", function () {
              sendData("data");
            });
        }
      }
      streamOn();

      sendData(0);

      setInterval(function () { sendData("data"); }, 1500);
    };

    // setTimeout(function () { }, 100);
  </script>
</body>

</html> )=====";