#include "Arduino.h"
const char INDEX_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Sterring</title>
    <style>
        :root {
            --accent: #c90101;
            --accent-dark: #a10000;
            --radius: 10px;
            --radius-lg: 14px;
            --shadow: 0 2px 10px rgba(0, 0, 0, 0.18);
            --shadow-hover: 0 4px 16px rgba(0, 0, 0, 0.25);
            --bg: #ffffff;
            --text: #111111;
            --toggle-bg: #2a2d36;
            --toggle-text: #ffffff;
            --measures-color: #111111;
            --panel-bg: rgba(0, 0, 0, 0.03);
            --button-bg: #d9d9d9;
            --button-bg-hover: #c4c4c4;
            --button-text: #111111;
            --chip-bg: #dff0ff;
            --chip-bg-first: #fdffe0;
        }

        body.dark-theme {
            --bg: #102130;
            --text: #ffffff;
            --toggle-bg: #ffffff;
            --toggle-text: #102130;
            --measures-color: #ffffff;
            --panel-bg: rgba(255, 255, 255, 0.06);
            --button-bg: #2c3e50;
            --button-bg-hover: #34495e;
            --button-text: #ffffff;
            --chip-bg: #1c3a52;
            --chip-bg-first: #22405c;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: "Rubik", "Segoe UI", sans-serif;
            user-select: none;
        }

        html,
        body {
            height: 100%;
        }

        body {
            background-color: var(--bg);
            color: var(--text);
            transition: background-color 0.25s ease, color 0.25s ease;
            display: flex;
            flex-direction: column;
            min-height: 100vh;
        }

        a {
            text-decoration: none;
            color: inherit;
        }

        /* NAV */
        nav ul {
            list-style-type: none;
            display: flex;
            align-items: center;
            justify-content: center;
            flex-wrap: wrap;
            gap: 0.4rem;
            background-color: var(--accent);
            padding: 0.5rem;
            box-shadow: var(--shadow);
            position: sticky;
            top: 0;
            z-index: 10;
        }

        nav li {
            text-align: center;
            font-size: clamp(1.1rem, 2vw, 1.6rem);
            font-weight: 500;
            color: white;
            padding: 0.85rem 1.6rem;
            border-radius: 8px;
            cursor: pointer;
            transition: background-color 0.15s ease, transform 0.1s ease;
        }

        nav li:hover {
            background-color: rgba(0, 0, 0, 0.35);
        }

        nav li:active {
            transform: scale(0.97);
        }

        nav li.nav-spacer {
            margin-left: auto;
            cursor: default;
        }

        nav li.nav-spacer:hover {
            background-color: transparent;
        }

        #darkLight {
            cursor: pointer;
            width: 44px;
            height: 44px;
            border: none;
            border-radius: 50%;
            background-color: var(--toggle-bg);
            color: var(--toggle-text);
            font-size: 1.3rem;
            box-shadow: var(--shadow);
            transition: background-color 0.25s ease, color 0.25s ease, transform 0.15s ease;
            position: fixed;
            top: 8rem;
            right: 1rem;
            z-index: 20;
        }

        #darkLight:hover {
            transform: scale(1.08);
            filter: brightness(90%);
        }

        /* LAYOUT */
        main {
            flex: 1;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 1.75rem;
            padding: 1.75rem 1rem 2.5rem;
        }

        .dashboard {
            width: 100%;
            max-width: 1900px;
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            align-items: flex-start;
            gap: 1.75rem;
        }

        /* VIDEO PANEL */
        .video-panel {
            flex: 1 0 1280px;
            max-width: 1280px;
            width: 100%;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 0.85rem;
            padding-bottom: 3.5rem;
        }

        .video-frame {
            width: 100%;
            max-width: 1280px;
            aspect-ratio: 16 / 9;
            background-color: #000;
            border-radius: var(--radius-lg);
            overflow: hidden;
            box-shadow: var(--shadow);
        }

        #video {
            width: 100%;
            height: 100%;
            object-fit: contain;
            display: block;
        }

        .video-controls {
            display: flex;
            align-items: center;
            justify-content: center;
            flex-wrap: wrap;
            gap: 0.75rem;
        }

        #precSterInfo {
            font-size: 1.1rem;
        }

        /* STEERING ARROWS */
        .arrow-panel {
            flex: 0 0 220px;
            max-width: 260px;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 1.1rem;
            padding: 0.5rem;
            margin-right: 2.5rem;
        }

        #downArrows {
            display: flex;
            gap: 1rem;
        }

        .sterButton,
        .servoButton,
        .actionButton,
        #precSter,
        #onOffVideo {
            color: var(--button-text);
            border: none;
            border-radius: var(--radius);
            background-color: var(--button-bg);
            box-shadow: var(--shadow);
            cursor: pointer;
            transition: background-color 0.15s ease, transform 0.1s ease, box-shadow 0.15s ease;
        }

        .sterButton:hover,
        .servoButton:hover,
        .actionButton:hover,
        #precSter:hover,
        #onOffVideo:hover {
            background-color: var(--button-bg-hover);
            box-shadow: var(--shadow-hover);
        }

        .sterButton:active,
        .servoButton:active,
        .actionButton:active,
        #precSter:active,
        #onOffVideo:active {
            transform: scale(0.95);
        }

        .sterButton {
            width: clamp(64px, 9vw, 100px);
            height: clamp(64px, 9vw, 100px);
            font-size: 1.6rem;
        }

        /* SERVO CONTROL */
        .servo-panel {
            flex: 0 0 160px;
            max-width: 180px;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 0.5rem;
        }

        .servoButton {
            width: clamp(52px, 6vw, 72px);
            height: clamp(52px, 6vw, 72px);
            font-size: 1.3rem;
        }

        #servoInfo {
            font-weight: bold;
            font-size: 1.2rem;
            padding: 0.5rem 0;
        }

        #onOffVideo {
            min-width: 70px;
            padding: 0.6rem 0.9rem;
            font-size: 0.95rem;
            background-color: var(--accent);
            color: white;
        }

        #onOffVideo.live {
            background-color: #1f9e4f;
        }

        #precSter {
            min-width: 80px;
            padding: 0.6rem 0.9rem;
            font-size: 0.95rem;
        }

        /* MEASURES */
        #measures {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: clamp(1.5rem, 6vw, 4rem);
            color: var(--measures-color);
            font-size: clamp(1.4rem, 3vw, 2.2rem);
            font-weight: 500;
        }

        /* ACTION BUTTONS */
        .actions {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 0.85rem;
        }

        .actionButton {
            padding: 0.75rem 1.4rem;
            font-size: 1rem;
            font-weight: 500;
        }

        #info {
            opacity: 0.6;
            font-size: 0.85rem;
        }

        /* MOVE RESULTS */
        #moveResults {
            width: 100%;
            max-width: 1400px;
            display: flex;
            flex-direction: row;
            flex-wrap: wrap;
            justify-content: center;
            gap: 0.6rem;
        }

        .time {
            padding: 0.7rem 1rem;
            background-color: var(--chip-bg);
            border: 1px solid rgba(0, 0, 0, 0.15);
            font-size: 1.15rem;
            text-align: center;
            border-radius: var(--radius);
            box-shadow: var(--shadow);
        }

        .time:first-of-type {
            font-size: 1.6rem;
            background-color: var(--chip-bg-first);
        }

        @media only screen and (max-width: 1850px) {
            .video-panel {
                order: -1;
                flex-basis: 100%;
            }

            .arrow-panel,
            .servo-panel {
                flex-basis: auto;
            }

            .arrow-panel {
                margin-right: 0;
            }
        }

        @media only screen and (max-width: 960px) {
            .arrow-panel {
                margin-right: 0;
            }

            .video-panel {
                padding-bottom: 2rem;
            }
        }

        @media only screen and (max-width: 700px) {
            #darkLight {
                top: auto;
                bottom: 1rem;
                right: 1rem;
            }

            nav li {
                padding: 0.6rem 0.9rem;
            }

            .video-frame {
                border-radius: var(--radius);
            }
        }
    </style>
