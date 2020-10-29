const char webPage[] PROGMEM = R"=====(

<html>
<style>
    body {
        background-color: #0d6fca;
        font-family: Arial, Helvetica, Sans-Serif;
        Color: #000088;
    }
</style>

<head>
    <title>electroPi NTP Clock</title>
</head>

<body>

    <h1>electroPi NTP Clock Setup</h1>


    <br>
    <fieldset>
        <legend> UTC time offset </legend>
        <form method='post' enctype='application/x-www-form-urlencoded' action='/postUTCTimeOffset/'>
            <label for="utcTimeOffset">UTC time offset (seconds (1hour = 60x60 = 3600))</label>
            <input type="number" min="1" max="900000" value="{{UTC_TIME_OFFSET}}" id='utcTimeOffset' name='utcTimeOffset'>
            <input type="submit">
        </form>
    </fieldset>
    <br>

    <fieldset>
        <legend>Hour Ring</legend>
        <input type='checkbox' id='clockEnable' name='clockEnable' onclick='window.location.assign("/toggleHours")' {{CLOCK_ENABLE_CHECKED}}>
        <label for='clockEnable'>Enable/Disable Hours strip</label>
        <br>
        <form method='post' enctype='application/x-www-form-urlencoded' action='/postHoursIntensity/'>
            <label for='hourStripIntensitySlider'>Intensity</label>
            <input type='range' min='1' max='100' value='{{HOUR_STRIP_INTENSITY_VALUE}}' class='slider' id='hourStripIntensitySlider' name='hourStripIntensitySlider'>
            <p>Value: <span id='hourStripIntensitySliderValue'></span></p>
            <input type='submit'>
            <script>
                var slider = document.getElementById('hourStripIntensitySlider');
                var output = document.getElementById('hourStripIntensitySliderValue');
                output.innerHTML = slider.value;
                slider.oninput = function() {
                    output.innerHTML = this.value;
                }
            </script>
        </form>
    </fieldset>
    <br>

    <fieldset>
        <legend>Minutes Ring</legend>
        <input type='checkbox' id='minutesEnable' name='minutesEnable' onclick='window.location.assign("/toggleMinutes")' {{MINUTES_ENABLED_CHECKED}}>
        <label for='minutesEnable'>Enable/Disable Minutes strip</label>
        <br>
        <form method='post' enctype='application/x-www-form-urlencoded' action='/postMinutesIntensity/'>
            <label for='minutesStripIntensitySlider'>Intensity</label>
            <input type='range' min='1' max='100' value='{{MINUTES_STRIP_INTENSITY_VALUE}}' class='slider' id='minutesStripIntensitySlider' name='minutesStripIntensitySlider'>
            <p>Value: <span id='minutesStripIntensitySliderValue'></span></p>
            <input type='submit'>
            <script>
                var minutesSlider = document.getElementById('minutesStripIntensitySlider');
                var output1 = document.getElementById('minutesStripIntensitySliderValue');
                output1.innerHTML = minutesSlider.value;
                minutesSlider.oninput = function() {
                    output1.innerHTML = this.value;
                }
            </script>
        </form>
    </fieldset>
    <br>

    <fieldset>
        <legend>Display</legend>
        <input type='checkbox' id='displayEnable' name='displayEnable' onclick='window.location.assign( "/toggleDisplay") ' {{DISPLAY_ENABLED_CHECKED}}>
        <label for='displayEnable'>Enable/Disable mini Display</label>
        <br>
        <input type='checkbox' id='displayEnableTime' name='displayEnableTime' onclick='window.location.assign( "/toggleDisplayEnableTime") ' {{DISPLAY_ENABLED_TIME_CHECKED}}>
        <label for='displayEnableTime'>Enable/Disable Time print on mini Display</label>
        <br>
        <input type='checkbox' id='displayEnableDate ' name='displayEnableDate' onclick='window.location.assign( "/toggleDisplayEnableDate") ' {{DISPLAY_ENABLED_DATE_CHECKED}}>
        <label for='displayEnableDate'>Enable/Disable Date print on mini Display</label>
        <br>
        <form method='post' enctype='application/x-www-form-urlencoded' action='/postDisplayIntensity/'>
            <label for='displayIntensitySlider'>Intensity</label>
            <input type='range' min='1' max='15' value='{{DISPLAY_INTENSITY_VALUE}}' class='slider' id='displayIntensitySlider' name='displayIntensitySlider'>
            <p>Value: <span id='displayIntensitySliderValue'></span></p>
            <input type='submit'>
            <script>
                var displaySlider = document.getElementById('displayIntensitySlider');
                var output2 = document.getElementById('displayIntensitySliderValue');
                output2.innerHTML = displaySlider.value;
                displaySlider.oninput = function() {
                    output2.innerHTML = this.value;
                }
            </script>
        </form>

        <form method='post' enctype='application/x-www-form-urlencoded' action='/postDisplayScrollSpeed/'>
            <label for='scrollSpeed'>Scrolling speed (ms)</label>
            <input type="number" min='0' max='500' step='5' value="{{SCROLL_SPEED_VALUE}}" id='scrollSpeed' name='scrollSpeed'>
            <input type='submit'>
        </form>

    </fieldset>
    <br>



    <fieldset>
        <legend> Automatic night settings </legend>
        <input type="checkbox" id="enAuto" name="enAuto" onchange="document.getElementById('autoChSubmit').disabled = !this.checked;" {{AUTO_NIGHT_MODE_CHECKED}}>
        <label for="enAuto">Enable/Disable</label>
        <form method='post' enctype='application/x-www-form-urlencoded' action='/postAutoDim/'>
            <label for='startTime'>Night-mode starts at:</label>
            <input type='time' id='startTime' name='startTime' value="{{AUTO_TIME_START_TIME}}">
            <label for='endTime'>Night-mode ends at:</label>
            <input type='time' id='endTime' name='endTime' value="{{AUTO_TIME_END_TIME}}">
            <input type='submit' name="autoChSubmit" id="autoChSubmit" disabled>
        </form>
    </fieldset>
    <br>

    <br>
    <button onclick='window.location.assign("/reboot")'>Reboot</button>
    <button onclick='window.location.assign("/poweroff")'>Power Off</button>
    <button onclick='window.location.assign("/saveEEPROM")'>Save to EEPROM</button>
    <br>

</body>

</html>

)=====";
