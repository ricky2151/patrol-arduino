/*********************
  | Custom Library
*********************/
#include "monitoring_system.h"


/*********************
  | SCREEN TFT ILI9225 176*220
*********************/
#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include <../fonts/FreeSans9pt7b.h>
#include <../fonts/FreeSans12pt7b.h>
#define TFT_RST 15    // RST P17
#define TFT_RS  2    // RS P16
#define TFT_CLK 18    // CLK P18
#define TFT_SDI 23    // SDA P23
#define TFT_CS  5     // CS P5
#define TFT_LED 0     // 0 if wired to +5V directly
#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)
// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 TFTscreen = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
int16_t x=0, y=0, width, height; //position;
String strTextDisplay;


/*********************
  | QR CODE
*********************/
#include <qrcode.h>
QRCode qrcode;
uint8_t vers = 3;
uint8_t pixel = 5;
uint8_t offset_x = 15;
uint8_t offset_y = 5;
uint8_t borderWidth = 5;
char dataCharArray[100];
String strQrCode="";


/*********************
  | EEPROM
*********************/
String nodeID;// = "1";
String nodeName;// = "Ruang D.1.2";

/***************************************************************
  | DATA QRCODE
***************************************************************/
String currentDataQRCode = "";
unsigned long timerHandlingNoData;

void setup()
{

  //setting serial
  Serial.begin(9600);
  Serial.setTimeout(0);
  Serial.println("start setup.... Please wait until 4 process is finish !");

  //====== testing section =======

  //==============================


  //set pin M0 & M1 to OUTPUT 
  Serial.println("[1/4] Set pin M0 & M1 to output");
  pinMode(pinM0, OUTPUT);
  pinMode(pinM1, OUTPUT);
  Serial.println("[1/4] Set pin M0 & M1 is done !");

  //Connect to EEPROM from serial
  Serial.println("[2/4] Connect to EEPROM");
  connectEEPROM();
  Serial.println("[2/4] Connected to EEPROM ! Waiting serial data in 10 seconds");

  updateEEPROMFromSerial();


  //nodeID#nodeName#strQrCode>
  //1#Ruang D.1.2#sdlkfjvliwmeflkjslkfj>

  Serial.println("[2/4] Read data from EEPROM");
  //EEPROM.writeString(0,"DESKTOP-P4FL5H1_9931#patrol1234#1#broker.shiftr.io#904e4807#cfdc8ca761caadf9#QR_Display1");
  //EPROM.commit();
  String readData = EEPROM.readString(0);
  Serial.println(readData);

  nodeID = getValue(readData, '#', 0); Serial.print("NodeID     : "); Serial.println(nodeID);
  nodeName = getValue(readData, '#', 1); Serial.print("nodeName: "); Serial.println(nodeName);
  strQrCode = getValue(readData, '#', 2);  Serial.print("strQrCode: "); Serial.println(strQrCode);

  Serial.println("[2/4] Finish read data from EEPROM !");

  


  //setup lora
  Serial.println("[3/4] Setup LoRa... (wait 10 sec)");
  loraSerial.begin(9600, SERIAL_8N1, pinRX, pinTX);
  
  loraSerial.setTimeout(10);
  clearLoraSerial();
  clearSerial();
  //loraSerial.flush();
  //Serial.flush();
  

  //get adl and adh
  int intNodeId = nodeID.toInt();
  myAdl = intNodeId / 256;
  myAdh = intNodeId % 256;

  //set sleep mode in LoRa & setting parameter lora
  setupParameterLoRa();
  Serial.println("This LoRa address : " + String(myAdl) + " " + String(myAdh));
  Serial.println("[3/4] Setup LoRa has finished !");

  //setup qrcode
  Serial.println("[4/4] Setup QRCode...");
  TFTscreen.begin();
  TFTscreen.setOrientation(2);
  TFTscreen.setBacklightBrightness(128);
  printShiftNotFound();
  Serial.println("[4/4] Setup QRCode is finished !");
  
  
}

void loop()
{
  //if, for 1.1 minutes no data is received, then the qrcode is replaced by "No Shift Found"
  if(millis() - timerHandlingNoData >= 90000)
  {
    printShiftNotFound();
    timerHandlingNoData = millis();
  }

  if (loraSerial.available())
  {
    timePeriodTest = millis();
    Serial.println("ada incoming message");

    
    String incomingString = "";
    //read incoming string
    incomingString = loraSerial.readString();
    
    //evaluation purpase
    if(incomingString.substring(6,7) == "T")
    {
      sendMessage(0, 4, 0x17, "Acknowledgelll", false);
      Serial.println("cek milisnya donggg");
      Serial.println(millis() - timePeriodTest);
    }
    else
    {
      
  
      bool fromDevice = checkMessageFromLoRaOrNot(incomingString);
  
      if (fromDevice == true)
      {
        //get sender from string message (3 byte)
        //format message : (WITH ACK OR NOT (1 / 0)) 11 11 (myADDRESS LOW) (myADDRESS HIGH) (myCHANNEL) (MESSAGE)
        
        byte addressLowSender = incomingString.charAt(3);
        byte addressHighSender = incomingString.charAt(4);
        byte channelSender = incomingString.charAt(5);
        String message = incomingString.substring(6);
  
        //send ack to sender
        if (incomingString.charAt(0) == 1) //if must send ack to sender
        {
          sendMessage(addressLowSender, addressHighSender, channelSender, "Acknowledge", false);
        }
        currentDataQRCode = incomingString.substring(6);
        timerHandlingNoData = millis();
        printQR(currentDataQRCode);
        
        
      }
      else
      {
        Serial.println("undefined type message");
        Serial.println("dalam string : ");
        Serial.println(incomingString);
        Serial.println("dalam hexa : ");
        for (int i = 0; i < incomingString.length(); i++)
        {
          Serial.print(incomingString[i], HEX);
          Serial.println(" ");
        }
      }
    }
    

    delay(10);
  }

  if (Serial.available())
  {
    String input = Serial.readString();
    if (input.startsWith("param"))
    {
      Serial.println("lihat param : ");
      byte cmd[] = {0xC1, 0xC1, 0xC1};
      loraSerial.write(cmd, sizeof(cmd));
    }
    else if (input.startsWith("set param:"))
    {
      readParamFromSerialAndSave(input);
    }
    else if (input.startsWith("kirim pesan:"))
    {
      //get low address input
      String stringMessage = input.substring(12);
      sendMessage(0, 4, recChannel, stringMessage, true);
    }
    else if (input.startsWith("mode normal"))
    {
      setMode("normal");
    }
    else if (input.startsWith("mode sleep"))
    {
      setMode("sleep");
    }
  }

  
}

