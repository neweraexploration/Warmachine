//Last Update:23;jan;2024
//working on web monitoring data
//working on reset engage data
//working on programable map mode
//working on code optimization 

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

#include <ESP32Servo.h>
#include <iostream>
#include <sstream>

#include "soc/rtc_wdt.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

// ESPNOW
#include <WiFi.h>
#include <esp_now.h>


String myData = "";
// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x08, 0xD1, 0xF9, 0x6B, 0x21, 0xF0};

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
// ESPNOW

//Motion 
const int trigPin = 2; // Trigger pin of the ultrasonic sensor
const int echoPin = 4; // Echo pin of the ultrasonic sensor
//const int motionSensorPin = 22;
//int motionState;
bool guard_flag=false;
//Motion Sensor

//Auto home mode
const int basehomebtn = 25;
const int armhomebtn = 21;
#define basehomeservo_PIN 26
Servo basehomeservo;
bool basehome_flag = false;
bool armhome_flag = false;
//Auto home mode

//gun
#define gun_SERVO_PIN 27
//gun

//Map mode
bool mapmode = false;
unsigned long prevRequestTime = 0;
bool firstRequest = true;
int value;
byte seq = 0;  //stores the current number of executed sequences
byte delay_Seq = 0;
byte seq_Array[50];  // array to store the movement sequence in terms of integers(1 for FWD, 2 for LEFT and so on..)
unsigned long delay_Array[50];
//Map mode

int target_hit=1;
int target=0;


struct MOTOR_PINS {
  int pinEn;
  int pinIN1;
  int pinIN2;
};

std::vector<MOTOR_PINS> motorPins = {
  { 22, 16, 17 },  //RIGHT_MOTOR Pins (EnA, IN1, IN2)
  { 23, 18, 19 },  //LEFT_MOTOR  Pins (EnB, IN3, IN4)
};

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define STOP 0

#define RIGHT_MOTOR 0
#define LEFT_MOTOR 1

#define FORWARD 1
#define BACKWARD -1

const int PWMFreq = 1000; /* 1 KHz */
const int PWMResolution = 8;
const int PWMSpeedChannel = 4;

const char *ssid = "WARMACHINE";
const char *password = "warmachine1234";

AsyncWebServer server(80);
AsyncWebSocket wsCarInput("/CarInput");

const char *htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>War Machine Main Frame</title>
    <link rel="stylesheet" href="style.css">
    <script src="script.js"></script>
<style>
*{
    margin: 0;
    padding: 0;
    background-color: #161616;
}
#nav-bar{
    width: 100%;
    height: 75px;
    display: flex;
    align-items: center;
    justify-content: center;
}
.nav-container{
    font-size: 55px;
    font-weight: 800;
    color: #B6BBC4;
    text-shadow: 4px 4px 10px black;
}



#second-bar{
    width: 100%;
    height: 250px;
    display: flex;
    justify-content: center;
}
.video-headline{
    margin: 3px 0;
    text-align: center;
    color: #B6BBC4;
    font-size: 18px;
}
.video-container{
    margin: 12px;
    flex: 1;
    display: flex;
    flex-direction: column;
    box-shadow: 0 2px 4px 2px #888888;
}

.video-box{
    /* padding: 5px; */
    margin: 0 5px;
    /* height: 190px; */  
}

.connect{
    height: 80px;
    flex: 1;
    display: flex;
    align-items: center;
    justify-content: space-around;
}
.connect input{
    padding-left: 5px;
    text-decoration: none;
    border: none;
    border-bottom: 2px solid #888888;
    background: none;
    color: white;
}
input::placeholder{
    color: white;
}
.connect button{
    width: 90px;
    height: 20px;
    font-size: 16px;
    background-color: #4993FA;
}
.video-container1{
    margin: 12px;
    flex: 1;
    display: flex;
    flex-direction: column;
    box-shadow: 0 2px 4px 2px #888888;
}
.Data-container{
    margin: 12px;
    flex: 3;
    display: flex;
    flex-direction: column;
    box-shadow: 0 2px 4px 2px #888888;
}
#esp-data{
    margin:12px 10px;
}
#esp-data textarea{
    color: green;
}
#guard-mode-status {
    color: green;
}




