#if defined(ESP32)
#ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
#if CONFIG_IDF_TARGET_ESP32  // ESP32/PICO-D4
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 32
#define RADAR_TX_PIN 33
#elif CONFIG_IDF_TARGET_ESP32S2
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 9
#define RADAR_TX_PIN 8
#elif CONFIG_IDF_TARGET_ESP32C3
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 4
#define RADAR_TX_PIN 5
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif
#else // ESP32 Before IDF 4.0
#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 32
#define RADAR_TX_PIN 33
#endif
#endif

#include <ld2410.h>
#include <WiFi.h>
#include <esp_now.h>

ld2410 radar;

static const uint8_t peerAddress[] = {0x94, 0xA9, 0x90, 0x96, 0xCE, 0x94};

bool DEBUG = true;
int BLUE_LED_PIN = 0;
int RED_LED_PIN = 1;
int GREEN_LED_PIN = 2;
int queueValue = 0;

uint32_t lastReading = 0;
bool isPowerOn = false;

uint32_t lastMoviment = 0;
int movimentLimitCM = 5 * 100;
int stationaryLimitCM = 5 * 100;

uint32_t windowStart = 0;
uint32_t windowDuration = 10000; // 10s
unsigned long detections = 0;
unsigned long totalReads = 0;


void setLedColor(int colorNumber)
{
  if (colorNumber == 0){ // red
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);
  }
  else if (colorNumber == 1){ // green
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);
  }
  else if (colorNumber == 2){ // blue
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, HIGH);
  }
  else if (colorNumber == 3){ // yellow
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);

  }
  else if (colorNumber == 99){ // white
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, HIGH);
  }
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

    MONITOR_SERIAL.printf("(onReceive) New msg from: %s → %d\n", macStr, rcvdVal);
  }
}

void onSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if (DEBUG)
  {
    MONITOR_SERIAL.print("(onsent) Msg to -> ");
    for (int i = 0; i < 6; i++)
    {
      MONITOR_SERIAL.printf("%02X", mac_addr[i]);
      if (i < 5)
        MONITOR_SERIAL.print(":");
    }
    MONITOR_SERIAL.print(" | Status: ");
    MONITOR_SERIAL.println(status == ESP_NOW_SEND_SUCCESS ? "Sucess" : "Fail");
    if (status){
      setLedColor(1);
    }else{
      setLedColor(0);
    }
  }
}

void setup()
{

  setLedColor(99);
  MONITOR_SERIAL.begin(115200);
  WiFi.mode(WIFI_STA);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);


#if defined(ESP32)
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
#elif defined(__AVR_ATmega32U4__)
  RADAR_SERIAL.begin(256000);
#endif

  delay(500);

  if (DEBUG)
  {
    MONITOR_SERIAL.println("****** Welcome To Klinus ********");
    MONITOR_SERIAL.println("---- You are in Debug Mode. ----");
    MONITOR_SERIAL.println("+++ AS: RADIO FIRMWARE +++");
    MONITOR_SERIAL.print("This device MAC Addr is: ");
    MONITOR_SERIAL.println(WiFi.macAddress());
    MONITOR_SERIAL.print("** Lets begin =) **");
  }

  if (esp_now_init() != ESP_OK)
  {
    setLedColor(0);
    MONITOR_SERIAL.println("CRITICAL ERROR: Start ESP32.");
    return;
  }

  if (radar.begin(RADAR_SERIAL))
  {
    if (DEBUG)
    {
      MONITOR_SERIAL.println(F("Sucess. Radar conected"));
    }
  }
  else
  {
    setLedColor(0);
    MONITOR_SERIAL.println(F("CRITICAL ERROR: Radar not conected"));
  }

  windowStart = millis();
  esp_now_register_recv_cb(onReceive);
  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    setLedColor(0);
    MONITOR_SERIAL.println("CRITICAL ERROR: Add Peer.");
    return;
  }

  MONITOR_SERIAL.println("Network Ready. (Receiv and Send)");
  setLedColor(1);

}

void updateLastmoviment()
{
  lastMoviment = millis();
}

void loop()
{

  radar.read();

  if (radar.isConnected() && millis() - lastReading > 10)
  {
    lastReading = millis();
    totalReads++;

    bool detected = false;

    if (radar.presenceDetected())
    {
      if (radar.stationaryTargetDetected())
      {
        auto distance = radar.stationaryTargetDistance();
        if (distance <= stationaryLimitCM)
        {
          detected = true;
          updateLastmoviment();
        }
      }
      // psbl radar.movingTargetDetected()
    }

    if (detected)
    {
      detections++;
    }

    if (millis() - windowStart >= windowDuration)
    {
      setLedColor(3);

      float ratio = (totalReads > 0) ? (detections * 100.0 / totalReads) : 0;

      if (DEBUG)
      {
        MONITOR_SERIAL.print("++ In :");
        MONITOR_SERIAL.print(windowDuration);
        MONITOR_SERIAL.print("ms -> ");
        MONITOR_SERIAL.print(ratio, 1);
        MONITOR_SERIAL.println("% moviment");
      }

      if (ratio > 80.0)
      {
        if (!isPowerOn)
        {
          if (DEBUG)
          {
            setLedColor(2);
            MONITOR_SERIAL.print(" ON ---> Sending [Power On] in head device.");
            queueValue = 111;
          }
          isPowerOn = true;
        }
      }
      else if (ratio < 20.0)
      {
        if (isPowerOn)
        {
          if (DEBUG)
          {
            setLedColor(2);
            MONITOR_SERIAL.print(" OFF ---> Sending [Power Off] in head device.");
            queueValue = 999;
          }
          isPowerOn = false;
        }
      }

      windowStart = millis();
      detections = 0;
      totalReads = 0;
    }
  }

  if (queueValue != 0)
  {
    esp_err_t result = esp_now_send(peerAddress, (uint8_t *)&queueValue, sizeof(queueValue));

    if (result == ESP_OK)
    {
      queueValue = 0;
      if (DEBUG)
      {
        MONITOR_SERIAL.printf("New MSG Sended (loop stage) : %d\n", queueValue);
      }
    }
    else if (DEBUG)
    {
      setLedColor(0);
      MONITOR_SERIAL.println("CRITICAL ERROR: Send Msg to peer (loop stage)");
    }
  }
}
