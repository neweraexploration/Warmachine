#include "esp_camera.h"
#include "camera_pins.h"  // Must include this after setting
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h"           // disable brownout problems
#include "soc/rtc_cntl_reg.h"  // disable brownout problems
#include "esp_http_server.h"
#include <ESP32Servo.h>
#include "helpers.h"
#include "vector"


unsigned long last_still = 0;

bool last_region_g4 = true;
bool toggle_track = true;

unsigned long squirt_t;
bool tracking_on = false;



#define PAN_PIN 14
#define TILT_PIN 15

Servo panServo;
Servo tiltServo;

struct MOTOR_PINS {
  int pinEn;
  int pinIN1;
  int pinIN2;
};

std::vector<MOTOR_PINS> motorPins = {
  { 2, 12, 13 },  //RIGHT_MOTOR Pins (EnA, IN1, IN2)
  { 2, 14, 15 },    //LEFT_MOTOR  Pins (EnB, IN3, IN4)
};

#define LIGHT_PIN 4

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
const int PWMSpeedChannel = 2;
const int PWMLightChannel = 3;



// Replace with your network credentials
const char *ssid = "batman";
const char *password = "batman1234";

#define PART_BOUNDARY "123456789000000000000987654321"


//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WITHOUT_PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM_B
//#define CAMERA_MODEL_WROVER_KIT


#define SERVO_1 14  //top
#define SERVO_2 15  //bottom

////here
#define DRIVER_PIN 12  //

#define SERVO_STEP 5

Servo servoN1;
Servo servoN2;
Servo servo1;
Servo servo2;

int servo1Pos = 90;
int servo2Pos = 90;


static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>ESP32-CAM Robot</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
      table { margin-left: auto; margin-right: auto; }
      td { padding: 8 px; }
      .button {
        background-color: #2f4468;
        border: none;
        color: white;
        padding: 10px 20px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 18px;
        margin: 6px 3px;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }
      img {  width: auto ;
        max-width: 100% ;
        height: auto ; 
      }
    </style>
  </head>
  <body>
    
    <img src="" id="photo" >
    
       
   <script>
   function toggleCheckbox(x) {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/action?go=" + x, true);
     xhr.send();
   }
   window.onload = document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
  </script>
  </body>
</html>
)rawliteral";


static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

/*
void move_left(int degs){
//  degs = 1;
  Serial.print("in move left ");
  Serial.println(servo2Pos);
  Serial.println(degs);
  for(int i = 0; i<degs; i++){
    if(servo2Pos <= 180-SERVO_STEP) {
      servo2Pos += SERVO_STEP;
      Serial.print("in move left ");
      Serial.println(servo2Pos);
      servo2.write(servo2Pos);
      Serial.println(degs);
      delay(30);
    }
  }
}

void move_right(int degs){
//  degs = 1;
  Serial.print("in move right ");
  Serial.println(degs);
  for(int i = 0; i<degs; i++){
    if(servo2Pos >= SERVO_STEP) {
      servo2Pos -= SERVO_STEP;
      Serial.print("in move right ");
      Serial.println(servo2Pos);
      servo2.write(servo2Pos);
      Serial.println(degs);
      delay(30);
    }
  }
}
*/

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

void moveCar(int inputValue) {
  Serial.printf("Got value as %d\n", inputValue);
  int motorSpeed = 40; // Adjust this value as needed, ranging from 0 to 1023

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
    default:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);
      motorSpeed = 0; // Set speed to zero for stopping
      break;
  }

  // Apply speed to the motors
  ledcWrite(PWMSpeedChannel, motorSpeed);
}