#third-bar{
    width: 100%;
    height: 435px;
    display: flex;
}
.left{
    flex: 1;
    display: flex;
    flex-direction: column;
    margin: 5px 10px;
}
.mode1{
    flex: 1;
    display: flex;
    justify-content: center;
    align-items: center;
}
.mode2{
    flex: 1;
    display: flex;
    justify-content: center;
    align-items: center;
}
.mode3{
    flex: 1;
    display: flex;
    justify-content: center;
    align-items: center;
}
.daba1{
    width: 260px;
    height: 120px;
    display: flex;
    justify-content: center;
    align-items: center;
    font-size: 24px;
    font-weight: 600;
    background-color: green;
    border: 2px solid black;
    box-shadow: 5px 5px #888888;
}


.daba2{
    width: 260px;
    height: 120px;
    display: flex;
    justify-content: center;
    align-items: center;
    font-size: 24px;
    font-weight: 600;
    background-color: #FF1700;
    border: 2px solid black;
    box-shadow: 5px 5px #888888;
}

.daba2.active {
    background-color: green; /* Color when active */
}

.daba3{
    width: 260px;
    height: 120px;
    display: flex;
    justify-content: center;
    align-items: center;
    font-size: 24px;
    font-weight: 600;
    background-color: #FF1700;
    border: 2px solid black;
    box-shadow: 5px 5px #888888
}
.daba3.active {
    background-color: green; /* Color when active */
}
.middle{
    flex: 3;
    display: flex;
    flex-direction: column;
    color: #B6BBC4;
}
.headline{
    flex: .5;
    display: flex;
    justify-content: center;
    align-items: center;
    font-size: 24px;
    font-weight: 600;
    text-shadow: 2px 2px 5px black;

}

.dots-container {
    margin: 20px 30px;
    display: flex;
    flex-direction: column;
    justify-content: space-between;
    /* width: 400px; */

  }
  
  .dotr {
    width: 30px;
    height: 30px;
    border-radius: 50%;
    background-color: red;
    animation: blink 2s infinite ;
  }
  .dotg {
    width: 30px;
    height: 30px;
    border-radius: 50%;
    background-color: green;
    animation: blink 2.5s infinite ;/* Blinking animation*/
  }

  
  @keyframes blink {
    0% {
      opacity: 1; /* Fully visible */
    }
    100% {
      opacity: 0.4; /* Fully transparent */
    }
  }
  
  

  /* Delay the animation of each dot */
  .dotr:nth-child(1) {
    animation-delay: 0s;
  }
  
  .dotr:nth-child(2) {
    animation-delay: 0.1s;
  }
  
  .dotg:nth-child(3) {
    animation-delay: 0.3s;
  }
  
  .dotg:nth-child(4) {
    animation-delay: 0.1s;
  }
  
  .doty:nth-child(5) {
    animation-delay: 0s;
  }
  .doty:nth-child(5) {
    animation-delay: 0.1s;
  }

.subpart{
    flex: 3;
    display: flex;
    align-items: center;
    justify-content: center;
}

.mainTable{
    display: flex;
    flex-direction: row;
    align-items: center;
    justify-content: center;
}

  .arrows {
    font-size:40px;
    color:red;
    background-color: black;
  }
  .label {
    font-size:20px;
    color:red;
    background-color: black;
  }
  

  .button {
    display: inline-block;
  }
  td.button {
    cursor: pointer;
    color: black;
    background-color:black;
    border-radius:25%;
    box-shadow: 5px 5px #888888;
    width: 100px;
  }
  td.button:active {
    transform: translate(5px,5px);
    box-shadow: none; 
  }
  td.button1 {
    cursor: pointer;
    color: black;
    background-color:black;
    border-radius:25%;
    box-shadow: 5px 5px #888888;
    width: 120px;
    height: 40px;
  }
  td.button1:active {
    transform: translate(5px,5px);
    box-shadow: none; 
  }


.right{
    flex: 1;
    display: flex;
    flex-direction: column;
    margin: 5px 10px;
}
.button{
    flex: 1.5;
    display: flex;
    align-items: center;
    justify-content: space-around;
}
.button .btn{
    width: 200px;
    height: 90px;
    background-color: #4993FA;
    font-size: 18px;
    font-weight: 600;
    box-shadow: 5px 5px #888888;
}

.target-container{
    flex: 3;
    display: flex;
    flex-direction: column;
}
.target-above{
    flex: 1;
    display: flex;
    justify-content: space-around;
    align-items: flex-start;
}
.target-item{
    width: 110px;
    height: 70px;
    display: flex;
    justify-content: center;
    align-items: center;
    font-size: 20px;
    font-weight: 600;
    border: 2px solid #888888;
    box-shadow: 2px 2px #888888;
    color: #B6BBC4;
}
.target-middle{
    flex: 1;
    display: flex;
    justify-content: space-around;
    align-items: flex-start;
}
.target-below{
    flex: 1;
    display: flex;
    justify-content: space-around;
    align-items: flex-start;
}



