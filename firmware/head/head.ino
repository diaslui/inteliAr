#include <WiFi.h>
#include <IRremote.hpp>
#include <esp_now.h>


static const uint8_t peerAddress[] = {0x20, 0x6E, 0xF1, 0x6A, 0xC9, 0xFC};

const bool DEBUG = true;
int transmitterPin = 2;

static const uint16_t PowerOnrawData[199] = {4330,4370, 530,1670, 480,620, 480,1670, 480,1670, 480,620, 480,620, 480,1670, 480,620, 480,570, 480,1720, 480,570, 480,620, 480,1670, 480,1670, 530,570, 480,1670, 480,620, 480,620, 480,1670, 480,1670, 480,1670, 530,1670, 480,1670, 480,1670, 480,1670, 530,1670, 480,570, 480,620, 480,620, 480,570, 480,620, 480,620, 480,570, 530,1670, 480,570, 530,1670, 480,570, 480,620, 480,620, 480,570, 530,1620, 530,570, 530,1670, 480,570, 530,1620, 530,1620, 530,1670, 480,1670, 480,5220, 4330,4370, 530,1620, 530,570, 530,1620, 530,1670, 480,570, 530,570, 530,1620, 530,570, 530,570, 480,1670, 530,570, 480,570, 530,1620, 530,1670, 480,570, 530,1670, 480,570, 530,570, 530,1620, 530,1670, 480,1670, 480,1670, 530,1620, 530,1620, 530,1670, 480,1670, 530,570, 480,570, 530,570, 530,570, 480,570, 530,570, 530,570, 480,1670, 530,570, 480,1670, 480,620, 480,620, 480,570, 530,570, 480,1670, 530,570, 480,1670, 480,620, 480,1670, 480,1670, 530,1670, 480,1670, 480};  // Protocol=UNKNOWN Hash=0x7DD87158 100 bits (incl. gap and start) received
static const uint16_t preOffRaw[199] = {4330,4370, 530,1620, 530,570, 530,1670, 480,1670, 480,570, 530,570, 480,1670, 530,570, 480,620, 480,1670, 480,620, 480,570, 530,1670, 480,1670, 480,570, 530,1670, 480,620, 480,570, 480,1670, 530,1670, 480,1670, 480,1670, 480,1670, 480,1720, 480,1620, 530,1670, 480,620, 480,570, 530,570, 480,620, 480,570, 530,570, 480,620, 480,1670, 480,1670, 480,1670, 530,570, 480,620, 480,570, 530,570, 480,1720, 480,570, 480,620, 480,620, 480,1620, 530,1670, 480,1670, 480,1670, 530,5220, 4330,4370, 480,1670, 530,570, 480,1670, 530,1620, 530,570, 480,620, 480,1670, 480,620, 480,620, 480,1620, 530,570, 530,570, 480,1670, 530,1620, 530,570, 480,1670, 530,570, 480,570, 530,1670, 480,1670, 530,1620, 530,1620, 530,1620, 530,1670, 480,1670, 530,1620, 530,570, 530,570, 480,570, 530,570, 530,570, 480,570, 530,570, 530,1620, 530,1620, 530,1670, 480,570, 530,570, 530,570, 480,570, 530,1670, 480,570, 530,570, 530,570, 480,1670, 530,1620, 530,1670, 480,1670, 480};  // Protocol=UNKNOWN Hash=0x26D39198 100 bits (incl. gap and start) received

static const uint16_t PowerOffrawData[199] = {4330,4370, 480,1670, 480,620, 480,1670, 480,1670, 530,570, 480,620, 480,1670, 480,620, 480,570, 530,1670, 480,570, 530,570, 480,1670, 480,1670, 530,570, 480,1670, 530,570, 480,1670, 530,1670, 480,1670, 480,1670, 480,620, 480,1670, 480,1670, 480,1670, 530,570, 480,620, 480,570, 530,570, 480,1670, 530,570, 480,620, 480,1670, 480,1670, 530,1670, 480,570, 480,620, 480,570, 530,570, 480,620, 480,620, 480,570, 530,570, 480,1670, 530,1670, 480,1670, 480,1670, 480,1670, 480,5220, 4330,4370, 530,1670, 480,620, 480,1670, 480,1670, 480,620, 480,620, 480,1670, 480,570, 530,570, 480,1720, 480,570, 480,620, 480,1670, 480,1670, 480,620, 480,1670, 480,620, 480,1670, 480,1670, 530,1620, 530,1670, 480,570, 530,1620, 530,1670, 480,1670, 530,570, 480,620, 480,570, 530,570, 480,1670, 480,620, 480,620, 480,1670, 480,1670, 480,1670, 530,570, 480,570, 530,570, 530,570, 480,570, 530,570, 530,570, 480,570, 530,1670, 480,1670, 530,1620, 530,1620, 530,1670, 480};  // Protocol=UNKNOWN Hash=0x5E7E4778 100 bits (incl. gap and start) received


int queueValue = 0;


bool reponseInterpreter(int val){

    if (DEBUG){
        Serial.println("Interpreter Solv -> ");

    }

    if (val == 111){
        IrSender.sendRaw(PowerOnrawData, 199, 38); 
        return true;
    }
    else if (val == 999)
    {

        IrSender.sendRaw(PowerOffrawData, 199, 38); 
        return true;
    }

    return false;

}

void onReceive(const esp_now_recv_info *info, const uint8_t *data, int len)
{
  int rcvdVal;
  memcpy(&rcvdVal, data, sizeof(rcvdVal));

  if (DEBUG)
  {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             info->src_addr[0], info->src_addr[1], info->src_addr[2],
             info->src_addr[3], info->src_addr[4], info->src_addr[5]);

    Serial.printf("(onReceive) New msg from: %s → %d\n", macStr, rcvdVal);
  }
  reponseInterpreter(rcvdVal);
}

void onSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (DEBUG)
  {
    Serial.print("(onsent) Msg to -> ");
    for (int i = 0; i < 6; i++)
    {
      Serial.printf("%02X", mac_addr[i]);
      if (i < 5)
        Serial.print(":");
    }
    Serial.print(" | Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sucess" : "Fail");
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  IrSender.begin(transmitterPin); 

  if (DEBUG)
  {
    Serial.println("****** Welcome To Klinus ********");
    Serial.println("---- You are in Debug Mode. ----");
    Serial.println("+++ AS: HEAD FIRMWARE +++");
    Serial.print("This device MAC Addr is: ");
        Serial.println(WiFi.macAddress());
    Serial.print("** Lets begin =) **");
  }

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("CRITICAL ERROR: Start ESP32.");
    return;
  }

  esp_now_register_recv_cb(onReceive);
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("CRITICAL ERROR: Add Peer.");
    return;
  }

  Serial.println("Network Ready. (Receiv and Send)");
}

void loop()
{

  if (queueValue != 0)
  {
    esp_err_t result = esp_now_send(peerAddress, (uint8_t *)&queueValue, sizeof(queueValue));

    if (result == ESP_OK)
    {
      queueValue = 0;
      if (DEBUG)
      {
        Serial.printf("New MSG Sended (loop stage) : %d\n", queueValue);
      }
    }
    else if (DEBUG)
    {
      Serial.println("CRITICAL ERROR: Send Msg to peer (loop stage)");
    }
  }
}
