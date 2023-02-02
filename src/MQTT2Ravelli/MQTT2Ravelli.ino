/*
 * Written by DuchkPy
 * https://github.com/DuchkPy/MQTT2Ravelli/
 *
 * Created to interface a Ravelli stove with an ESP8266 board
 * List of compatible stove models available on github
 */

// PARAMETERS
const char* ssid = "your WiFi network name";  // WiFi SSID
const char* password = "your WiFi password";  // WiFi Password
const char* mqtt_server = "192.168.1.103";    // IP Broker MQTT
const char *mqtt_username = "MQTT username";  // MQTT username
const char *mqtt_password = "MQTT password";  // MQTT Password
const int mqtt_port = 1883;                   // MQTT port

// PIN Connection to HC-SR04 to measure hopper fill level (optional)
const uint8_t TRIGGER_PIN = 12; //GPIO12 of ESP8266==>D6 for D1 mini
const uint8_t ECHO_PIN = 14; //GPIO14 of ESP8266==>D5 for D1 mini

//*****------------------------------*****//
// DO NOT MODIFY ANYTHING AFTER THIS LINE //
//*****------------------------------*****//

#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "CRC16.h" // CRC16/XMODEM

const char* topic_EspStatus = "Ravelli/EspStatus";                                    // topic to return indicate ESP status
const char* topic_OTAStatus = "Ravelli/OtaStatus";                                    // topic to return indicate OTA status
const char* topic_EspRestart_Cmnd = "Ravelli/EspRestart/Cmnd";                        // topic to restart the ESP
const char* topic_EspRestart = "Ravelli/EspRestart";                                  // topic to confirm the ESP restart
//Action topics:
const char* topic_OnOff = "Ravelli/OnOff";                                            // Topic to switch On or Off the stove
const char* topic_OnOff_TX = "Ravelli/OnOff/Tx";                                      // Topic to return messages sent by ESP
const char* topic_OnOff_RX = "Ravelli/OnOff/Rx";                                      // Topic to return messages received by ESP
const char* topic_RoomTemp = "Ravelli/RoomTemp";                                      // Topic to indicate The room temperature
const char* topic_RoomTemp_TX = "Ravelli/RoomTemp/Tx";                                // Topic to return messages sent by ESP
const char* topic_RoomTemp_RX = "Ravelli/RoomTemp/Rx";                                // Topic to return messages received by ESP
const char* topic_SetpointTemp = "Ravelli/SetpointTemp";                              // Topic to define the target temperature of the room
const char* topic_SetpointTemp_TX = "Ravelli/SetpointTemp/Tx";                        // Topic to return messages sent by ESP
const char* topic_SetpointTemp_RX = "Ravelli/SetpointTemp/Rx";                        // Topic to return messages received by ESP
const char* topic_HeatingPower = "Ravelli/HeatingPower";                              // Topic to define the heating power (1 to 5)
const char* topic_HeatingPower_TX = "Ravelli/HeatingPower/Tx";                        // Topic to return messages sent by ESP
const char* topic_HeatingPower_RX = "Ravelli/HeatingPower/Rx";                        // Topic to return messages received by ESP
const char* topic_FanPower = "Ravelli/FanPower";                                      // Topic to define front fan power (0 to 6)
const char* topic_FanPower1_TX = "Ravelli/FanPower/1Tx";                              // Topic to return messages sent by ESP
const char* topic_FanPower1_RX = "Ravelli/FanPower/1Rx";                              // Topic to return messages received by ESP
const char* topic_FanPower2_TX = "Ravelli/FanPower/2Tx";                              // Topic to return messages sent by ESP
const char* topic_FanPower2_RX = "Ravelli/FanPower/2Rx";                              // Topic to return messages received by ESP
const char* topic_ScrewLoading = "Ravelli/ScrewLoading";                              // Topic to empty the fireplace and fill it with fresh pellet
const char* topic_ScrewLoading_TX = "Ravelli/ScrewLoading/Tx";                        // Topic to return messages sent by ESP
const char* topic_ScrewLoading_RX = "Ravelli/ScrewLoading/Rx";                        // Topic to return messages received by ESP
//Information topics:
const char* topic_StoveStatus = "Ravelli/StoveStatus";                                // Topic to return the status of the stove
const char* topic_StoveStatus_Cmnd = "Ravelli/StoveStatus/Cmnd";                      // Topic to request the status of the stove
const char* topic_StoveStatus_TX = "Ravelli/StoveStatus/Tx";                          // Topic to return messages sent by ESP
const char* topic_StoveStatus_RX = "Ravelli/StoveStatus/Rx";                          // Topic to return messages received by ESP
const char* topic_SetpointTempStatus = "Ravelli/SetpointTempStatus";                  // Topic to return the temperature of the target
const char* topic_SetpointTempStatus_Cmnd = "Ravelli/SetpointTempStatus/Cmnd";        // Topic to request the temperature of the target
const char* topic_SetpointTempStatus_TX = "Ravelli/SetpointTempStatus/Tx";            // Topic to return messages sent by ESP
const char* topic_SetpointTempStatus_RX = "Ravelli/SetpointTempStatus/Rx";            // Topic to return messages received by ESP
const char* topic_HeatingPowerStatus = "Ravelli/HeatingPowerStatus";                  // Topic to return the heating power
const char* topic_HeatingPowerStatus_Cmnd = "Ravelli/HeatingPowerStatus/Cmnd";        // Topic to request the heating power
const char* topic_HeatingPowerStatus_TX = "Ravelli/HeatingPowerStatus/Tx";            // Topic to return messages sent by ESP
const char* topic_HeatingPowerStatus_RX = "Ravelli/HeatingPowerStatus/Rx";            // Topic to return messages received by ESP
const char* topic_FanPowerStatus = "Ravelli/FanPowerStatus";                          // Topic to return the fan power
const char* topic_FanPowerStatus_Cmnd = "Ravelli/FanPowerStatus/Cmnd";                // Topic to request the fan power
const char* topic_FanPowerStatus_TX = "Ravelli/FanPowerStatus/Tx";                    // Topic to return messages sent by ESP
const char* topic_FanPowerStatus_RX = "Ravelli/FanPowerStatus/Rx";                    // Topic to return messages received by ESP
const char* topic_ScrewLoadingTime = "Ravelli/ScrewLoadingTime";                      // Topic to return the loading time (s)
const char* topic_ScrewLoadingTime_Cmnd = "Ravelli/ScrewLoadingTime/Cmnd";            // Topic to request the loading time (s)
const char* topic_ScrewLoadingTime_TX = "Ravelli/ScrewLoadingTime/Tx";                // Topic to return messages sent by ESP
const char* topic_ScrewLoadingTime_RX = "Ravelli/ScrewLoadingTime/Rx";                // Topic to return messages received by ESP
const char* topic_ScrewLoadingRemaining = "Ravelli/ScrewLoadingRemaining";            // Topic to return the remaning loading time (s)
const char* topic_ScrewLoadingRemaining_Cmnd = "Ravelli/ScrewLoadingRemaining/Cmnd";  // Topic to request the remaning loading time (s)
const char* topic_ScrewLoadingRemaining_TX = "Ravelli/ScrewLoadingRemaining/Tx";      // Topic to return messages sent by ESP
const char* topic_ScrewLoadingRemaining_RX = "Ravelli/ScrewLoadingRemaining/Rx";      // Topic to return messages received by ESP
const char* topic_PartialCounter = "Ravelli/PartialCounter";                          // Topic to return the counter since last maintenance (hr)
const char* topic_PartialCounter_Cmnd = "Ravelli/PartialCounter_Cmnd";                // Topic to request the counter since last maintenance (hr)
const char* topic_PartialCounter_TX = "Ravelli/PartialCounter/Tx";                    // Topic to return messages sent by ESP
const char* topic_PartialCounter_RX = "Ravelli/PartialCounter/Rx";                    // Topic to return messages received by ESP
const char* topic_TotalCounter = "Ravelli/TotalCounter";                              // Topic to return the counter since stove installation (hr)
const char* topic_TotalCounter_Cmnd = "Ravelli/TotalCounter/Cmnd";                    // Topic to request the counter since stove installation (hr)
const char* topic_TotalCounter_TX = "Ravelli/TotalCounter/Tx";                        // Topic to return messages sent by ESP
const char* topic_TotalCounter_RX = "Ravelli/TotalCounter/Rx";                        // Topic to return messages received by ESP
const char* topic_StartupCounter = "Ravelli/StartupCounter";                          // Topic to return the number of start-up
const char* topic_StartupCounter_Cmnd = "Ravelli/StartupCounter/Cmnd";                // Topic to request the number of start-up
const char* topic_StartupCounter_TX = "Ravelli/StartupCounter/Tx";                    // Topic to return messages sent by ESP
const char* topic_StartupCounter_RX = "Ravelli/StartupCounter/Rx";                    // Topic to return messages received by ESP
const char* topic_ExhaustTemperature = "Ravelli/ExhaustTemperature";                  // Topic to return the temperature of exhaust gases
const char* topic_ExhaustTemperature_Cmnd = "Ravelli/ExhaustTemperature/Cmnd";        // Topic to request the temperature of exhaust gases
const char* topic_ExhaustTemperature_TX = "Ravelli/ExhaustTemperature/Tx";           // Topic to return messages sent by ESP
const char* topic_ExhaustTemperature_RX = "Ravelli/ExhaustTemperature/Rx";           // Topic to return messages received by ESP
const char* topic_ElectronicTemperature = "Ravelli/ElectronicTemperature";            // Topic to return the temperature of the electronic
const char* topic_ElectronicTemperature_Cmnd = "Ravelli/ElectronicTemperature/Cmnd";  // Topic to request the temperature of the electronic
const char* topic_ElectronicTemperature_TX = "Ravelli/ElectronicTemperature/Tx";      // Topic to return messages sent by ESP
const char* topic_ElectronicTemperature_RX = "Ravelli/ElectronicTemperature/Rx";      // Topic to return messages received by ESP
const char* topic_Custom_Serial_Cmnd = "Ravelli/CustomCmnd/Cmnd";                     // Topic to request a custom command
const char* topic_Custom_Serial_TX = "Ravelli/CustomCmnd/Tx";                         // Topic to return messages sent by ESP
const char* topic_Custom_Serial_RX = "Ravelli/CustomCmnd/Rx";                         // Topic to return messages received by ESP
const char* topic_HopperLevel = "Ravelli/HopperLevel";                                // Topic to return the fill level of the hopper
const char* topic_HopperLevel_Cmnd = "Ravelli/HopperLevel/Cmnd";                      // Topic to request the fill level of the hopper