.popup {
    position: fixed;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    background-color: white;
    padding: 20px;
    border: 2px solid black;
    z-index: 9999;
    display: none;
}

/* Center the login form */
.login-popup {
    position: fixed;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    background-color: rgba(0, 0, 0, 0.8); /* Semi-transparent background */
    width: 100%;
    height: 100%;
    z-index: 9999;
    display: flex;
    justify-content: center;
    align-items: center;
}

/* Style the login container */
.login-container {
    background-color: #fff; /* White background */
    padding: 20px;
    border-radius: 5px;
    width: 300px;
    max-width: 90%; /* Responsive design, adjust width based on screen size */
    text-align: center; /* Center the content */
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.2); /* Box shadow for a subtle effect */
}

/* Style the input fields */
.id,
.pass {
    width: 100%;
    margin-bottom: 10px;
    padding: 10px;
    color: white;
    border: 1px solid #ccc; /* Border color */
    border-radius: 3px;
    box-sizing: border-box; /* Ensure padding and border are included in the width */
}

/* Style the login button */
.btn1 {
    width: 100%;
    padding: 10px;
    background-color: #007bff; /* Blue background */
    color: #fff; /* White text color */
    border: none;
    border-radius: 3px;
    cursor: pointer;
    transition: background-color 0.3s; /* Smooth transition for hover effect */
}

/* Hover effect for the login button */
.btn1:hover {
    background-color: #0056b3; /* Darker blue on hover */
}

/* Style for the placeholder text */
.id::placeholder,
.pass::placeholder {
    transition: color 0.3s ease; /* Smooth transition for placeholder text color */
}

/* Faded placeholder text when input is focused */
.id:focus::placeholder,
.pass:focus::placeholder {
    color: transparent; /* Make the placeholder text transparent */
}


.unselectable {
  -webkit-user-select: none; /* Chrome, Safari, Opera */
  -moz-user-select: none;    /* Firefox */
  -ms-user-select: none;     /* Internet Explorer/Edge */
  user-select: none;         /* Non-prefixed version, currently supported by most browsers */
}



