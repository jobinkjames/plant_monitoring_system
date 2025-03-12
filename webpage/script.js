
        function currentTime() {
            let date = new Date();
            let hh = date.getHours();
            let mm = date.getMinutes();
            let ss = date.getSeconds();
            let session = "AM";
            let yy = date.getFullYear();
            const month = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
            let mon = month[date.getMonth()];
            let dd = date.getDate();
            let da = dd + " " + mon + " " + yy + " ";

            if (hh == 0) hh = 12;
            if (hh > 12) {
                hh = hh - 12;
                session = "PM";
            }

            hh = (hh < 10) ? "0" + hh : hh;
            mm = (mm < 10) ? "0" + mm : mm;
            ss = (ss < 10) ? "0" + ss : ss;

            let time = da + '\xa0\xa0\xa0\xa0\xa0\xa0\xa0' + hh + ":" + mm + ":" + ss + " " + session;
            document.getElementById("clock").innerText = time;
            setTimeout(currentTime, 1000);
        }
        currentTime();

        import { initializeApp } from "https://www.gstatic.com/firebasejs/9.22.2/firebase-app.js";
        import { getAnalytics } from "https://www.gstatic.com/firebasejs/9.22.2/firebase-analytics.js";
        import { getDatabase, ref, update } from "https://www.gstatic.com/firebasejs/9.22.2/firebase-database.js";

        const firebaseConfig = {
            apiKey: "AIzaSyAi0p43J0Q5ed228RWx9T0UGN1ZXlaHtqI",
            authDomain: "smart-es-8a1fc.firebaseapp.com",
            databaseURL: "https://smart-es-8a1fc-default-rtdb.firebaseio.com",
            projectId: "smart-es-8a1fc",
            storageBucket: "smart-es-8a1fc.appspot.com",
            messagingSenderId: "465348551850",
            appId: "1:465348551850:web:b456c9b4205381f425dcd3",
            measurementId: "G-LCYLRS9EHR"
        };

        const app = initializeApp(firebaseConfig);
        const analytics = getAnalytics(app);
        const db = getDatabase();

        var time = document.getElementById("time");
        var btn = document.getElementById("insert");

        function insertdata() {
            var timch = time.value;
            if (timch == "") {
                alert("Select Time!");
            } else {
                update(ref(db, "devices"), { time: timch })
                .then(() => {
                    location.reload(true);
                    alert("Time updated successfully.");                   
                })
                .catch((error) => {
                    alert("Time not inserted." + error);
                });
            }
        }
        btn.addEventListener('click', insertdata);

        firebase.initializeApp(firebaseConfig);
        var firebaseRef = firebase.database().ref("devices");
        firebaseRef.once("value", function (snapshot) {
            var data = snapshot.val();
            // document.getElementById("temp").innerText = data["Temperature"] + " °C";
            document.getElementById("humidity").innerText = data["Humidity"] + " %";
            document.getElementById("moisture").innerText = data["moistureValue"] + " %";
            document.getElementById("tt").innerText = data["time"];
            document.getElementById("temp").value = data["mode"];
        });

    lottie.loadAnimation({
        container: document.getElementById("temp-animation"),
        renderer: "svg",
        loop: true,
        autoplay: true,
        path: "https://lottie.host/b4f63fb6-4184-499b-bf3a-86b860c62a00/481ATa0gl5.json"
        // path: "https://lottie.host/d721f971-c21f-4261-94ef-92d2d1b3e5fd/6EuGUQOWUt.json"
    });

    lottie.loadAnimation({
        container: document.getElementById("humidity-animation"),
        renderer: "svg",
        loop: true,
        autoplay: true,
        path: "https://lottie.host/9289249c-6df4-4080-8269-3651bf0b0110/dw8X7RN9vr.json"
    });

    lottie.loadAnimation({
        container: document.getElementById("moisture-animation"),
        renderer: "svg",
        loop: true,
        autoplay: true,
        path: "https://lottie.host/6ff5ee78-a749-45aa-9223-add77eb0f3f2/JAquYAibMp.json"
    });
    lottie.loadAnimation({
        container: document.getElementById("select-animation"),
        renderer: "svg",
        loop: true,
        autoplay: true,
        path: "https://lottie.host/551fa15e-0e30-47f9-9633-328fc331bcf3/89FEGCMdoK.json"
    });

 function changeMode() {
    let modeSelector = document.getElementById("modeSelector");
    let selectedMode = modeSelector.value;
    
    // Mode names mapping
    let modeNames = {
        "1": "Automatic",
        "2": "Scheduled",
        "3": "Manual"
    };

    // Show confirmation popup
    let confirmChange = confirm(`Are you sure you want to switch to "${modeNames[selectedMode]}" mode?`);

    if (confirmChange) {
        // Change the mode
        document.querySelectorAll('.mode-content').forEach(el => el.style.display = 'none');
        document.getElementById('mode' + selectedMode).style.display = 'block';
    } else {
        // Revert to the previous selection
        modeSelector.value = modeSelector.dataset.previousValue || "1";
    }

    // Store the last selected value
    modeSelector.dataset.previousValue = selectedMode;
}

// Variables to track motor and humidifier state
let motorState = false; // false = OFF, true = ON
let humidifierState = false; // false = OFF, true = ON

// Function to toggle Motor ON/OFF
function toggleMotor() {
    motorState = !motorState;
    let motorButton = document.getElementById("motorButton");
    motorButton.innerText = motorState ? "Motor OFF" : "Motor ON";
    motorButton.className = motorState ? "btn btn-danger" : "btn btn-success";
}

// Function to toggle Humidifier ON/OFF
function toggleHumidifier() {
    humidifierState = !humidifierState;
    let humidifierButton = document.getElementById("humidifierButton");
    humidifierButton.innerText = humidifierState ? "Humidifier OFF" : "Humidifier ON";
    humidifierButton.className = humidifierState ? "btn btn-danger" : "btn btn-primary";
}