WiFiClient MQTT2Ravelli;
PubSubClient client(MQTT2Ravelli);

void setup() {
  Serial.begin(4800);
  Serial.swap();
  pinMode(TRIGGER_PIN, OUTPUT); // Sets the TRIGGER_PIN as an Output for the HC-SR04
  pinMode(ECHO_PIN, INPUT); // Sets the ECHO_PIN as an Input for the HC-SR04
  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  connectMQTT();
  RunOTA(); //comment this line if you want to remove OTA
}

void connectWiFi() {
  uint8_t Wifistart = 0;
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Serial.print("Connecting to WiFi");
  while ((WiFi.status() != WL_CONNECTED) && (Wifistart < 16)) {
  //while (WiFi.status() != WL_CONNECTED) {
    Wifistart++;
    delay(1000);
    // Serial.print(".");
  }
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //Serial.println("Connection Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
  
   //Serial.println("");
   //Serial.print("WiFi connected on IP address ");
   //Serial.println(WiFi.localIP());
}

void RunOTA() {
  delay(10);
  
  // Port defaults to 8266
  ArduinoOTA.setPort(8266); //comment out to use default value

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("esp8266- Ravelli"); //comment out to use default value

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    client.publish(topic_OTAStatus, type.c_str());
  });

  ArduinoOTA.onEnd([]() {
    client.publish(topic_OTAStatus, "End");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    String TempForm = "Progress: ";
    TempForm.concat(String(progress));
    TempForm.concat("/");
    TempForm.concat(String(total));
    
    client.publish(topic_OTAStatus, TempForm.c_str());
  });

  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) {
      client.publish(topic_OTAStatus, "Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      client.publish(topic_OTAStatus, "Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      client.publish(topic_OTAStatus, "Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      client.publish(topic_OTAStatus, "Receive Failed");
    } else if (error == OTA_END_ERROR) {
      client.publish(topic_OTAStatus, "End Failed");
    }
  });

  ArduinoOTA.begin();
}