</style>
</head>
<body>
    <div id="nav-bar">
        <div class="nav-container">WAR MACHINE</div>
    </div>
   
    <div id="second-bar">
        <div class="video-container">
            <div class="video-headline">War-Eye</div>
            <div class="video">
                <div class="video-box">
                    <iframe id="stream" width="345" height="165"></iframe>
                </div>
            </div>
            <div class="connect">
                <input type="text" id="ipInput" placeholder="IP Address" value="192.168.4.2">
                <button type="submit" onclick="changeIP()" >connect</button>
            </div>
        </div>
        <div class="video-container1">
            <div class="video-headline">Esp-Eye</div>
            <div class="video">
                <div class="video-box">
                    <iframe id="stream1" width="345" height="165"></iframe>
                </div>
            </div>
            <div class="connect">
                <input type="text" id="ipInput1" placeholder="IP Address" value="192.168.4.3">
                <button type="submit" onclick="changeIP1()" >connect</button>
            </div>
        </div>
        <div class="Data-container">
            
                <!-- //Communication through esp -->
                <div id="esp-data">
                    <textarea id="esp-data-textarea" rows="13" cols="88" readonly></textarea>
                    
                </div>
                <!-- //Communication through esp             -->
        </div>
        
    </div>
    <div id="third-bar">
        <div class="left">
            <div class="connect">
              <input type="text" id="guarddata" value="1240">
              <button class="button1 unselectable" ontouchstart='sendButtonInput("sendmap","0")' 
              onmousedown='sendButtonInput("sendmap","0")'>Get MAP</button>
            </div>
            <div class="mode1">
                <div class="daba1">Control Mode</div>
            </div>
            <div class="mode2">
                <div class="daba2">Map Mode</div>
            </div>
            <div class="mode3">
                <div class="daba3">Guard Mode</div>
            </div>
        </div>
        <div class="dots-container">
            <div class="dotr"></div>
            <div class="dotr"></div>
            <div class="dotr"></div>
            <div class="dotg"></div>
            <div class="dotg"></div>
            <div class="dotg"></div>
          </div>
        <div class="middle">   
            <div class="headline">War Control</div>
            <div class="subpart" >
                <table id="mainTable" style="width:600px;margin:auto;table-layout:fixed" CELLSPACING=10>
                      <tr style="text-align:center">
                        <td ></td>
                        <td ></td>
                        <td class="button unselectable" ontouchstart='sendButtonInput("MoveCar","1")' ontouchend='sendButtonInput("MoveCar","0")' 
                                           onmousedown='sendButtonInput("MoveCar","1")' onmouseup='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8679;</span></td>
                        <td></td>
                      </tr>
                      <tr style="text-align:center">
                        <td ></td>
                        <td class="button unselectable" ontouchstart='sendButtonInput("MoveCar","3")' ontouchend='sendButtonInput("MoveCar","0")'
                                           onmousedown='sendButtonInput("MoveCar","3")' onmouseup='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8678;</span></td>
                        <td></td>    
                        <td class="button unselectable" ontouchstart='sendButtonInput("MoveCar","4")' ontouchend='sendButtonInput("MoveCar","0")'
                                           onmousedown='sendButtonInput("MoveCar","4")' onmouseup='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8680;</span></td>
                      </tr>
                      <tr style="text-align:center">

                        <td ></td>
                        <td ></td>
                        <td class="button unselectable" ontouchstart='sendButtonInput("MoveCar","2")' ontouchend='sendButtonInput("MoveCar","0")'
                                           onmousedown='sendButtonInput("MoveCar","2")' onmouseup='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8681;</span></td>
                        <td></td>
                      </tr>
                      <tr><tr/>
                      <tr><tr/>

                        <tr style="text-align:center">

                            <td class="button1 unselectable"  ontouchstart='sendButtonInput("addmark","9")' 
                            onmousedown='sendButtonInput("gun","0")' onmouseup='sendButtonInput("gun","90")'><span class="label" >Fire</span></td>
                            <td colspan=1>
                                <td class="button1 unselectable" ontouchstart='sendButtonInput("home","1")' 
                            onmousedown='sendButtonInput("home","1")'><span class="label" >Autohome</span></td>
                            <td colspan=1>
                                <td class="button1 unselectable" ontouchstart='sendButtonInput("reset","1")' 
                            onmousedown='sendButtonInput("reset","1")'><span class="label" >Reset</span></td>
         
                          </tr>
                          <tr><tr/>
                            <tr><tr/>
                          <tr style="text-align:center">
                            <td class="button1 unselectable" ontouchstart='sendButtonInput("mapmode","0")' 
                            onmousedown='sendButtonInput("mapmode","0")' onclick="changeModeColor()"><span class="label" >MapMode</span></td>
                            <td></td>
                            <td class="button1 unselectable" onclick="addMark()" ontouchstart='sendButtonInput("addmark","9")' 
                            onmousedown='sendButtonInput("addmark","9")'><span class="label" >AddMark</span></td>
                            <td></td>
                            <td class="button1 unselectable" ontouchstart='sendButtonInput("engage","0")' 
                            onmousedown='sendButtonInput("engage","0")'><span class="label" >Engage</span></td>
         
                          </tr>
                          

                     
                        
                        <td style="text-align:left;font-size:25px"><b>Speed:</b></td>
                        <td>
                         <div class="slidecontainer">
                            <input type="range" min="0" max="255" value="100" class="slider" id="Speed" oninput='sendButtonInput("Speed",value)'>
                          </div>
                        </td>
                        
                      </tr>       
                    </table>
            </div>
        </div>
        <div class="dots-container">
            <div class="dotr"></div>
            <div class="dotr"></div>
            <div class="dotr"></div>
            <div class="dotg"></div>
            <div class="dotg"></div>
            <div class="dotg"></div>
          </div>
        <div class="right">
            <div class="button">
                <button type="button" class="btn unselectable" onclick="guard()" >Guard Mode</button>
                
                <button type="button" class="btn unselectable" ontouchstart='sendButtonInput("guardmode","0")' 
                            onmousedown='sendButtonInput("guardmode","0")' >Capture Distance</button>
            </div>
            <div class="target-container">
                    <div class="target-above">
                    <div class="target-item" id="target1">Target 1</div>
                    <div class="target-item"id="target2">Target 2</div>
                </div>
                <div class="target-middle">
                    <div class="target-item"id="target3">Target 3</div>
                    <div class="target-item"id="target4">Target 4</div>
                </div>
                <div class="target-below">
                    <div class="target-item"id="target5">Target 5</div>
                    <div class="target-item"id="target6">Target 6</div>
                </div>
            </div>
           
        </div>
    </div>

    <div class="login-popup" id="loginPopup">
        <div class="login-container">
            <input type="text" placeholder="LOGIN ID" class="id" onfocus="placeholderFade(this)">
            <input type="password" placeholder="PASSWORD" class="pass" onfocus="placeholderFade(this)">
            <button type="button" class="btn1" onclick="attemptLogin()">Login</button>
        </div>
    </div> 
    
