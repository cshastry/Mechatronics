#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<Servo.h>


//Pin to change channels
int channel_pin=D3;


//Motor control pins
int motor1_enable=D0;
int motor1A=D8;
int motor1B=D7;

int motor2_enable=D1;
int motor2A=D6;
int motor2B=D5;

//Servo control pin
int steer_pin=D4;
Servo steering;

//Router Configuration
const char* ssid1= "modlab1";
const char* password1= "ESAP2017";
const char* ssid2= "Mechatronics";
const char* password2= "YayFunFun";

IPAddress ipSendto(192, 168, 1, 61);//Reciever IP


//Initializing state variables
int drive;
char drive_direction;
int steer;
char steer_direction;
int channel;

//Initializing wifi object
unsigned int udpRemotePort=3000;
unsigned int udplocalPort=3100;
const int UDP_PACKET_SIZE = 11;
char udpBuffer[ UDP_PACKET_SIZE];
WiFiUDP udp;

void handleUDPServer(){
  
  //Read Data packets
  int c = udp.parsePacket();
  if(c){
      udp.read(udpBuffer, UDP_PACKET_SIZE);
      drive_direction=udpBuffer[0];//Direction Data 'F'-Forward,'B'-backward,'N'-Neutral
      steer_direction=udpBuffer[1];//Direction Data 'L'-left,'R'-Right,'N'-Neutral
      
      String myData = "";
      String test = "";
      for(int i = 2; i < UDP_PACKET_SIZE; i++){ 
          test=(char)udpBuffer[i];
          if(test==","){
            drive=myData.toInt();//Drive PWM value
            myData="";
            continue;       
          }
          myData += (char)udpBuffer[i];
          
        }
      steer=myData.toInt();//Servo Steering angle           
    }
    
}

void setup() {
  
  Serial.begin(115200);
  Serial.print("Connecting to ");
  
  
  //Channel switching
  channel=digitalRead(channel_pin);
  if(channel==0){
    WiFi.begin(ssid1,password1);
    Serial.print("Connecting to A"); 
  }
  else{
    WiFi.begin(ssid2,password2);
    Serial.print("Connecting to B");
  }

  WiFi.config(IPAddress(192, 168, 1, 75), //my IP address
              IPAddress(192, 168, 1, 1),
              IPAddress(255, 255, 255, 0));
  
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  } 
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  udp.begin(udplocalPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  //Steering Setup
  steering.attach(steer_pin,400,2400);

  //Drive Setup
  pinMode(motor1A,OUTPUT);
  pinMode(motor1B,OUTPUT);
  pinMode(motor2A,OUTPUT);
  pinMode(motor2B,OUTPUT);
  pinMode(motor1_enable,OUTPUT);
  pinMode(motor2_enable,OUTPUT);
  
}

void loop() {
  handleUDPServer();
//  Serial.println(drive_direction);
//  delay(1);
//  Serial.println(steer_direction);
//  delay(1);
//  Serial.println(drive);
//  delay(1);
//  Serial.println(steer);
//  delay(1);

  //Steering to recieved angle 91 degree is calibrated neutral point-steer has been intentionally limited from input side to +-30 degrees for smooth car motion
  if(steer_direction=='L'){
    steering.write(91-steer);
    delay(1);
  }
  else if(steer_direction=='R'){
    steering.write(91+steer);
    delay(1);
  }
  else if(steer_direction=='N'){
    steering.write(91);
    delay(1);
  }

  //Driving at desired direction
  if(drive_direction=='F'){
    digitalWrite(motor1A,HIGH);
    digitalWrite(motor1B,LOW);
    digitalWrite(motor2A,HIGH);
    digitalWrite(motor2B,LOW);
    analogWrite(motor1_enable,drive);
    analogWrite(motor2_enable,drive);
    delay(1);
  }
  else if(drive_direction=='B'){
    digitalWrite(motor1A,LOW);
    digitalWrite(motor1B,HIGH);
    digitalWrite(motor2A,LOW);
    digitalWrite(motor2B,HIGH);
    analogWrite(motor1_enable,drive);
    analogWrite(motor2_enable,drive);
    delay(1);
  }
  else if(drive_direction=='N'){
    analogWrite(motor1_enable,drive);
    analogWrite(motor2_enable,drive);
    delay(1);
  }
  
}
