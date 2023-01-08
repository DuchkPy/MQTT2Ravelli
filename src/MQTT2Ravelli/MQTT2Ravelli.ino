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

//*****------------------------------*****//
// DO NOT MODIFY ANYTHING AFTER THIS LINE //
//*****------------------------------*****//

#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
#include "CRC16.h"

const char* topic_EspStatus = "Ravelli/EspStatus";                          // topic to return indicate ESP status
//Action topics:
const char* topic_OnOff = "Ravelli/OnOff";                                  // Topic to switch On or Off the stove
const char* topic_RoomTemp = "Ravelli/RoomTemp";                            // Topic to indicate The room temperature
const char* topic_SetpointTemp = "Ravelli/SetpointTemp";                    // Topic to define the target temperature of the room
const char* topic_HeatingPower = "Ravelli/HeatingPower";                    // Topic to define the heating power (1 to 5)
const char* topic_FanPower = "Ravelli/FanPower";                            // Topic to define front fan power (0 to 6)
const char* topic_ScrewLoading = "Ravelli/ScrewLoading";                    // Topic to empty the fireplace and fill it with fresh pellet
//Information topics:
const char* topic_StoveStatus = "Ravelli/StoveStatus";                      // Topic to request the status of the stove
const char* topic_SetpointTempStatus = "Ravelli/SetpointTempStatus";        // Topic to request the temperature of the target
const char* topic_HeatingPowerStatus = "Ravelli/HeatingPowerStatus";        // Topic to request the heating power
const char* topic_FanPowerStatus = "Ravelli/FanPowerStatus";                // Topic to request the fan power
const char* topic_ScrewLoadingTime = "Ravelli/ScrewLoadingTime";            // Topic to request the loading time (s)
const char* topic_ScrewLoadingRemaining = "Ravelli/ScrewLoadingRemaining";  // Topic to request the remaning loading time (s)
const char* topic_PartialCounter = "Ravelli/PartialCounter";                // Topic to request the counter since last maintenance (hr)
const char* topic_TotalCounter = "Ravelli/TotalCounter";                    // Topic to request the counter since stove installation (hr)
const char* topic_StartupCounter = "Ravelli/StartupCounter";                // Topic to request the number of start-up
const char* topic_ExhaustTemperature = "Ravelli/ExhaustTemperature";        // Topic to request the temperature of exhaust gases
const char* topic_ElectronicTemperature = "Ravelli/ElectronicTemperature";  // Topic to request the temperature of the electronic

WiFiClient MQTT2Ravelli;
PubSubClient client(MQTT2Ravelli);