<script>
let guardflag=1;
let currentTarget = 1;
let hittarget = 1;
let mapModeActive = false; // Flag to indicate if MapMode is active

var webSocketCarInputUrl = "ws:\/\/" + window.location.hostname + "/CarInput"; 
var websocketCarInput;

function initCarInputWebSocket() {
  websocketCarInput = new WebSocket(webSocketCarInputUrl);
  websocketCarInput.onopen = function (event) {
    var speedButton = document.getElementById("Speed");
    sendButtonInput("Speed", speedButton.value);
    changeIP();
  };
  websocketCarInput.onclose = function (event) {
    guardflag=1;
    alert("Disconnected From the Server");
    setTimeout(initCarInputWebSocket, 2000);
  };
  //Communication through esp
  websocketCarInput.onmessage = function (event) {
    var data = event.data;
    console.log("Received message: " + data);
    if (data == "Engaged Completed MAP Mode OFF") {
      var daba2 = document.querySelector(".daba2");

  // Toggle the 'active' class to change the color
  daba2.classList.toggle("active");

  mapModeActive = !mapModeActive; // Toggle the mapModeActive flag
      //changecolour function
      targethit();
    }
    else if (data == "Recieving request From ESP-EYE to Deactivated Guardmode"){
      var daba3 = document.querySelector(".daba3");
  // Toggle the 'active' class to change the color
    daba3.classList.toggle("active");
    }else if(data === "Map Mode ON" || data === "Map Mode OFF"){
      var daba2 = document.querySelector(".daba2");

  // Toggle the 'active' class to change the color
  daba2.classList.toggle("active");

  mapModeActive = !mapModeActive; // Toggle the mapModeActive flag
    }

    if (guardflag == 0) {
      var guarddata = document.getElementById("guarddata").value;
      
      // Convert string numerical data to numbers
      var numericGuardData = Number(guarddata);
      var numericData = Number(data);
      
      // Ensure the conversion is successful
      if (!isNaN(numericGuardData) && !isNaN(numericData)) {
          if (numericData <= numericGuardData) {
              sendButtonInput("alert", "0");
              var textarea = document.getElementById("esp-data-textarea");
              textarea.value +="Intruder Alert" + "\n";
              textarea.scrollTop = textarea.scrollHeight;
          }
      } else {
          console.log("Invalid numerical input");
      }
  }

    var textarea = document.getElementById("esp-data-textarea");
    textarea.value += data + "\n";
    textarea.scrollTop = textarea.scrollHeight;
  };
  //Communication through esp
}
function guard(){
  var textarea = document.getElementById("esp-data-textarea");
    
  if (guardflag === 0) {
    guardflag = 1;
    textarea.value += "Guard Mode is Deactivated" + "\n";
    textarea.scrollTop = textarea.scrollHeight;
} else {
    guardflag = 0;
    textarea.value += "Guard Mode is Activated" + "\n";
    textarea.scrollTop = textarea.scrollHeight;
}

    // Get the .daba3 container
    var daba3 = document.querySelector(".daba3");

    // Toggle the 'active' class to change the color
    daba3.classList.toggle("active");
     
}

function sendButtonInput(key, value) {
  var data = key + "," + value;
  websocketCarInput.send(data);
}

function changeIP() {
  var ip = document.getElementById("ipInput").value;
  var streamFrame = document.getElementById("stream");
  streamFrame.src = "http://" + ip + "/";
}

function changeIP1() {
  var ip = document.getElementById("ipInput1").value;
  var streamFrame = document.getElementById("stream1");
  streamFrame.src = "http://" + ip + "/";
}

window.onload = initCarInputWebSocket;
document
  .getElementById("mainTable").addEventListener("touchend", function (event) {
    event.preventDefault();
  });


function changeModeColor() {
  // Get the .daba2 container
  
}

function changeModeColor1() {

}

function addMark() {
  // Check if MapMode is active
  if (mapModeActive) {
      const targetId = "target" + currentTarget;
      const targetElement = document.getElementById(targetId);
  
      if (targetElement) {
          targetElement.style.backgroundColor = "#D71313";
          targetElement.textContent = "Lock";
          currentTarget = (currentTarget % 6) + 1; // Loop back to 1 after reaching 6
      }
  } else {
      // Display a message or handle the scenario when MapMode is not active
      // alert("MapMode is not active. Cannot add mark.");
  }
}


