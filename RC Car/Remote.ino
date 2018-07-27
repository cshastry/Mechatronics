#include<ESP8266WiFi.h>
#include<WiFiUdp.h>

//Define parameter pins
int drive_pin=D8;//Potentiometer input for controlling speed and direction of motor
int steer_pin=D7;//Potentiometer input for controlling steering
int channel_pin=D5;


//Router Configuration
const char* ssid1= "modlab1";
const char* password1= "ESAP2017";
const char* ssid2= "Mechatronics";
const char* password2= "YayFunFun";
IPAddress ipSendto(192, 168, 1, 75);//Reciever IP


//Initializing state variables
float drive;
char drive_direction;
float steer;
char steer_direction;
int channel;


//Initializing wifi object
unsigned int udpRemotePort=3100;
unsigned int udplocalPort=3000;
const int UDP_PACKET_SIZE = 11;
char udpBuffer[ UDP_PACKET_SIZE];
WiFiUDP udp;


//Data send handle
void fncUdpSend(){
    //Create data packet and send
    //Sends data in following packet format(FR1022,20) 
    //F-forward B-Backward N-Neutral, L-left,R-Right,N-Neutral,0-1024 PWM values,0-30 degress of steering
    String output=String(drive_direction);
    output.concat(String(steer_direction));
    output.concat(String((int)drive));
    output.concat(",");
    output.concat(String((int)steer));
    Serial.println(output);
    output.toCharArray(udpBuffer,sizeof(udpBuffer));
    
    //send packet to reciever
    udp.beginPacket(ipSendto, udpRemotePort);
    udp.write(udpBuffer, sizeof(udpBuffer));
    udp.endPacket();
  }


void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  Serial.print("Connecting to ");
  
  //Wifi Channel switching
  channel=digitalRead(channel_pin);
  if(channel==1){
    WiFi.begin(ssid1,password1);
    Serial.print("Connecting to A");
    
  }
  else{
    WiFi.begin(ssid2,password2);
    Serial.print("Connecting to B");
  }


  WiFi.config(IPAddress(192, 168, 1, 61), //my IP address
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
  
  
  // ADC control pin setup-Used to power Potentiometers alternatively for sensor reading multiplexing
  pinMode(drive_pin,OUTPUT);
  pinMode(steer_pin,OUTPUT);
  
  
  
}

void loop() {
  
  //Multiplex reading of Driving and Steering Potentiometers by alternatively powering them
  digitalWrite(drive_pin, HIGH);
  digitalWrite(steer_pin, LOW);

  //Calibrating potentiometers(Dead zone of values between 255 and 355 where the motors are set to neutral-corresponds to 5mm travel left and right of center point
  if(analogRead(A0)<=255){
    delay(1);
    drive=(255-analogRead(A0))*4.09;//Linearizing and Scaling
    drive_direction='B';
  }
  else if(analogRead(A0)>=355){
    delay(1);
    drive=(analogRead(A0)-355)*2.07;
    drive_direction='F';
  }
  else{
    delay(1);
    drive=0;
    drive_direction='N';
  }
  
  //Serial.println((int)drive);
  //Serial.println(analogRead(A0));
  delay(1);
  
  digitalWrite(drive_pin, LOW);
  digitalWrite(steer_pin, HIGH);
  

  //Calibrating potentiometers(Dead zone of values between 255 and 355 where the motors are set to neutral-corresponds to 5mm travel left and right of center point
  if(analogRead(A0)<=255){
    steer=(255-analogRead(A0))/8.33;//Linearizing and scaling
    steer_direction='L';
    delay(1);
  }
  else if(analogRead(A0)>=355){
    steer=(analogRead(A0)-355)/16.67;
    steer_direction='R';
    delay(1);
  }
  else{
    steer=0;
    steer_direction='N';
    delay(1);
  }
  
  //Serial.println((int)steer);
  //Serial.println(analogRead(A0));
  delay(1);
  fncUdpSend();
  delay(50);//packet delay
  
  
  //Serial.println(digitalRead(channel_pin));
  //delay(1);
  
  
}