void connectMQTT() {
  while (!client.connected()) {
    if (client.connect("ESPClient", mqtt_username, mqtt_password)) {
      // Serial.println("MQTT broker connected");
      client.publish(topic_EspStatus, "Connected!");
    } else {
      // Serial.print("failed with state ");
      // Serial.println(client.state());
      // Serial.println("New try in 2 seconds");
      delay(2000);
    }
  }

  //Esp Manager topics:
  client.subscribe(topic_EspRestart_Cmnd);

  //Action topics:
  client.subscribe(topic_OnOff);
  client.subscribe(topic_RoomTemp);
  client.subscribe(topic_SetpointTemp);
  client.subscribe(topic_HeatingPower);
  client.subscribe(topic_FanPower);
  client.subscribe(topic_ScrewLoading);
  //Information topics:
  client.subscribe(topic_StoveStatus_Cmnd);
  client.subscribe(topic_SetpointTempStatus_Cmnd);
  client.subscribe(topic_HeatingPowerStatus_Cmnd);
  client.subscribe(topic_FanPowerStatus_Cmnd);
  client.subscribe(topic_ScrewLoadingTime_Cmnd);
  client.subscribe(topic_ScrewLoadingRemaining_Cmnd);
  client.subscribe(topic_PartialCounter_Cmnd);
  client.subscribe(topic_TotalCounter_Cmnd);
  client.subscribe(topic_StartupCounter_Cmnd);
  client.subscribe(topic_ExhaustTemperature_Cmnd);
  client.subscribe(topic_ElectronicTemperature_Cmnd);
  client.subscribe(topic_Custom_Serial_Cmnd);
}

uint16_t PingSensor() {  // Ultrasonic (Ping) Distance Sensor
    // Pulse
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);

    // End Pulse & Calculate didtance
    //pulseIn output is us, while 0.017 = sound velocity[cm/us] / 2
    uint16_t distance = pulseIn(ECHO_PIN, HIGH) * 0.017; //[cm]

    return distance;

}

void loop() {
  
  uint32_t moment = millis();

  while (millis() - moment < 500) {
    ArduinoOTA.handle();
    yield();
  }

  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();
}


