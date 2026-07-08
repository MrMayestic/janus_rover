#include "Arduino.h"
const char JOYSTICK_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no,maximum-scale=1">
    <title>Joystick</title>
    <style>
        :root {
            --accent: #c90101;
            --radius: 10px;
            --radius-lg: 14px;
            --shadow: 0 2px 10px rgba(0, 0, 0, 0.18);
            --shadow-hover: 0 4px 16px rgba(0, 0, 0, 0.25);
            --bg: #ffffff;
            --text: #111111;
            --toggle-bg: #2a2d36;
            --toggle-text: #ffffff;
            --measures-color: #111111;
            --button-bg: #d9d9d9;
            --button-bg-hover: #c4c4c4;
            --button-text: #111111;
            --joystick-bg: #d4d4d4;
            --joystick-knob: #ffffff;
        }

        body.dark-theme {
            --bg: #102130;
            --text: #ffffff;
            --toggle-bg: #ffffff;
            --toggle-text: #102130;
            --measures-color: #ffffff;
            --button-bg: #2c3e50;
            --button-bg-hover: #34495e;
            --button-text: #ffffff;
            --joystick-bg: #1c3a52;
            --joystick-knob: #dfe8ef;
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
            top: 6rem;
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
            max-width: 1400px;
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            align-items: flex-start;
            gap: 1.75rem;
        }

        .actionButton,
        .servoButton,
        #onOffVideo {
            color: var(--button-text);
            border: none;
            border-radius: var(--radius);
            background-color: var(--button-bg);
            box-shadow: var(--shadow);
            cursor: pointer;
            transition: background-color 0.15s ease, transform 0.1s ease, box-shadow 0.15s ease;
        }

        .actionButton:hover,
        .servoButton:hover,
        #onOffVideo:hover {
            background-color: var(--button-bg-hover);
            box-shadow: var(--shadow-hover);
        }

        .actionButton:active,
        .servoButton:active,
        #onOffVideo:active {
            transform: scale(0.95);
        }

        /* VIDEO PANEL */
        .video-panel {
            flex: 1 1 320px;
            max-width: 420px;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 0.85rem;
        }

        .video-frame {
            width: 100%;
            max-width: 380px;
            aspect-ratio: 4 / 3;
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
            transform: rotate(180deg);
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

        /* JOYSTICK PANEL */
        .joystick-panel {
            flex: 1 1 300px;
            max-width: 340px;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 1.1rem;
        }

        #joystick {
            position: relative;
            width: min(300px, 55vw);
            aspect-ratio: 1;
            background-color: var(--joystick-bg);
            border-radius: 50%;
            box-shadow: var(--shadow);
            touch-action: none;
        }

        #joyButton {
            position: absolute;
            top: calc(50% - 17.5px);
            left: calc(50% - 17.5px);
            width: 35px;
            height: 35px;
            border: none;
            border-radius: 50%;
            background-color: var(--joystick-knob);
            box-shadow: var(--shadow);
            cursor: grab;
        }

        #joyButton:active {
            cursor: grabbing;
        }

        .telemetry {
            width: 100%;
            display: flex;
            flex-direction: column;
            gap: 0.4rem;
            font-size: 1rem;
        }

        .telemetry-row {
            display: flex;
            justify-content: space-between;
            gap: 0.5rem;
            padding: 0.4rem 0.75rem;
            background-color: var(--button-bg);
            border-radius: var(--radius);
        }

        /* SERVO + MEASURES PANEL */
        .servo-panel {
            flex: 1 1 160px;
            max-width: 200px;
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 1.25rem;
        }

        #servoSter {
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

        #measures {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 1.5rem;
            color: var(--measures-color);
            font-size: clamp(1.2rem, 2.5vw, 1.7rem);
            font-weight: 500;
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
        }
    </style>
</head>