void setup() {
  Serial.begin(4800);
  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void connectWiFi() {
  delay(10);
  
  WiFi.begin(ssid, password);
  // Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    // Serial.print(".");
  }
  // Serial.println("");
  // Serial.print("WiFi connected on IP address ");
  // Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!client.connected()) {
    if (client.connect("ESPClient", mqtt_username, mqtt_password)) {
      // Serial.println("MQTT broker connected");
      client.publish(topic_EspStatus, "Connected");
    } else {
      // Serial.print("failed with state ");
      // Serial.println(client.state());
      // Serial.println("New try in 2 seconds");
      delay(2000);
    }
  }

  //Action topics:
  client.subscribe(topic_OnOff);
  client.subscribe(topic_RoomTemp);
  client.subscribe(topic_SetpointTemp);
  client.subscribe(topic_HeatingPower);
  client.subscribe(topic_FanPower);
  client.subscribe(topic_ScrewLoading);
  //Information topics:
  client.subscribe(topic_StoveStatus);
  client.subscribe(topic_SetpointTempStatus);
  client.subscribe(topic_HeatingPowerStatus);
  client.subscribe(topic_FanPowerStatus);
  client.subscribe(topic_ScrewLoadingTime);
  client.subscribe(topic_ScrewLoadingRemaining);
  client.subscribe(topic_PartialCounter);
  client.subscribe(topic_TotalCounter);
  client.subscribe(topic_StartupCounter);
  client.subscribe(topic_ExhaustTemperature);
  client.subscribe(topic_ElectronicTemperature);
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();
  delay(1000);
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
  for (uint8_t i = 0; i < QueryL; i++) Serial.read();
  for (uint8_t i = 0; i < TReplyL; i++) {
    if (Serial.read() != TReply[i]) Compt++;
  }

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

  // Purge remaining data on buffer
  while (Serial.available() > 0) Serial.read();

  // Send query to stove and wait for it's reply
  Serial.write(Query, QueryL);
  Serial.flush();
  delay(50);

  // Get reply from stove and compare to theoritical reply
  for (uint8_t i = 0; i < QueryL; i++) Serial.read();
  for (uint8_t i = 0; i < TReplyL; i++) ReceivedHexa[i] = Serial.read();

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
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

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
  else if (ReqType == topic_StoveStatus) { // Request stove status
    String MyText = "Stove status: ";

    uint8_t PreQuery[] = {0x21, 0x00, 0x10, 0x07, 0x04, 0x38, 0x95};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x10, 0x07, 0x04, 0xA1, 0x00, 0x00, 0xB1, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();
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
        default:
          break;
        }

        MyText.concat(" - Alarme state: ");
        switch (Reply[8]) {
        case 0:
          MyText.concat("stop");
          break;
        case 1:
          MyText.concat("break");
          break;
        case 2:
          MyText.concat("in operation");
          break;
        case 3:
          MyText.concat("default");
          break;
        case 7:
          MyText.concat("cleaning");
          break;
        case 8:
          MyText.concat("pellet hatch open");
          break;
        default:
          break;
        }

        client.publish(topic_StoveStatus, MyText.c_str());
      }
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_SetpointTempStatus) { // Request registered setpoint temperature (°C)
    uint8_t PreQuery[] = {0x21, 0x00, 0x01, 0x00, 0x53, 0xFF, 0x43};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x01, 0x00, 0x53, 0x1f, 0x07, 0x29, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_StoveStatus, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_HeatingPowerStatus) { // Request current heating power
    uint8_t PreQuery[] = {0x21, 0x00, 0x01, 0x00, 0x52, 0xEF, 0x62};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x01, 0x00, 0x52, 0x05, 0x01, 0x05, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_HeatingPowerStatus, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_FanPowerStatus) { // Request current fan power
    uint8_t PreQuery[] = {0x21, 0x00, 0x01, 0x00, 0x58, 0x4E, 0x28};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x01, 0x00, 0x58, 0x22, 0x00, 0x06, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_FanPowerStatus, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_ScrewLoadingTime) { // Request loading time (s)
    uint8_t PreQuery[] = {0x21, 0x00, 0x10, 0x0b, 0x01, 0x2D, 0x5D};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x10, 0x0b, 0x01, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_ScrewLoadingTime, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_ScrewLoadingRemaining) { // Request remaining loading time (s)
    uint8_t PreQuery[] = {0x21, 0x00, 0x10, 0x0a, 0x02, 0x2E, 0x0F};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x10, 0x0a, 0x02, 0x00, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_ScrewLoadingRemaining, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_PartialCounter) { // Requestpartial counter (h)
    uint8_t PreQuery[] = {0x21, 0x00, 0x06, 0xD1, 0x30};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x06, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_PartialCounter, String(((Reply[4] & 0xFF) << 8) + ((Reply[3] & 0xFF) << 0), DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_TotalCounter) { // Request total counter (h)
    uint8_t PreQuery[] = {0x21, 0x00, 0x06, 0xD1, 0x30};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x06, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_TotalCounter, String(((Reply[6] & 0xFF) << 8) + ((Reply[5] & 0xFF) << 0), DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_StartupCounter) { // Request number of start-up
    uint8_t PreQuery[] = {0x21, 0x00, 0x06, 0xD1, 0x30};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x06, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_StartupCounter, String(((Reply[8] & 0xFF) << 8) + ((Reply[7] & 0xFF) << 0), DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_ExhaustTemperature) { // Request temperature of exhaust gases
    uint8_t PreQuery[] = {0x21, 0x00, 0x10, 0x1e, 0x02, 0xE1, 0xB8};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x10, 0x1e, 0x02, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_ExhaustTemperature, String(Reply[8], DEC).c_str());
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  }
  else if (ReqType == topic_ElectronicTemperature) { // Request temperature of the electronic
    uint8_t PreQuery[] = {0x21, 0x00, 0x10, 0x12, 0x01, 0x94, 0xB6};
    uint8_t TheoricalReply[] = {0x11, 0x00, 0x10, 0x12, 0x01, 0xAA, 0x00, 0x00};
    uint8_t Reply[sizeof(TheoricalReply)];

    FixQuery_VarReply(PreQuery, sizeof(PreQuery));
    for (uint8_t i = 0; i < sizeof(PreQuery); i++) Serial.read();
    for (uint8_t i = 0; i < sizeof(TheoricalReply); i++) Reply[i] = Serial.read();

    if (ConfirmChkSum(Reply, sizeof(TheoricalReply))) {
      client.publish(topic_ElectronicTemperature, String(Reply[8], DEC).c_str());
      // To be divided by 2
    } else {
      client.publish(topic_EspStatus, "ERROR: incorrect stove response");
    }
  } else
    client.publish(topic_EspStatus, "ERROR: request not recognized");
}