// Function to calculate checksum and return complete frame
uint8_t GetChkSum(uint8_t frame[], uint8_t length) {
  Crc16 crc;
  crc.clearCrc();
  uint16_t TtlChkSum;
  // Get checksum
  TtlChkSum = crc.XModemCrc(frame, 0, length - 2);
  // Complete frame
  frame[length - 1] = TtlChkSum >> 0;
  frame[length - 2] = TtlChkSum >> 8;

  return frame[length];
}

// Function to confirm the checksum of stove response
bool ConfirmChkSum(uint8_t frame[], uint8_t length) {
  uint8_t ChkSum1;
  uint8_t ChkSum2;
  Crc16 crc;
  crc.clearCrc();
  uint16_t TtlChkSum;
  // Get checksum
  TtlChkSum = crc.XModemCrc(frame, 0, length - 2);
  ChkSum2 = TtlChkSum >> 0;
  ChkSum1 = TtlChkSum >> 8;

  if (ChkSum1 == frame[length - 2] && ChkSum2 == frame[length - 1]) {
    return true;
  } else {
    return false;
  }
}

// Function to send a variable message and receive a known reply
void VarQuery_FixReply(uint8_t Query[], uint8_t QueryL, uint8_t TReply[], uint8_t TReplyL) {
  uint8_t Compt = 0;
  uint8_t ReceivedHexa[TReplyL];
  uint8_t tag_id_tx = 0;
  String ValTemp_tx = "";
  String ValTemp_rx = "";


  // Get checksum of request and reply
  Query[QueryL] = GetChkSum(Query, QueryL);
  TReply[TReplyL] = GetChkSum(TReply, TReplyL);

  // Purge remaining data on buffer
  while (Serial.available() > 0) Serial.read();

  // Send query to stove and wait for it's reply
  Serial.write(Query, QueryL);
  Serial.flush();
  delay(50);

  // Get reply from stove and compare to theoritical reply
  for (uint8_t i = 0; i < QueryL; i++){
    ReceivedHexa[i] = Serial.read();
    ValTemp_tx.concat(String(ReceivedHexa[i], HEX));
    ValTemp_tx.concat("_");
  }
  if (ReceivedHexa[2] == 0x11 && ReceivedHexa[4] == 0x1){ //RoomTemp
    client.publish(topic_RoomTemp_TX, ValTemp_tx.c_str()); //for testing only
    tag_id_tx = 1;
  }
  else if (ReceivedHexa[2] == 0x2 && ReceivedHexa[4] == 0x53){ //SetPointTemp
    client.publish(topic_SetpointTemp_TX, ValTemp_tx.c_str()); //for testing only
    tag_id_tx = 2;
  }
  else if (ReceivedHexa[2] == 0x2 && ReceivedHexa[4] == 0x52){ //HeatingPower
    client.publish(topic_HeatingPower_TX, ValTemp_tx.c_str()); //for testing only
    tag_id_tx = 3;
  }
  else if (ReceivedHexa[2] == 0x2 && ReceivedHexa[4] == 0x58){ // FanPower1
    client.publish(topic_FanPower1_TX, ValTemp_tx.c_str()); //for testing only
    tag_id_tx = 4;
  }
  else{ //General
    client.publish(topic_EspStatus, ValTemp_tx.c_str()); //for testing only
    tag_id_tx = 0;
  }


  for (uint8_t i = 0; i < TReplyL; i++) {
    ReceivedHexa[i] = Serial.read();
    ValTemp_rx.concat(String(ReceivedHexa[i], HEX));
    ValTemp_rx.concat("_");
    if (ReceivedHexa[i] != TReply[i]) Compt++;
  }
  if (tag_id_tx == 1) client.publish(topic_RoomTemp_RX, ValTemp_rx.c_str()); //for testing only
  else if (tag_id_tx == 2) client.publish(topic_SetpointTemp_RX, ValTemp_rx.c_str()); //for testing only
  else if (tag_id_tx == 3) client.publish(topic_HeatingPower_RX, ValTemp_rx.c_str()); //for testing only
  else if (tag_id_tx == 4) client.publish(topic_FanPower1_RX, ValTemp_rx.c_str()); //for testing only
  else client.publish(topic_EspStatus, ValTemp_rx.c_str()); //for testing only

  // Send back the status to python
  if (Compt == 0) {
    client.publish(topic_EspStatus, "ok");
  } else {
    client.publish(topic_EspStatus, "nok");
  }
}

//Function to send a known message and receive a variable reply
void FixQuery_VarReply(uint8_t Query[], uint8_t QueryL) {
  uint8_t Compt = 0;
  uint8_t Escape = 0;
  String ReplyData = "";

  // Purge remaining data on buffer
  while (Serial.available() > 0) Serial.read();

  // Send query to stove and wait for it's reply
  Serial.write(Query, QueryL);
  Serial.flush();
  delay(250);
}