<body>
    <nav>
        <ul>
            <li><a href="index.html">Sterring</a></li>
            <li><a href="joystick.html">Joystick</a></li>
            <li>Data</li>
            <li class="nav-spacer">About</li>
        </ul>
    </nav>
    <button type="button" id="darkLight">&#9790;</button>
    <main>
        <div class="dashboard">
            <div class="video-panel">
                <div class="video-frame">
                    <img id="video" alt="Rover live feed" width="640" height="480" />
                </div>
                <button type="button" id="onOffVideo">Off</button>
            </div>
            <div class="joystick-panel">
                <div id="joystick">
                    <button type="button" id="joyButton"></button>
                </div>
                <div class="telemetry">
                    <div class="telemetry-row">
                        <span id="firstindex">RatioX:</span>
                        <span id="ratioX">null</span>
                    </div>
                    <div class="telemetry-row">
                        <span>RatioY:</span>
                        <span id="ratioY">null</span>
                    </div>
                    <div class="telemetry-row">
                        <span>Left:</span>
                        <span id="left">null</span>
                    </div>
                    <div class="telemetry-row">
                        <span>Right:</span>
                        <span id="right">null</span>
                    </div>
                </div>
            </div>
            <div class="servo-panel">
                <div id="servoSter">
                    <button type="button" id="servoPlus" class="servoButton">&#8593;</button>
                    <p id="servoInfo">Servo</p>
                    <button type="button" id="servoMinus" class="servoButton">&#8595;</button>
                </div>
                <div id="measures">
                    <p class="measure" id="temp">0&#176;C</p>
                    <p class="measure" id="humi">0%</p>
                </div>
            </div>
        </div>
    </main>
    <script>
        let streamOnToggle = false;

        const joyButton = document.querySelector("#joyButton");
        const joyArea = document.querySelector("#joystick");
        const onOffVideoBtn = document.querySelector("#onOffVideo");
        const videoEl = document.querySelector("#video");
        const darkLightBtn = document.querySelector("#darkLight");

        let joyAP = joyArea.getBoundingClientRect(); //Area Params
        let joyButtonAP = joyButton.getBoundingClientRect();

        let toCenterPos = joyButtonAP.width / 2;

        let currentL = 0;
        let currentR = 0;

        let server_ip = "change_this_ip";

        function isMobileOrTablet() {
            let check = false;
            (function (a) { if (/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows ce|xda|xiino|android|ipad|playbook|silk/i.test(a) || /1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(a.substr(0, 4))) check = true; })(navigator.userAgent || navigator.vendor || window.opera);
            return check;
        }

        async function sendData(what) {
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

                if (data.temperature > 0 && data.temperature < 50) {
                    document.querySelector("#temp").innerHTML = `${data.temperature}&#176;C`;
                }
                if (data.humidity > 0 && data.humidity <= 100) {
                    document.querySelector("#humi").innerHTML = `${data.humidity}%`;
                }
            } catch (err) {
                console.log("Failed to send data", err);
            }
        }

        function updateDisplay(event) {
            let mousX;
            let mousY;

            if (event.pageX) {
                mousX = event.clientX;
                mousY = event.clientY;
            } else if (event.touches[0].clientX) {
                mousX = event.touches[0].clientX;
                mousY = event.touches[0].clientY;
            }

            joyAP = joyArea.getBoundingClientRect(); //Area Params
            joyButtonAP = joyButton.getBoundingClientRect();

            toCenterPos = joyButtonAP.width / 2;

            if (mousX < joyAP.left + toCenterPos) {
                joyButton.style.left = `0px`;
                joyButton.style.right = "";
            } else if (mousX > joyAP.right - toCenterPos) {
                joyButton.style.right = `${joyAP.width - joyButtonAP.width}px`;
                joyButton.style.left = "";
            } else {
                joyButton.style.left = `${mousX - toCenterPos - joyAP.x}px`;
                joyButton.style.right = "";
            }

            if (mousY < joyAP.top + toCenterPos) {
                joyButton.style.top = `0px`;
            } else if (mousY > joyAP.bottom - toCenterPos) {
                joyButton.style.top = `${joyAP.height - joyButtonAP.height}px`;
            } else {
                joyButton.style.top = `${mousY - toCenterPos - joyAP.y}px`;
            }

            joyButtonAP = joyButton.getBoundingClientRect();

            let relX = (joyButtonAP.x + toCenterPos - joyAP.x) - (joyAP.width / 2);
            let relY = (joyButtonAP.y + toCenterPos - joyAP.y) - (joyAP.height / 2);

            let ratioX = relX / ((joyAP.width - (toCenterPos * 2)) / 2);
            let ratioY = relY / ((joyAP.height - (toCenterPos * 2)) / 2);

            document.querySelector("#ratioX").textContent = ratioX;
            document.querySelector("#ratioY").textContent = ratioY;

            let Xtoggle = true;
            let Ytoggle = true;

            if (ratioX < 0) {
                Xtoggle = false;
            }

            if (ratioY < 0) {
                Ytoggle = false;
            }

            ratioX = Math.abs(ratioX);
            ratioY = Math.abs(ratioY);

            let left = Math.floor((256 - (128 * (1 - Math.abs(ratioY))) - (128 * ratioX)) * ratioY);
            let right = Math.floor((ratioX > ratioY) ? 256 * ratioX : 256 * ratioY);

            let c;

            if (Xtoggle) {
                c = left;
                left = right;
                right = c;
            }

            if (Ytoggle) {
                left *= -1;
                right *= -1;
            }

            document.querySelector("#left").textContent = left;
            document.querySelector("#right").textContent = right;

            currentL = left;
            currentR = right;
        }

        function resetJoyButton() {
            currentL = 0;
            currentR = 0;

            joyButton.style.left = `${(joyAP.width / 2) - (joyButtonAP.width / 2)}px`;
            joyButton.style.top = `${(joyAP.height / 2) - (joyButtonAP.height / 2)}px`;
            joyButton.style.right = "";

            document.querySelector("#left").textContent = currentL;
            document.querySelector("#right").textContent = currentR;
        }

        sendData("joystickTrueWEB");

        function startStopStream() {
            if (streamOnToggle) {
                videoEl.src = "";
                onOffVideoBtn.classList.remove("live");
                onOffVideoBtn.textContent = "Off";
                streamOnToggle = false;
            } else {
                videoEl.src = `http://${server_ip}/video`;
                onOffVideoBtn.classList.add("live");
                onOffVideoBtn.textContent = "On";
                streamOnToggle = true;
            }
        }

        function changeSiteStyle() {
            const isDark = document.body.classList.toggle("dark-theme");
            darkLightBtn.innerHTML = isDark ? "&#9788;" : "&#9790;";
        }

        function desktopBinds() {
            let dragging = false;
            joyButton.addEventListener("mousedown", () => {
                dragging = true;
            });
            window.addEventListener("mousemove", (event) => {
                if (dragging) updateDisplay(event);
            });
            window.addEventListener("mouseup", () => {
                if (!dragging) return;
                dragging = false;
                resetJoyButton();
            });
        }

        function mobileBinds() {
            joyButton.addEventListener("touchstart", () => {
                joyAP = joyArea.getBoundingClientRect();
                document.body.style.overflowY = "hidden";
                window.addEventListener("touchmove", updateDisplay);
            });
            document.addEventListener("touchend", () => {
                document.body.style.overflowY = "auto";
                window.removeEventListener("touchmove", updateDisplay);
                resetJoyButton();
            });
        }

        function defaultBinds() {
            onOffVideoBtn.addEventListener("click", startStopStream);
            darkLightBtn.addEventListener("click", changeSiteStyle);

            document.querySelector("#servoPlus").addEventListener("click", () => sendData("servoplus"));
            document.querySelector("#servoMinus").addEventListener("click", () => sendData("servominus"));
        }

        window.onload = () => {
            console.log(isMobileOrTablet());
            if (isMobileOrTablet()) {
                mobileBinds();
            } else {
                desktopBinds();
            }
            defaultBinds();
            setInterval(() => sendData(`x${currentL}y${currentR}`), 100);
        };
        setInterval(() => sendData(`data`), 3000);
    </script>
</body>

</html> )=====";