function targethit() {
  
  for(i=1;i<currentTarget;i++){
  const targetId = "target" + i;
  const targetElement = document.getElementById(targetId);
  if (targetElement) {
    targetElement.style.backgroundColor = "green";
    targetElement.textContent = "Hit";
  }
}
currentTarget=1;
}

function attemptLogin() {
  // Retrieve login credentials
  var id = document.querySelector(".id").value;
  var pass = document.querySelector(".pass").value;

  // Check if both ID and password are 'admin', if yes, grant access
  if (id.trim() === "admin" && pass.trim() === "admin") {
    sendButtonInput("initialized", 1);
    // For simplicity, assume login is successful
    // Hide the login popup
    document.getElementById("loginPopup").style.display = "none";
    // Show the rest of the content
    document.getElementById("content").style.display = "block";
    
  } else if (id.trim() === "" && pass.trim() === "") {
    // Call the popup function if login credentials are not entered
    alert("Please Login to access this Function");
  } else {
    // Show an alert for incorrect ID or password
    alert("Incorrect ID or password. Please try again.");
    // Clear the input fields
    document.querySelector(".id").value = "";
    document.querySelector(".pass").value = "";
  }
}
 
</script>
</body>
</html>
)HTMLHOMEPAGE";



//Map mode  function start

void updatearray(int valueInt) {
  seq_Array[seq] = valueInt;
  seq++;
}
void updatedelay(unsigned long valuedelay) {
  delay_Array[delay_Seq] = valuedelay;
  delay_Seq++;
}
void sendmap(){
String data = "";
   int end = seq;

  for (int i = 0; i < end; i++) {
    if (seq_Array[i] == 9) {
      data += "seq:9,delay:1500;";  // Collect data
    } else if (seq_Array[i] == 0) {
      data += "seq:0,delay:0;";  // Collect data
    } else {
      data += "seq:" + String(seq_Array[i]) + ",delay:" + String(delay_Array[i]) + ";";  // Collect data
    }
  }

  // Send the concatenated data string
  if(data!=""){wsCarInput.textAll(data);}else{wsCarInput.textAll("No Map Found");}
}

void resetmapmode(){
   wsCarInput.textAll("Reseting Previous MAP");
  // mapmode=false;
  for(int i = 0; i < 50; i++) {
    delay_Array[i] = -1;
    seq_Array[i] = 0;
  }
  
  // Reset the sequence counters to 0
  delay_Seq = 0;
  seq = 0;
  firstRequest = true;
  // wsCarInput.textAll("Reset Complete");
}

void fire(int value) {
  if(value == 0){
    Serial.println(value);
    digitalWrite(gun_SERVO_PIN, HIGH);
  }
  else if (value=90){
    digitalWrite(gun_SERVO_PIN, LOW);
    Serial.println(value);
  }
  else{
    digitalWrite(gun_SERVO_PIN, LOW);
  }
}


//map mode functions end
void delayMilliseconds(unsigned long milliseconds) {

  unsigned long start = millis();
  while (millis() - start < milliseconds) {
    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;
    TIMERG0.wdt_wprotect = 0;

    // Allow other tasks to execute during this delay
    // Or perform non-blocking operations if needed
    // Example: Serial communication, checking sensors, etc.
    yield();  // This allows the ESP32 to service background tasks
  }
}

void rotateMotor(int motorNumber, int motorDirection) {
  if (motorDirection == FORWARD) {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
  } else if (motorDirection == BACKWARD) {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);
  } else {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
  }
}
/*
webdata(int sensorvalue){
  String sensorData = "sensor1: " + String(sensorvalue);
  webSocket.sendTXT(sensorData);
}
*/

void moveCar(int inputValue) {
  switch (inputValue) {
    case UP:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      break;
    case DOWN:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      break;
    case LEFT:
      rotateMotor(LEFT_MOTOR, BACKWARD);
      break;
    case RIGHT:
      rotateMotor(LEFT_MOTOR, FORWARD);
      break;
    case STOP:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);
      break;
    default:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);
      break;
  }
}

void engage() {
  int end = seq;
  for (int i = 0; i < end; i++) {
    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;
    TIMERG0.wdt_wprotect = 0;
    yield(); 
    if (seq_Array[i] == 9) {
    fire(0);
    delayMilliseconds(1500);
    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;
    TIMERG0.wdt_wprotect = 0;
    yield(); 

    fire(90);
    // target++;
    } else if(seq_Array[i] == 0) {
      moveCar(seq_Array[i]); 
    }else{
      moveCar(seq_Array[i]);
      delayMilliseconds(delay_Array[i]);
      Serial.print("Delay :");
      Serial.println(delay_Array[i]);
      Serial.print("value :");
      Serial.println(seq_Array[i]);
    } 
  }
  // target=0;
  // target_hit=1;

  wsCarInput.textAll("Engaged Completed MAP Mode OFF");
  mapmode = false;
}