static esp_err_t stream_handler(httpd_req_t *req) {
  fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }


  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    }


    else {
      //This is the motion tracking stuff here
      if (tracking_on && finished_tracking) {
        //        toggle_track = false;
        last_still = millis();
        if (!capture_still()) {
          Serial.println("Failed capture");
        } else {
          cnt++;
          if (first_capture) {
            //for first capture we copy the current to the previous frame
            //so that we don't have stale data from before we moved
            //the servo in the previous frame
            cnt = 0;
            update_frame();
            first_capture = false;
          }
        }

        if (motion_detect()) {
          cnt = 0;
          do_tracking = true;         //loop will activate the servo tracking if do_tracking is true
          finished_tracking = false;  //don't do any more image processing until serov
                                      //is finished because we don't want to mistake the motion of the servo
                                      //for something moving
          Serial.println("motion detected");
        } else if (cnt > 1000) {
          //motion hasn't been detected for 1000 frames
          servo2.write(90);
          delay(100);
          capture_still();
          update_frame();
        }
        update_frame();

      } else {
        toggle_track = true;  //this is not used
      }



      //end motion tracking and convert to jpeg for transportation
      if (fb->width > 100) {
        if (fb->format != PIXFORMAT_JPEG) {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted) {
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK) {
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}
static esp_err_t cmd_handler(httpd_req_t *req) {
  char *buf;
  size_t buf_len;
  char action[32] = { 0 };
  int value = 0;

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = (char *)malloc(buf_len);
    if (!buf) {
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      if (httpd_query_key_value(buf, "go", action, sizeof(action)) == ESP_OK) {
        // Splitting the received action and value using a delimiter (comma)
        char *token = strtok(action, ",");
        if (token != NULL) {
          strcpy(action, token);
          token = strtok(NULL, ",");
          if (token != NULL) {
            value = atoi(token);  // Convert string to integer value
          }
        }
      } else {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
      }
    } else {
      free(buf);
      httpd_resp_send_404(req);
      return ESP_FAIL;
    }
    free(buf);
  } else {
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }

  sensor_t *s = esp_camera_sensor_get();
  //flip the camera vertically
  //s->set_vflip(s, 1);          // 0 = disable , 1 = enable
  // mirror effect
  //s->set_hmirror(s, 1);          // 0 = disable , 1 = enable

  int res = 0;
  Serial.println(action);
  Serial.println(value);
  if(!strcmp(action, "MoveCar")) {
    moveCar(value);
  } else if(!strcmp(action, "Speed")) {
    ledcWrite(PWMSpeedChannel, value);
  } else if(!strcmp(action, "Light")) {
    ledcWrite(PWMLightChannel, value);
  } else {
    res = -1;
  }

  if (res) {
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri = "/action",
    .method = HTTP_GET,
    .handler = cmd_handler,
    .user_ctx = NULL
  };
  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setUpPinModes() {
  // Set up PWM
  ledcSetup(PWMSpeedChannel, PWMFreq, PWMResolution);
  ledcSetup(PWMLightChannel, PWMFreq, PWMResolution);

  for (int i = 0; i < motorPins.size(); i++) {
    pinMode(motorPins[i].pinEn, OUTPUT);
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);

    digitalWrite(motorPins[i].pinIN1, LOW);
    digitalWrite(motorPins[i].pinIN2, LOW);

    /* Attach the PWM Channel to the motor enb Pin */
    ledcAttachPin(motorPins[i].pinEn, PWMSpeedChannel);
  }

  ledcWrite(PWMSpeedChannel, 0); // Set initial speed to 0
  moveCar(STOP); // Stop the motors initially

  pinMode(LIGHT_PIN, OUTPUT);
  ledcAttachPin(LIGHT_PIN, PWMLightChannel);
  ledcWrite(PWMLightChannel, 50);
}


void setup() {

  setUpPinModes();

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brownout detector
  ////////////////////////////////////////////////////////////
  //changed here
  squirt_t = millis();

  /*
  pinMode(DRIVER_PIN, OUTPUT);
  digitalWrite(DRIVER_PIN, LOW); //make sure motor is off
  ///////////////////////////////////////////////////////
  
  servo1.setPeriodHertz(50);    // standard 50 hz servo
  servo2.setPeriodHertz(50);    // standard 50 hz servo
  servoN1.attach(2, 1000, 2000);
  servoN2.attach(13, 1000, 2000);
  
  servo1.attach(SERVO_1, 1000, 2000);
  servo2.attach(SERVO_2, 1000, 2000);
  
  servo1.write(servo1Pos);
  servo2.write(servo2Pos);
  */

  Serial.begin(115200);
  
  Serial.setDebugOutput(false);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;  //PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = MY_FRAMESIZE;  //FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = MY_FRAMESIZE;  //FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());

  // Start streaming web server
  startCameraServer();
}

void loop() {
  /*
  //changed this
  if((millis()-squirt_t)>400){
    digitalWrite(DRIVER_PIN, LOW);
    watergun_off_time = millis();
  }
  if(tracking_on && do_tracking){
    do_tracking = false;
    finished_tracking = false;
     Serial.println("Motion detected");
     
    //do stuff here
    if(region_of_interest<4){
      
      //to the left
//      if(!last_region_g4)
      move_left(3*(4-region_of_interest));
      last_region_g4 = false;
    } else if(region_of_interest>4){
//      if(last_region_g4)
      move_right(3*(region_of_interest-4));
      last_region_g4 = true;
    }
    if(fire_waterpistol){ //only fire water pistol if enough motion is detected
      if(millis() - watergun_off_time > 100){
        //don't want to turn the water gun on again unless it's been off for at least 300ms
        digitalWrite(DRIVER_PIN,HIGH);
        squirt_t = millis(); }
    }
    
    delay(100);
    first_capture = true;
    finished_tracking = true;
    Serial.print("regin_of_interest = ");
    Serial.println(region_of_interest);
    
  }
//  delay(10);
*/
}
