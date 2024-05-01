#include "LoRaWan_APP.h"

#include <HardwareSerial.h>
HardwareSerial SerialPort(1);
#include <Arduino.h>
#include "MH-Z14A.h"
MHZ14A sensor(SerialPort, Serial);

#include <DHT.h>
#define DHTPIN 17
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

String hexString;
String DT_hexString;
String DH_hexString;

/* OTAA para 範圍連線*/
uint8_t devEui[] = { 0x22, 0x32, 0x33, 0x00, 0x00, 0x88, 0x88, 0x02 };
uint8_t appEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appKey[] = { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88 };

/* ABP para 指定連線*/
uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda,0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef,0x67 };
uint32_t devAddr =  ( uint32_t )0x007e6ae1;

/*LoraWan channelsmask, default channels 0-7 通道掩碼*/ 
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported A先說後聽 B特定時間聽 C一直聽*/
DeviceClass_t  loraWanClass = CLASS_A;

/*the application data transmission duty cycle.  value in [ms]. 幾秒發送一次*/
uint32_t appTxDutyCycle = 15000;

/*OTAA or ABP 要OTAA 還是 要ABP*/
bool overTheAirActivation = true;

/*ADR enable 速率自適應*/
bool loraWanAdr = true;

/* Indicates if the node is sending confirmed or unconfirmed messages 是否要回傳收到*/
bool isTxConfirmed = true;

/* Application port 類似群組*/
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
/*傳送次數 1或2 不會調速*/
uint8_t confirmedNbTrials = 4;

/* Prepares the payload of the frame 傳的資料*/
static void prepareTxFrame( uint8_t port )
{
  /*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
  *appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
  *if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
  *if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
  *for example, if use REGION_CN470, 
  *the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
  */
    appDataSize = 9;
    appData[0] = 0x02 ;
    appData[1] = 0x0E ;
    appData[2] = 0x83 ;
    String firstPart = hexString.substring(0, 2);
    String secondPart = hexString.substring(2, 4);
    uint8_t firstValue = strtoul(firstPart.c_str(), nullptr, 16);
    uint8_t secondValue = strtoul(secondPart.c_str(), nullptr, 16);
    appData[3] = firstValue;
    appData[4] = secondValue;
    String DT_firstPart = DT_hexString.substring(0, 2);
    String DT_secondPart = DT_hexString.substring(2, 4);
    uint8_t DT_firstValue = strtoul(DT_firstPart.c_str(), nullptr, 16);
    uint8_t DT_secondValue = strtoul(DT_secondPart.c_str(), nullptr, 16);
    appData[5] = DT_firstValue;
    appData[6] = DT_secondValue;
    String DH_firstPart = DH_hexString.substring(0, 2);
    String DH_secondPart = DH_hexString.substring(2, 4);
    uint8_t DH_firstValue = strtoul(DH_firstPart.c_str(), nullptr, 16);
    uint8_t DH_secondValue = strtoul(DH_secondPart.c_str(), nullptr, 16);
    appData[7] = DH_firstValue;
    appData[8] = DH_secondValue;
}

//if true, next uplink will add MOTE_MAC_DEVICE_TIME_REQ 


void setup() {
  SerialPort.begin(115200, SERIAL_8N1, 47, 48);
  Serial.begin(115200);
  sensor.begin(4000);
  dht.begin();
  sensor.setDebug(true);
  Mcu.begin();
  deviceState = DEVICE_STATE_INIT;
}

void loop()
{
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
      hexString = String(sensor.readConcentrationPPM(0x01), HEX);
      if(hexString.length()==1){
        hexString = "000"+hexString;
      }else if(hexString.length()==2){
        hexString = "00"+hexString;
      }else if(hexString.length()==3){
        hexString = "0"+hexString;
      }
      Serial.println(hexString);

      float t = dht.readTemperature();
      int x1;
      t = t*100;
      x1 = (int) t;
      DT_hexString = String(x1,HEX);
      if(DT_hexString.length()==1){
        DT_hexString = "000"+DT_hexString;
      }else if(DT_hexString.length()==2){
        DT_hexString = "00"+DT_hexString;
      }else if(DT_hexString.length()==3){
        DT_hexString = "0"+DT_hexString;
      }
      Serial.println(DT_hexString);

      float h = dht.readHumidity();
      int x2;
      h = h*100;
      x2 = (int) h;
      DH_hexString = String(x2,HEX);
      if(DH_hexString.length()==1){
        DH_hexString = "000"+DH_hexString;
      }else if(DH_hexString.length()==2){
        DH_hexString = "00"+DH_hexString;
      }else if(DH_hexString.length()==3){
        DH_hexString = "0"+DH_hexString;
      }
      Serial.println(DH_hexString);
      
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