// Function to send a known message and receive a known reply
void FixQuery_FixReply(uint8_t Query[], uint8_t QueryL, uint8_t TReply[], uint8_t TReplyL) {
  uint8_t ReceivedHexa[TReplyL];
  uint8_t Compt = 0;
  String ValTemp_tx = "";
  String ValTemp_rx = "";

  // Purge remaining data on buffer
  while (Serial.available() > 0) Serial.read();

  // Send query to stove and wait for it's reply
  Serial.write(Query, QueryL);
  Serial.flush();
  delay(50);

  // Get reply from stove and compare to theoritical reply
  for (uint8_t i = 0; i < QueryL; i++){
    ReceivedHexa[i] = Serial.read();
    ValTemp_tx.concat(String(ReceivedHexa[i], HEX));
    ValTemp_tx.concat("_");
  }
  if (ValTemp_tx == "21_0_7_1_C8_4C_") client.publish(topic_OnOff_TX, ValTemp_tx.c_str()); //for testing only
  if (ValTemp_tx == "21_0_10_13_1_A7_87_") client.publish(topic_FanPower2_TX, ValTemp_tx.c_str()); //for testing only
  else client.publish(topic_EspStatus, ValTemp_tx.c_str()); //for testing only


  for (uint8_t i = 0; i < TReplyL; i++){
    ReceivedHexa[i] = Serial.read();
    ValTemp_rx.concat(String(ReceivedHexa[i], HEX));
    ValTemp_rx.concat("_");
  }
  if (ValTemp_tx == "21_0_7_1_C8_4C_") client.publish(topic_OnOff_RX, ValTemp_rx.c_str()); //for testing only
  if (ValTemp_tx == "21_0_10_13_1_A7_87_") client.publish(topic_FanPower2_RX, ValTemp_rx.c_str()); //for testing only
  else client.publish(topic_EspStatus, ValTemp_rx.c_str()); //for testing only

  for (uint8_t i = 0; i < TReplyL; i++) {
    if (ReceivedHexa[i] != TReply[i]) Compt++;
  }

  if (Compt == 0) {
    client.publish(topic_EspStatus, "ok");
  } else {
    client.publish(topic_EspStatus, "nok");
  }
}

