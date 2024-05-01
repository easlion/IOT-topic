#include "LoRaWan_APP.h"

#include "HX711.h"
#include <Wire.h>
#include "I2CKeyPad.h"

char keys[] = "D#0*C987B654A321NF";
unsigned long int start_time = 0;
unsigned long int key_delay_time = 0;

uint8_t mod = 0x00;
String S = "";
String classx;
String hexString;
String classxl;

const int DT_PIN = 16;
const int SCK_PIN = 4;

const int scale_factor = 211; //比例參數，從校正程式中取得

HX711 scale;
I2CKeyPad keyPad(0x38);

/* OTAA para*/
uint8_t devEui[] = { 0x22, 0x32, 0x33, 0x00, 0x00, 0x88, 0x88, 0x05 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88 };

/* ABP para*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

/*LoraWan channelsmask, default channels 0-7*/ 
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = CLASS_C;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 5000;

/*OTAA or ABP*/
bool overTheAirActivation = false;

/*ADR enable*/
bool loraWanAdr = true;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = false;

/* Application port */
uint8_t appPort = 2;
/*!
* Number of trials to transmit the frame, if the LoRaMAC layer did not
* receive an acknowledgment. The MAC performs a datarate adaptation,
* according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
* to the following table:
*
* Transmission nb | Data Rate
* ----------------|-----------
* 1 (first)       | DR
* 2               | DR
* 3               | max(DR-1,0)
* 4               | max(DR-1,0)
* 5               | max(DR-2,0)
* 6               | max(DR-2,0)
* 7               | max(DR-3,0)
* 8               | max(DR-3,0)
*
* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
* the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 4;

/* Prepares the payload of the frame */
static void prepareTxFrame( uint8_t port )
{
  /*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
  *appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
  *if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
  *if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
  *for example, if use REGION_CN470, 
  *the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
  */
    appDataSize = 8;
    appData[0] = 0x05;
    String class3 = classxl.substring(0, 2);
    String class4 = classxl.substring(2, 4);
    uint8_t class3Value = strtoul(class3.c_str(), nullptr, 16);
    uint8_t class4Value = strtoul(class4.c_str(), nullptr, 16);
    appData[1] = class3Value;
    appData[2] = class4Value;
    String firstPart = hexString.substring(0, 2);
    String secondPart = hexString.substring(2, 4);
    uint8_t firstValue = strtoul(firstPart.c_str(), nullptr, 16);
    uint8_t secondValue = strtoul(secondPart.c_str(), nullptr, 16);
    appData[3] = firstValue;
    appData[4] = secondValue;
    String class1 = classx.substring(0, 2);
    String class2 = classx.substring(2, 4);
    uint8_t class1Value = strtoul(class1.c_str(), nullptr, 16);
    uint8_t class2Value = strtoul(class2.c_str(), nullptr, 16);
    appData[5] = class1Value;
    appData[6] = class2Value;
    appData[7] = mod;
}

//if true, next uplink will add MOTE_MAC_DEVICE_TIME_REQ 


void setup() {
  Serial.begin(9600);
  Mcu.begin();
  deviceState = DEVICE_STATE_INIT;
  
  Wire.begin(33,34);
  scale.begin(DT_PIN, SCK_PIN);

  scale.set_scale(scale_factor);       // 設定比例參數
  scale.tare();               // 歸零
}

void loop()
{ 
  start_time = millis();
  if(start_time - key_delay_time > 250){
    uint8_t idx = keyPad.getKey();
    if(keys[idx] == 'N'){}
    else{
      key_delay_time = millis();
      Serial.println(keys[idx]);
    }
    if(keys[idx] == '*'){
      mod = 0x01;
    }
  }
  if(mod == 0x00){
      switch( deviceState )
    {
      case DEVICE_STATE_INIT:
      {
  #if(LORAWAN_DEVEUI_AUTO)
        LoRaWAN.generateDeveuiByChipID();
  #endif
        LoRaWAN.init(loraWanClass,loraWanRegion);
        break;
      }
      case DEVICE_STATE_JOIN:
      {
        LoRaWAN.join();
        break;
      }
      case DEVICE_STATE_SEND:
      { 
        int distance0;
        distance0 = scale.get_units(10);
        hexString = String(distance0, HEX);
        if(hexString.length()==1){
          hexString = "000"+hexString;
        }else if(hexString.length()==2){
          hexString = "00"+hexString;
        }else if(hexString.length()==3){
          hexString = "0"+hexString;
        }
        Serial.println(hexString);
        
        prepareTxFrame( appPort );
        LoRaWAN.send();
        deviceState = DEVICE_STATE_CYCLE;
        break;
      }
      case DEVICE_STATE_CYCLE:
      {
        // Schedule next packet transmission
        txDutyCycleTime = appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
        LoRaWAN.cycle(txDutyCycleTime);
        deviceState = DEVICE_STATE_SLEEP;
        break;
      }
      case DEVICE_STATE_SLEEP:
      {
        LoRaWAN.sleep(loraWanClass);
        break;
      }
      default:
      {
        deviceState = DEVICE_STATE_INIT;
        break;
      }
    }
  }else if(mod == 0x01 || mod == 0x02){
    start_time = millis();
    if(start_time - key_delay_time > 250){
      uint8_t idx = keyPad.getKey();
      if(keys[idx] == 'N'){}
      else{
        key_delay_time = millis();
        Serial.println(keys[idx]);
      }
      switch (keys[idx]) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
        case 'A':
        case 'B':
        case 'C':
          S += keys[idx];
          Serial.println(keys[idx]);
          break;
        case '#':
          S = "";
          classx = "";
          mod = 0x00;
          break;
        case 'D':
          classxl = classx;
          mod = 0x02;
          break;
          
        default:
          break;
      }
    }
    switch( deviceState )
    {
      case DEVICE_STATE_INIT:
      {
  #if(LORAWAN_DEVEUI_AUTO)
        LoRaWAN.generateDeveuiByChipID();
  #endif
        LoRaWAN.init(loraWanClass,loraWanRegion);
        break;
      }
      case DEVICE_STATE_JOIN:
      {
        LoRaWAN.join();
        break;
      }
      case DEVICE_STATE_SEND:
      { 
        int distance1;
        distance1 = scale.get_units(10);
        hexString = String(distance1, HEX);
        if(hexString.length()==1){
          hexString = "000"+hexString;
        }else if(hexString.length()==2){
          hexString = "00"+hexString;
        }else if(hexString.length()==3){
          hexString = "0"+hexString;
        }
        Serial.println(hexString);

        String distance2;
        distance2 = S;
        classx = distance2;
        if(classx.length()==1){
          classx = "000"+classx;
        }else if(classx.length()==2){
          classx = "00"+classx;
        }else if(classx.length()==3){
          classx = "0"+classx;
        }
        Serial.println(classx);
        
        prepareTxFrame( appPort );
        LoRaWAN.send();
        deviceState = DEVICE_STATE_CYCLE;
        break;
      }
      case DEVICE_STATE_CYCLE:
      {
        // Schedule next packet transmission
        txDutyCycleTime = appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
        LoRaWAN.cycle(txDutyCycleTime);
        deviceState = DEVICE_STATE_SLEEP;
        break;
      }
      case DEVICE_STATE_SLEEP:
      {
        LoRaWAN.sleep(loraWanClass);
        break;
      }
      default:
      {
        deviceState = DEVICE_STATE_INIT;
        break;
      }
    }
  }
}