void autohome(){
//add infinite loop saftey trigger
//add base servo controls  --done--
  wsCarInput.textAll("Auto home Sequence Started");
  ledcWrite(PWMSpeedChannel, 90);
  basehomeservo.write(110);
  while(basehome_flag==false){
    rotateMotor(LEFT_MOTOR, FORWARD);
    if (digitalRead(basehomebtn) == LOW){
      rotateMotor(LEFT_MOTOR, BACKWARD);
      delayMilliseconds(500);
      rotateMotor(LEFT_MOTOR, STOP);
      basehome_flag=true;
    }
  }
  basehomeservo.write(0);
  delayMilliseconds(1000);
  while(armhome_flag==false){
    rotateMotor(RIGHT_MOTOR, BACKWARD);
    if (digitalRead(armhomebtn) == LOW){
      rotateMotor(RIGHT_MOTOR, FORWARD);
      delayMilliseconds(1200);
      rotateMotor(RIGHT_MOTOR, STOP);
      armhome_flag=true;
    }
  }
  basehome_flag=false;
  armhome_flag=false;
  ledcWrite(PWMSpeedChannel, 100);
  wsCarInput.textAll("Auto home Sequence Finshed");
}

void initialized(){

  String mac = WiFi.softAPmacAddress();
  wsCarInput.textAll("Initialized Sequence Start\nChecking Services\nBoard Voltage: 5.1V\nDevice MAC-Address : " + mac);
  
  // ledcWrite(PWMSpeedChannel, 70);
  moveCar(3);
  delay(500);
  moveCar(4);
  delay(500);
  moveCar(1);
  delay(500);
  moveCar(2);
  delay(500);
  moveCar(0);
  // while(armhome_flag==false){
  //   rotateMotor(RIGHT_MOTOR, BACKWARD);
  //   if (digitalRead(armhomebtn) == LOW){
  //     moveCar(4);
  //     rotateMotor(RIGHT_MOTOR, FORWARD);
  //     delayMilliseconds(1000);
  //     rotateMotor(RIGHT_MOTOR, STOP);
  //     armhome_flag=true;
  //   }
  // }
  // armhome_flag=false;
  // moveCar(0);
  // ledcWrite(PWMSpeedChannel, 100);
}


void handleRoot(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "File Not Found");
}

void onCarInputWebSocketEvent(AsyncWebSocket *server,
                              AsyncWebSocketClient *client,
                              AwsEventType type,
                              void *arg,
                              uint8_t *data,
                              size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      moveCar(STOP);
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo *)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {


        std::string myData = "";
        myData.assign((char *)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');

        int valueInt = atoi(value.c_str());


        if (mapmode == true) {
          if (key == "MoveCar") {
            updatearray(valueInt);
            unsigned long currentTime = millis();
            unsigned long delay = 0;
            if (!firstRequest) {
              delay = currentTime - prevRequestTime;
              updatedelay(delay);
              Serial.print("Delay :");
              Serial.println(delay);
            }
            prevRequestTime = currentTime;
            firstRequest = false;
            Serial.print("value :");
            Serial.println(valueInt);
            moveCar(valueInt);
          } else if (key == "addmark") {
            updatearray(valueInt);
            updatedelay(111);
          } else if (key == "Speed") {
            ledcWrite(PWMSpeedChannel, valueInt);
          } else if (key == "gun") {
            Serial.println(valueInt);
            fire(valueInt);
          }else if(key == "mapmode")
          {
            mapmode = false;
            wsCarInput.textAll("Map Mode OFF");
          } else if (key == "engage") {
            // wsCarInput.textAll("Map Mode OFF");
            engage();
          }
          else if (key == "home") {
            autohome();
          }
          else if(key == "guardmode"){
            
            if(guard_flag==true)
            {guard_flag=false;
              wsCarInput.textAll("STOP Capturing Distance");
            }
            else{guard_flag=true;wsCarInput.textAll("Start Capturing Distance");}
          }
          else if(key == "initialized"){
            Serial.println("initialised on");
            initialized();
          }
          else if(key=="alert"){
            fire(0);
            delay(2000);
            fire(90);
          }
          else if(key=="reset"){
            resetmapmode();
          }else if(key=="sendmap"){
            sendmap();
          }
        } else if (mapmode == false) {
          if (key == "MoveCar") {
            moveCar(valueInt);
          } else if (key == "Speed") {
            ledcWrite(PWMSpeedChannel, valueInt);
          } else if (key == "gun") {
            Serial.println(valueInt);
            fire(valueInt);
          } else if (key == "mapmode") {
            mapmode = true;
            wsCarInput.textAll("Map Mode ON");
          }
          else if (key == "home") {
            autohome();
          }
          else if(key=="sendmap"){
            sendmap();
          }
          else if(key == "guardmode"){
            
            if(guard_flag==true)
            {guard_flag=false;
              wsCarInput.textAll("STOP Capturing Distance");
            }
            else{guard_flag=true;wsCarInput.textAll("Start Capturing Distance");}
          }
          else if(key=="initialized"){
            
            initialized();
          }
          else if(key=="alert"){
            fire(0);
            delay(2000);
            fire(90);
          }
          else if(key=="reset"){
            resetmapmode();
          }
        }
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;
  }
}

