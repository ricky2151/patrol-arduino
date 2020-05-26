#include "Arduino.h"
#include "LoRa_E32.h"
#include <SoftwareSerial.h>
#include <HardwareSerial.h>   

//SoftwareSerial mySerial(3, 1); 
HardwareSerial loraSerial(1);
//LoRa_E32 e32ttl100(satu, tiga);
//txd rxd 1 3

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  //set output LED
 pinMode(18, OUTPUT);

  //set normal mode in LoRa
  //digitalWrite(4, 0); //M0
  //digitalWrite(2, 1); //M1


  //MANUAL
 //mySerial.begin(9600);

 //MANUAL 2
 //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  //Serial1.begin(9600, SERIAL_8N1, 1, 3);

  //MANUAL 3
  loraSerial.begin(9600, SERIAL_8N1, 16,17);
  

  
  //startup all pins and UART
  //e32ttl100.begin()

  //send message
  //ReponseStatus rs = e32ttl100.sendMessage("Hello world");

  //check if there is some problem of successfully send
  //Serial.println(rs.getResponseDescription());
  
}

void loop() {


  //manual
//  if (mySerial.available()) {
//    Serial.write(mySerial.read());
//  }
//  if (Serial.available()) {
//    mySerial.write(Serial.read());
//  }

  //manual 2
//  if (Serial1.available()) {
//    String msg = String(Serial.read());
//    if(msg.startsWith("pesan dari board"))
//    {
//      Serial.println("New Message");  
//      Serial.println(msg);
//      Serial.println("End of Message");
//    }
//  }
//  Serial1.write("pesan dari board ricky");

  //manual 3
    if (loraSerial.available()){                 
        String incomingString = loraSerial.readString();
        //loraSerial.println("New Message");
        Serial.println(incomingString);
        //loraSerial.println("End of Message");
        //loraSerial.println("Ack mess");
        delay(10);
    }

    if (Serial.available()) {
      loraSerial.print(Serial.readString());
    }
  
   // If something available
//  if (e32ttl100.available()>1) {
//      // read the String message
//    ResponseContainer rc = e32ttl100.receiveMessage();
//    // Is something goes wrong print error
//    if (rc.status.code!=1){
//        rc.status.getResponseDescription();
//    }else{
//        // Print the data received
//        Serial.println(rc.data);
//    }
//  }
//  if (Serial.available()) {
//      String input = Serial.readString();
//      e32ttl100.sendMessage(input);
//  }

  
  //BLINK LED
  Serial.println("================");
//   //mySerial.write("Hello, world?");
  digitalWrite(18, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(18, LOW);    // turn the LED off by making the voltage LOW
  delay(1000); 
}