/*
   ==== FUNCTIONS ====
*/



/*
   Message to other device
*/
void sendMessage(byte adl, byte adh, byte channel, String msg, bool withAck)
{
  //format message : (WITH ACK OR NOT (1 / 0)) 11 11 (myADDRESS LOW) (myADDRESS HIGH) (myCHANNEL) (MESSAGE)
  //11 11 is a sign that it is a message from another device
  Serial.println("siap-siap kirim pesan !");
  byte cmd[100] = {adl, adh, channel, withAck, 11, 11, myAdl, myAdh, myChannel};
  for (int i = 0; i < msg.length(); i++)
  {
    cmd[i + 9] = msg.charAt(i);
  }
//  for (int i = 0; i < msg.length() + 9; i++)
//  {
//    Serial.println(cmd[i]);
//  }
  
  loraSerial.write(cmd, sizeof(cmd));
  
}

void printShiftNotFound(){
  TFTscreen.clear();
  TFTscreen.setGFXFont(&FreeSans9pt7b);
  strTextDisplay = "No Shift Found"; // Create string object
  TFTscreen.getGFXTextExtent(strTextDisplay, x, y, &width, &height); // Get string extents
  x = 10; y = height;
  TFTscreen.drawGFXText(x, y, strTextDisplay, COLOR_GREEN); // Print string
}

void printQR(String strData)
{ 
  TFTscreen.clear();

  if(strData!=""){
      /**********************
      | QR Code             |
    **********************/
    strData.toCharArray(dataCharArray, sizeof(dataCharArray));
    // Print to TFT
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, vers, 0, dataCharArray); // your text in last parameter, e.g. "Hello World"

    //make border
    TFTscreen.fillRectangle((offset_x - borderWidth), (offset_y - borderWidth), ((qrcode.size * pixel) + offset_x + borderWidth), offset_y, COLOR_WHITE); //square1 horizontal atas
    TFTscreen.fillRectangle((offset_x - borderWidth), offset_y, offset_x, ((qrcode.size * pixel) + offset_y), COLOR_WHITE); //square2 vertical kiri
    TFTscreen.fillRectangle((offset_x - borderWidth), ((qrcode.size * pixel) + offset_y), ((qrcode.size * pixel) + offset_x + borderWidth), ((qrcode.size * pixel) + offset_y + borderWidth), COLOR_WHITE); //square3 horizontal bawah
    TFTscreen.fillRectangle(((qrcode.size * pixel) + offset_x), offset_y, ((qrcode.size * pixel) + offset_x + borderWidth), ((qrcode.size * pixel) + offset_y), COLOR_WHITE); //square4

    for (uint8_t y = 0; y < qrcode.size; y++) { //vertical
      for (uint8_t x = 0; x < qrcode.size; x++) { //horizontal
        if (!qrcode_getModule(&qrcode, x, y)) {
          uint16_t x1 = (x * pixel) + offset_x;
          uint16_t y1 = (y * pixel) + offset_y;
          uint16_t x2 = (x1 + pixel - 1);
          uint16_t y2 = (y1 + pixel - 1);
          TFTscreen.fillRectangle(x1, y1, x2, y2, COLOR_WHITE);
        }
      }
    }
  }

  strTextDisplay = nodeName; // Create string object
  TFTscreen.setGFXFont(&FreeSans9pt7b);
  TFTscreen.getGFXTextExtent(strTextDisplay, x, y, &width, &height); // Get string extents
  x = 10;
  y = 160 + height; // Set y position to string height plus shift down 10 pixels
  TFTscreen.drawGFXText(x, y, strTextDisplay, COLOR_WHITE); // Print string

  strTextDisplay = "UKDW"; // Create string object
  TFTscreen.getGFXTextExtent(strTextDisplay, x, y, &width, &height); // Get string extents
  x = 10;
  y += height + 5; // Set y position to string height plus shift down 10 pixels
  TFTscreen.drawGFXText(x, y, strTextDisplay, COLOR_ORANGE); // Print string

  strTextDisplay = "Patrolee System"; // Create string object
  TFTscreen.setGFXFont(&FreeSans9pt7b);
  TFTscreen.getGFXTextExtent(strTextDisplay, x, y, &width, &height); // Get string extents
  x = 10;
  y += height + 5; // Set y position to string height plus shift down 10 pixels
  TFTscreen.drawGFXText(x, y, strTextDisplay, COLOR_CYAN); // Print string
    
  
}