void setUpPinModes() {
  //Set up PWM
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //pinMode(motionSensorPin, INPUT);
  pinMode(gun_SERVO_PIN, OUTPUT);

  //auto home mode
  pinMode(basehomebtn, INPUT_PULLUP);
  pinMode(armhomebtn, INPUT_PULLUP);
  //basehomeservo.attach(basehomeservo_PIN);
  //auto home mode
  ledcSetup(PWMSpeedChannel, PWMFreq, PWMResolution);

  for (int i = 0; i < motorPins.size(); i++) {
    pinMode(motorPins[i].pinEn, OUTPUT);
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);

    /* Attach the PWM Channel to the motor enb Pin */
    ledcAttachPin(motorPins[i].pinEn, PWMSpeedChannel);
  }
  moveCar(STOP);

  basehomeservo.attach(basehomeservo_PIN);
  basehomeservo.write(0);
  fire(90);
}
void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    // Callback function to handle received data
    Serial.print("Data received: ");
    for (int i = 0; i < len; i++)
    {
        Serial.print((char)data[i]);
    }
    // wsCarInput.textAll("");
    // Serial.println();

    if (guard_flag == true)
    {
        // Blink the LED if the message contains "1001"
        if (strncmp((const char *)data, "1001", len) == 0)
        {
            wsCarInput.textAll("Recieving request From ESP-EYE to Deactivated Guardmode");
            guard_flag = false;
            wsCarInput.textAll("Guardmode-Deactivated");
            myData = "1002";
            // Convert String to C-style string
            char dataBuffer[myData.length() + 1];
            myData.toCharArray(dataBuffer, myData.length() + 1);

            // Send message via ESP-NOW
            esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)dataBuffer, myData.length() + 1);

            if (result == ESP_OK)
            {
                Serial.println("Sent with success");
            }
            else
            {
                Serial.println("Error sending the data");
            }
            delay(2000);
        }
    }else{
      if (strncmp((const char *)data, "1001", len) == 0)
        {
            wsCarInput.textAll("Recieving Message From ESP-EYE");
            guard_flag = false;
            wsCarInput.textAll("Guardmode-Deactivated");
            myData = "1002";
            // Convert String to C-style string
            char dataBuffer[myData.length() + 1];
            myData.toCharArray(dataBuffer, myData.length() + 1);

            // Send message via ESP-NOW
            esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)dataBuffer, myData.length() + 1);

            if (result == ESP_OK)
            {
                Serial.println("Sent with success");
            }
            else
            {
                Serial.println("Error sending the data");
            }
            delay(2000);
        }
    }
}
// Ultrasonic Sensor
void getDistance(){
  long duration, distance;
  
  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the pulse
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance based on the speed of sound
  // Speed of sound in air is approximately 343 meters per second (or 0.0343 centimeters per microsecond)
  distance = duration * 0.0343 / 2;

  // Print the distance to the Serial Monitor
  String distanceString = String(distance);
  wsCarInput.textAll(distanceString);
   // Delay before next measurement
}


// Ultrasonic Sensor


void setup(void) {
  setUpPinModes();
  Serial.begin(115200);

  // ESPNOW
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Register callback for received data
  esp_now_register_recv_cb(onDataReceived);

  // ESPNOW

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);

  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);

  server.begin();
  Serial.println("HTTP server started");

}

void loop() {
  if(guard_flag==true){
    getDistance();
    delay(100);
  }
  if(target_hit==target){
    wsCarInput.textAll("t1");
    target_hit++;
  }
  wsCarInput.cleanupClients();
}