void callback(char *msgTopic, byte *msgPayload, unsigned int msgLength) {
  String ReqType = "";
  String ValTemp = "";
  ReqType = msgTopic;
  String ValTemp_tx_call = "";
  String ValTemp_rx_call = "";

  // Get request type and if it exist the requested value
  for (int i = 0; i < msgLength; i++) {
    ValTemp += ((char)msgPayload[i]);
  }
  uint8_t ReqValue = ValTemp.toInt();
  
  /** Get the job done ! **/
  // Action items:
  if (ReqType == topic_OnOff) { // Switch ON or OFF
    uint8_t PreQuery[] = {0x21, 0x00, 0x07, 0x01, 0xC8, 0x4C};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x07, 0x01, 0x00, 0x18, 0xAA};

    FixQuery_FixReply(PreQuery, sizeof(PreQuery), TheoricalReply, sizeof(TheoricalReply));
  }
  else if (ReqType == topic_RoomTemp) { // Send room temperature (°C)
    if (ReqValue >= 0 && ReqValue <= 99) {
      uint8_t PreQuery[] = {0x21, 0x00, 0x11, 0x00, 0x01, ReqValue, 0x00, 0x00};
      uint8_t TheoricalReply[] = {0x11, 0x00, 0x11, 0x00, 0x01, 0xCA, 0x79};

      VarQuery_FixReply(PreQuery, sizeof(PreQuery), TheoricalReply, sizeof(TheoricalReply));
    } else {
      client.publish(topic_EspStatus, "ERROR: Maximum 2 digit number allowed");
    }
  }
  else if (ReqType == topic_SetpointTemp) { // Send setpoint temperature (°C)
    if (ReqValue >= 0 && ReqValue <= 99) {
      uint8_t PreQuery[] = {0x21, 0x00, 0x02, 0x00, 0x53, ReqValue, 0x00, 0x00};
      uint8_t TheoricalReply[] = {0x11, 0x00, 0x02, 0x00, 0x53, ReqValue, 0x00, 0x00};

      VarQuery_FixReply(PreQuery, sizeof(PreQuery), TheoricalReply, sizeof(TheoricalReply));
    } else {
      client.publish(topic_EspStatus, "ERROR: Maximum 2 digit number allowed");
    }
  }
  else if (ReqType == topic_HeatingPower) { // Send heating power (1 to 5)
    if (ReqValue >= 1 && ReqValue <= 5) {
      uint8_t PreQuery[] = {0x21, 0x00, 0x02, 0x00, 0x52, ReqValue, 0x00, 0x00};
      uint8_t TheoricalReply[] = {0x11, 0x00, 0x02, 0x00, 0x52, ReqValue, 0x00, 0x00};

      VarQuery_FixReply(PreQuery, sizeof(PreQuery), TheoricalReply, sizeof(TheoricalReply));
    } else {
      client.publish(topic_EspStatus, "ERROR: Heating power can only be an integer between 1 and 5");
    }
  }
  else if (ReqType == topic_FanPower) { // Send from fan power (0 to 6)
    if (ReqValue >= 0 && ReqValue <= 6) {
      uint8_t PreQuery[] = {0x21, 0x00, 0x02, 0x00, 0x58, ReqValue, 0x00, 0x00};
      uint8_t TheoricalReply[] = {0x11, 0x00, 0x02, 0x00, 0x58, ReqValue, 0x00, 0x00};

      VarQuery_FixReply(PreQuery, sizeof(PreQuery), TheoricalReply, sizeof(TheoricalReply));

      uint8_t PreQuery2[] = {0x21, 0x00, 0x10, 0x13, 0x01, 0xA7, 0x87};
      uint8_t TheoricalReply2[] = {0x11, 0x00, 0x10, 0x13, 0x01, 0x00, 0x6D, 0x81};
      FixQuery_FixReply(PreQuery2, sizeof(PreQuery2), TheoricalReply2, sizeof(TheoricalReply2));
    } else {
      client.publish(topic_EspStatus, "ERROR: Fan power can only be an integer between 0 and 6");
    }
  }
  else if (ReqType == topic_ScrewLoading) { // Request to empty the fireplace and fill it with fresh pellet
    uint8_t PreQuery[] = {0x21, 0x00, 0x07, 0x08, 0x59, 0x65};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x07, 0x08, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }
    client.publish(topic_ScrewLoading_TX, ValTemp_tx_call.c_str()); //for testing only

    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }
    client.publish(topic_ScrewLoading_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      if (Reply[4] == 0) {
        client.publish(topic_EspStatus, "Start of screw loading");
      }
      else if (Reply[4] == 1) {
        client.publish(topic_EspStatus, "Screw loading impossible");
      }
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  // Information items:
  else if (ReqType == topic_StoveStatus_Cmnd) { // Request stove status
    String MyText = "Stove status: ";

    uint8_t PreQuery[] = {0x21, 0x00, 0x10, 0x07, 0x04, 0x38, 0x95};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x10, 0x07, 0x04, 0xA1, 0x00, 0x00, 0xB1, 0x00, 0x00};
    // 11_0_10_7_4_13_0_0_10_5c_ab_ ==> maybe is modulation H2O
    //11_0_10_7_4_9_0_0_9_ac_bf_ ==> during eco mode
    //11_0_10_7_4_11_0_0_6_c3_34_ ==> before entering eco mode
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }
    client.publish(topic_StoveStatus_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }
    client.publish(topic_StoveStatus_RX, ValTemp_rx_call.c_str()); //for testing only
    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      if (Reply[0] == 0) {
        client.publish(topic_EspStatus, "ERROR: incorrect stove response");
      } else {

        switch (Reply[5]) {
        case 0:
          MyText.concat("stopped");
          break;
        case 1:
          MyText.concat("fireplace cleaning");
          break;
        case 2:
          MyText.concat("waiting for pellet arrival");
          break;
        case 3:
          MyText.concat("lighting of the spark plug, arrival of the pellets");
          break;
        case 4:
          MyText.concat("status not documented");
          break;
        case 5:
          MyText.concat("vented fireplace, flame present");
          break;
        case 6:
          MyText.concat("work");
          break;
        case 7:
          MyText.concat("hourglass ; cleaning fireplace");
          break;
        case 8:
          MyText.concat("during shutdown, final cleaning");
          break;
        case 9:
          MyText.concat("eco mode");
          break;
        case 10:
          MyText.concat("fault alarm");
          break;
        case 11:
          MyText.concat("Modulation");
          break;
        case 17:
          MyText.concat("Entering Eco"); //read it after the temp setpoint has been reached, before entering in eco mode. To understand
          break;
        case 18:
          MyText.concat("Pre Entering Eco"); //read it before the state "entering Eco" state[5]=17DEC. to understand
          break;
        case 19:
          MyText.concat("Modulation H2O");
          break;
        case 26:
          MyText.concat("Stopped by error");//read if after the stove turn off for AL-06, attach the ESP and power cycle the stove. if sent On/OFF cmnd, the stove goes off[state 0], resending it goes on.
          break;
        default:
          MyText.concat("Unknown");
          break;
        }

        MyText.concat(" - Alarme state: ");
        switch (Reply[8]) {
        case 0:
          MyText.concat("stop");
          break;
        case 1:
          MyText.concat("break");// read it during state "modulation h20" and hourglass.
          break;
        case 2:
          MyText.concat("in operation");
          break;
        case 3:
          MyText.concat("default");
          break;
        case 4:
          MyText.concat("remote"); //reading it during state "work" if attaching the ESP without power-cycling the stove. To be confirmed
          break;
        case 6:
          MyText.concat("entering Eco"); //read it while state = 17 DEC
          break;
        case 7:
          MyText.concat("cleaning");
          break;
        case 8:
          MyText.concat("pellet hatch open");
          break;
        case 9:
          MyText.concat("waiting");
          break;
        case 10:
          MyText.concat("checking error"); //after AL06, sending on/off i send it into stop, resending on on/off the stove has gone into state=1[fireplace cleaning] and error 10. After this state the stove has gone into fireplac ecleaning and error cleaning. To be confirmed
          break;
        case 12:
          MyText.concat("AL-06"); //read if after the stove turn off for AL-06, attach the ESP and power cycle the stove. if sent On/OFF cmnd, the stove goes off[state 0], resending it goes on.
          break;
        case 14:
          MyText.concat("cleaning"); //read it during the fireplace cleaning state[5]=1 and state[5]=7
          break;
        case 16:
          MyText.concat("in operation");//To be confirmed
          break;
        case 17:
          MyText.concat("prep. pre Eco");//read it one time before the state "entering Eco" state[5]=17DEC. to understand
          break;
        default:
          MyText.concat("unknown");
          break;
        }

        client.publish(topic_StoveStatus, MyText.c_str());
      }
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_SetpointTempStatus_Cmnd) { // Request registered setpoint temperature (°C)
    uint8_t PreQuery[] = {0x21, 0x00, 0x01, 0x00, 0x53, 0xFF, 0x43};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x01, 0x00, 0x53, 0x1f, 0x07, 0x29, 0xAA, 0x00, 0x00}; //11_0_1_0_53_5_0_0_0_47_19_ ==> asking for 21(or 20)
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }

    client.publish(topic_SetpointTempStatus_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }
    client.publish(topic_SetpointTempStatus_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_SetpointTempStatus, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_HeatingPowerStatus_Cmnd) { // Request current heating power
    uint8_t PreQuery[] = {0x21, 0x00, 0x01, 0x00, 0x52, 0xEF, 0x62};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x01, 0x00, 0x52, 0x05, 0x01, 0x05, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }

    client.publish(topic_HeatingPowerStatus_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }

    client.publish(topic_HeatingPowerStatus_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_HeatingPowerStatus, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_FanPowerStatus_Cmnd) { // Request current fan power
    uint8_t PreQuery[] = {0x21, 0x00, 0x01, 0x00, 0x58, 0x4E, 0x28};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x01, 0x00, 0x58, 0x22, 0x00, 0x06, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }

    client.publish(topic_FanPowerStatus_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }

    client.publish(topic_FanPowerStatus_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_FanPowerStatus, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_ScrewLoadingTime_Cmnd) { // Request loading time (s)
    uint8_t PreQuery[] = {0x21, 0x00, 0x10, 0x0b, 0x01, 0x2D, 0x5D};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x10, 0x0b, 0x01, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }

    client.publish(topic_ScrewLoadingTime_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }

    client.publish(topic_ScrewLoadingTime_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_ScrewLoadingTime, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_ScrewLoadingRemaining_Cmnd) { // Request remaining loading time (s)
    uint8_t PreQuery[] = {0x21, 0x00, 0x10, 0x0a, 0x02, 0x2E, 0x0F};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x10, 0x0a, 0x02, 0x00, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }
    client.publish(topic_ScrewLoadingRemaining_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }
    client.publish(topic_ScrewLoadingRemaining_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_ScrewLoadingRemaining, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_PartialCounter_Cmnd) { // Requestpartial counter (h)
    uint8_t PreQuery[] = {0x21, 0x00, 0x06, 0xD1, 0x30};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x06, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }

    client.publish(topic_PartialCounter_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }

    client.publish(topic_PartialCounter_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_PartialCounter, String(((Reply[4] & 0xFF) << 8) + ((Reply[3] & 0xFF) << 0), DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_TotalCounter_Cmnd) { // Request total counter (h)
    uint8_t PreQuery[] = {0x21, 0x00, 0x06, 0xD1, 0x30};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x06, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }

    client.publish(topic_TotalCounter_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }

    client.publish(topic_TotalCounter_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_TotalCounter, String(((Reply[6] & 0xFF) << 8) + ((Reply[5] & 0xFF) << 0), DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_StartupCounter_Cmnd) { // Request number of start-up
    uint8_t PreQuery[] = {0x21, 0x00, 0x06, 0xD1, 0x30};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x06, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }

    client.publish(topic_StartupCounter_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }

    client.publish(topic_StartupCounter_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_StartupCounter, String(((Reply[8] & 0xFF) << 8) + ((Reply[7] & 0xFF) << 0), DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_ExhaustTemperature_Cmnd) { // Request temperature of exhaust gases
    uint8_t PreQuery[] = {0x21, 0x00, 0x10, 0x1e, 0x02, 0xE1, 0xB8};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x10, 0x1e, 0x02, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }

    client.publish(topic_ExhaustTemperature_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }

    client.publish(topic_ExhaustTemperature_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_ExhaustTemperature, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_ElectronicTemperature_Cmnd) { // Request temperature of the electronic
    uint8_t PreQuery[] = {0x21, 0x00, 0x10, 0x12, 0x01, 0x94, 0xB6};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x10, 0x12, 0x01, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++){
      Reply[i] = Serial.read();
      ValTemp_tx_call.concat(String(Reply[i], HEX));
      ValTemp_tx_call.concat("_");
    }

    client.publish(topic_ElectronicTemperature_TX, ValTemp_tx_call.c_str()); //for testing only
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++){
      Reply[i] = Serial.read();
      ValTemp_rx_call.concat(String(Reply[i], HEX));
      ValTemp_rx_call.concat("_");
    }

    client.publish(topic_ElectronicTemperature_RX, ValTemp_rx_call.c_str()); //for testing only

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_ElectronicTemperature, String(Reply[8], DEC).c_str());
      // To be divided by 2
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_Custom_Serial_Cmnd) { // Send Custom Serial Messages

    //add even check on size of ValTemp

    uint8_t CustomCmnd[(sizeof(ValTemp)/2)]; // # of bytes of the commands
    uint8_t CustomCmndLenght = sizeof(ValTemp)/2;
    uint8_t CustomRespLenght = 20; // Max # of bytes of the response
    uint8_t CustomResp[CustomRespLenght]; 
    String ValTempCustTx = "";
    String ValTempCustRx = "";

    for(int i=0; i<sizeof(ValTemp); i++){
      if(i%2 == 0){
        if(ValTemp[i]=='0') CustomCmnd[i/2] = 0x00;
        else if(ValTemp[i]=='1') CustomCmnd[i/2] = 0x10;
        else if(ValTemp[i]=='2') CustomCmnd[i/2] = 0x20;
        else if(ValTemp[i]=='3') CustomCmnd[i/2] = 0x30;
        else if(ValTemp[i]=='4') CustomCmnd[i/2] = 0x40;
        else if(ValTemp[i]=='5') CustomCmnd[i/2] = 0x50;
        else if(ValTemp[i]=='6') CustomCmnd[i/2] = 0x60;
        else if(ValTemp[i]=='7') CustomCmnd[i/2] = 0x70;
        else if(ValTemp[i]=='8') CustomCmnd[i/2] = 0x80;
        else if(ValTemp[i]=='9') CustomCmnd[i/2] = 0x90;
        else if(ValTemp[i]=='A') CustomCmnd[i/2] = 0xA0;
        else if(ValTemp[i]=='B') CustomCmnd[i/2] = 0xB0;
        else if(ValTemp[i]=='C') CustomCmnd[i/2] = 0xC0;
        else if(ValTemp[i]=='D') CustomCmnd[i/2] = 0xD0;
        else if(ValTemp[i]=='E') CustomCmnd[i/2] = 0xE0;
        else if(ValTemp[i]=='F') CustomCmnd[i/2] = 0xF0;
        else CustomCmnd[i/2] = 0x00;//fault to handle better
      }
      else{
        if(ValTemp[i]=='0') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x00;
        else if(ValTemp[i]=='1') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x01;
        else if(ValTemp[i]=='2') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x02;
        else if(ValTemp[i]=='3') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x03;
        else if(ValTemp[i]=='4') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x04;
        else if(ValTemp[i]=='5') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x05;
        else if(ValTemp[i]=='6') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x06;
        else if(ValTemp[i]=='7') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x07;
        else if(ValTemp[i]=='8') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x08;
        else if(ValTemp[i]=='9') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x09;
        else if(ValTemp[i]=='A') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x0A;
        else if(ValTemp[i]=='B') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x0B;
        else if(ValTemp[i]=='C') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x0C;
        else if(ValTemp[i]=='D') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x0D;
        else if(ValTemp[i]=='E') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x0E;
        else if(ValTemp[i]=='F') CustomCmnd[i/2] = CustomCmnd[i/2] + 0x0F;
        else CustomCmnd[i/2] = CustomCmnd[i/2] + 0x00;//fault to handle better
      }
    }
  // Purge remaining data on buffer
  while (Serial.available() > 0) Serial.read();

  // Send query to stove and wait for it's reply
  Serial.write(CustomCmnd, sizeof(CustomCmnd));
  Serial.flush();
  delay(50);
  uint8_t iter_read = 0;


  // Read what you sent
  for (uint8_t i = 0; i < sizeof(CustomCmnd); i++){
    CustomResp[i] = Serial.read();
    ValTempCustTx.concat(String(CustomResp[i], HEX));
    ValTempCustTx.concat("_");
  }
  client.publish(topic_Custom_Serial_TX, ValTempCustTx.c_str()); //for testing only

  //read the responsee
  while(Serial.available() == 0 && iter_read < 4){
    delay(25);
    iter_read++;
  }
  iter_read = 0;
  while (Serial.available() > 0 && iter_read < CustomRespLenght){

    CustomResp[iter_read] = Serial.read();
    ValTempCustRx.concat(String(CustomResp[iter_read], HEX));
    ValTempCustRx.concat("_");
    iter_read++;
  }
  client.publish(topic_Custom_Serial_RX, ValTempCustRx.c_str()); //for testing only
  }
  else if (ReqType == topic_EspRestart_Cmnd) {
    client.publish(topic_EspRestart, "Restarting the ESP");
    delay(1000);
    ESP.restart();
  }
  else if (ReqType == topic_HopperLevel_Cmnd) {
    uint16_t cms = PingSensor();
    client.publish(topic_HopperLevel, String(cms).c_str());
    //if (cms > 45 and cms < 70){
    // 
    //}
  }
  else
    client.publish(topic_EspStatus, "ERROR: request not recognized");
}