</head>

<body>
    <nav>
        <ul>
            <li><a href="index.html">Sterring</a></li>
            <li><a href="joystick.html">Joystick</a></li>
            <li><a href="data.html">Data</a></li>
            <li class="nav-spacer">About</li>
        </ul>
    </nav>
    <button type="button" id="darkLight">&#9790;</button>
    <main>
        <div class="dashboard">
            <div class="arrow-panel">
                <button type="button" id="forward" class="sterButton">&#8593;</button>
                <div id="downArrows">
                    <button type="button" id="left" class="sterButton">&#8592;</button>
                    <button type="button" id="back" class="sterButton">&#8595;</button>
                    <button type="button" id="right" class="sterButton">&#8594;</button>
                </div>
            </div>
            <div class="video-panel">
                <div class="video-frame">
                    <img id="video" alt="Rover live feed" width="1280" height="720" />
                </div>
                <div class="video-controls">
                    <button type="button" id="onOffVideo">Off</button>
                    <p id="precSterInfo">Precise Sterring:</p>
                    <button id="precSter" type="button">None</button>
                </div>
            </div>
            <div class="servo-panel">
                <div id="servoSter">
                    <button type="button" id="servoPlus" class="servoButton">&#8593;</button>
                    <p id="servoInfo">Servo</p>
                    <button type="button" id="servoMinus" class="servoButton">&#8595;</button>
                </div>
            </div>
        </div>

        <div id="measures">
            <p class="measure" id="temp">0&#176;C</p>
            <p class="measure" id="humi">0%</p>
            <p class="measure" id="voltage">0V</p>
        </div>

        <div class="actions">
            <button type="button" id="sendPhoto" class="actionButton">Send photo</button>
            <button type="button" id="lowEnergy" class="actionButton">Low Energy</button>
            <button type="button" id="normalEnergy" class="actionButton">Normal Energy</button>
            <button type="button" id="moveReq" class="actionButton">Move Results</button>
        </div>

        <p id="info">null</p>

        <div id="moveResults"></div>
    </main>
    <script>
        let precSter = 0;
        let precLetter = ""; //to usprawnić sending data
        let streamOnToggle = false;
        let server_ip = "192.168.11.48";

        const darkLightBtn = document.querySelector("#darkLight");
        const onOffVideoBtn = document.querySelector("#onOffVideo");
        const videoEl = document.querySelector("#video");
        const infoEl = document.querySelector("#info");

        function isMobileOrTablet() {
            let check = false;
            (function (a) { if (/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows ce|xda|xiino|android|ipad|playbook|silk/i.test(a) || /1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(a.substr(0, 4))) check = true; })(navigator.userAgent || navigator.vendor || window.opera);
            return check;
        }

        async function sendData(what) {
            console.log(what);
            infoEl.textContent = what;
            try {
                const response = await fetch(`http://${server_ip}/` + what, {
                    method: "GET",
                    mode: "cors",
                    headers: {
                        "Access-Control-Request-Method": "*",
                        "Access-Control-Allow-Origin": "*",
                        Vary: "*",
                    },
                });
                const data = await response.json();

                if (what == "moveResults") {
                    const resultsEl = document.querySelector("#moveResults");
                    resultsEl.innerHTML = "";
                    let times = data.data.split("|");
                    times = times.sort().reverse();
                    for (const n of times) {
                        if (n) {
                            const p = document.createElement("p");
                            p.className = "time";
                            p.textContent = n;
                            resultsEl.appendChild(p);
                        }
                    }
                } else {
                    if (data.temperature > 0 && data.temperature < 50) {
                        document.querySelector("#temp").innerHTML = `${data.temperature}&#176;C`;
                    }
                    if (data.humidity > 0 && data.humidity <= 100) {
                        document.querySelector("#humi").innerHTML = `${data.humidity}%`;
                    }
                    if (data.voltage > 0) {
                        document.querySelector("#voltage").innerHTML = `${data.voltage / 1000}V`;
                    }
                }
            } catch (err) {
                console.log("Failed to send data", err);
            }
        }

        function changeSiteStyle() {
            const isDark = document.body.classList.toggle("dark-theme");
            darkLightBtn.innerHTML = isDark ? "&#9788;" : "&#9790;";
        }

        function changePrecSter() {
            const precSterBtn = document.querySelector("#precSter");
            precSter = (precSter + 1) % 5;
            if (precSter === 1) {
                precLetter = "l";
                precSterBtn.textContent = "800ms";
            } else if (precSter === 2) {
                precLetter = "";
                precSterBtn.textContent = "500ms";
            } else if (precSter === 3) {
                precLetter = "s";
                precSterBtn.textContent = "150ms";
            } else if (precSter === 4) {
                precLetter = "u";
                precSterBtn.textContent = "85ms";
            } else {
                precLetter = "";
                precSterBtn.textContent = "None";
            }
        }

        function startStopStream() {
            if (streamOnToggle) {
                sendData("streamStop");
                videoEl.src = "";
                onOffVideoBtn.classList.remove("live");
                onOffVideoBtn.textContent = "Off";
                videoEl.style.visibility = "hidden";
                streamOnToggle = false;
            } else {
                videoEl.src = `http://${server_ip}/video`;
                onOffVideoBtn.classList.add("live");
                onOffVideoBtn.textContent = "On";
                videoEl.style.visibility = "visible";
                streamOnToggle = true;
            }
        }

        function translateSterrData(data) {
            return precSter > 0 ? precLetter + "prec" + String(data) : data;
        }

        function desktopBinds() {
            const bind = (id, code) => {
                const el = document.querySelector(id);
                el.addEventListener("mousedown", () => precSter == 0 ? sendData(code) : console.log("null"));
                el.addEventListener("mouseup", () => precSter == 0 ? sendData(0) : console.log("null"));
            };
            bind("#forward", 1);
            bind("#left", 2);
            bind("#back", 3);
            bind("#right", 4);
        }

        function mobileBinds() {
            const bind = (id, code) => {
                const el = document.querySelector(id);
                el.addEventListener("touchstart", () => precSter == 0 ? sendData(code) : console.log("null"));
                el.addEventListener("touchend", () => precSter == 0 ? sendData(0) : console.log("null"));
            };
            bind("#forward", 1);
            bind("#left", 2);
            bind("#back", 3);
            bind("#right", 4);
            document.querySelector("#sendPhoto").addEventListener("touchend", () => sendData("sendPhoto"));
        }

        function defaultBinds() {
            darkLightBtn.addEventListener("click", changeSiteStyle);
            onOffVideoBtn.addEventListener("click", startStopStream);
            document.querySelector("#precSter").addEventListener("click", changePrecSter);

            const clickBind = (id, code) => {
                document.querySelector(id).addEventListener("click", () => precSter > 0 ? sendData(translateSterrData(code)) : console.log("null"));
            };
            clickBind("#forward", 1);
            clickBind("#left", 2);
            clickBind("#back", 3);
            clickBind("#right", 4);

            document.querySelector("#servoPlus").addEventListener("click", () => sendData("servoplus"));
            document.querySelector("#servoMinus").addEventListener("click", () => sendData("servominus"));

            document.querySelector("#lowEnergy").addEventListener("click", () => sendData("lowEnergy"));
            document.querySelector("#normalEnergy").addEventListener("click", () => sendData("normalEnergy"));

            document.querySelector("#moveReq").addEventListener("click", () => sendData("moveResults"));
            document.querySelector("#sendPhoto").addEventListener("click", () => sendData("sendPhoto"));
        }

        sendData("joystickFalse");

        window.onload = () => {
            console.log(isMobileOrTablet());
            if (isMobileOrTablet()) {
                mobileBinds();
            } else {
                desktopBinds();
            }
            defaultBinds();
            setInterval(() => sendData("data"), 5000);
        };
    </script>
</body>

</html> )=====";