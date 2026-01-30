#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <HttpClient.h>


#define LED_RED 25    
#define LED_YELLOW 32 
#define LED_GREEN 33 


Adafruit_AHTX0 aht;
char ssid[50];    
char pass[50];    

enum ComfortLevel {
    COMFORT_GOOD,    
    COMFORT_FAIR,   
    COMFORT_BAD     
};

ComfortLevel previousLevel = COMFORT_GOOD;

// 쾌적도 판단
ComfortLevel checkComfortLevel(float temp, float humidity) {
    if (temp < 10 || temp > 30) {
        return COMFORT_BAD;
    }
    if (humidity < 30 || humidity > 70) {
        return COMFORT_BAD;
    }
    bool tempGood = (temp >= 20 && temp <= 26);
    bool humidGood = (humidity >= 40 && humidity <= 60);
    if (tempGood && humidGood) {
        return COMFORT_GOOD;
    }
    return COMFORT_FAIR;
}


// LED 제어
void updateLEDs(ComfortLevel level) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    
    switch (level) {
        case COMFORT_GOOD:
            digitalWrite(LED_GREEN, HIGH);
            break;
        case COMFORT_FAIR:
            digitalWrite(LED_YELLOW, HIGH);
            break;
        case COMFORT_BAD:
            digitalWrite(LED_RED, HIGH);
            break;
    }
}



void nvs_access() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    Serial.printf("\n");
    Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } else {
        Serial.printf("Done\n");
        Serial.printf("Retrieving SSID/PASSWD\n");

        // SSID 길이 확인
        size_t ssid_len = 0;
        size_t pass_len = 0;
        err = nvs_get_str(my_handle, "ssid", NULL, &ssid_len);
        if (err == ESP_OK && ssid_len <= sizeof(ssid)) {
            nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
        } else {
            Serial.printf("SSID not found or invalid length!\n");
            ssid[0] = '\0';
        }

        // PASSWORD 길이 확인
        err = nvs_get_str(my_handle, "pass", NULL, &pass_len);
        if (err == ESP_OK && pass_len <= sizeof(pass)) {
            nvs_get_str(my_handle, "pass", pass, &pass_len);
        } else {
            Serial.printf("Password not found or invalid length!\n");
            pass[0] = '\0';
        }

        if (ssid[0] != '\0' && pass[0] != '\0') {
            Serial.printf("SSID: %s\n", ssid);
            Serial.printf("Password: %s\n", pass);
        }
    }
    nvs_close(my_handle);
}

void sendToServer(float temp, float humidity, ComfortLevel level) {
    WiFiClient c;
    HttpClient http(c);
    
    String comfort;
    switch (level) {
        case COMFORT_GOOD:
            comfort = "good";
            break;
        case COMFORT_FAIR:
            comfort = "fair";
            break;
        case COMFORT_BAD:
            comfort = "bad";
            break;
    }
    
    String path = String("/?var=Temp:") + temp + "_Humidity:" + humidity + "_Condition:" + comfort;
    http.get("18.188.34.***", 5000, path.c_str(), NULL);
    http.stop();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // LED 핀 설정
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);

    // AHT20 센서 초기화
    if (!aht.begin()) {
        Serial.println("Could not find AHT sensor!");
        while (1) delay(10);
    }

   // WiFi 정보 읽기
   nvs_access();

   // WiFi 연결 시도
   Serial.print("Connecting to ");
   Serial.println(ssid);
   
   WiFi.begin(ssid, pass);
   while (WiFi.status() != WL_CONNECTED) {
       delay(500);
       Serial.print(".");
   }

   Serial.println("");
   Serial.println("WiFi connected");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        sensors_event_t humidity, temp;
        aht.getEvent(&humidity, &temp);
        
        Serial.print("Temperature: "); 
        Serial.print(temp.temperature); 
        Serial.println(" °C");
        Serial.print("Humidity: "); 
        Serial.print(humidity.relative_humidity); 
        Serial.println(" %");
        
        // 현재 상태 확인
        ComfortLevel currentLevel = checkComfortLevel(temp.temperature, humidity.relative_humidity);
        updateLEDs(currentLevel);
        
        // 상태 변경되었을 때만 서버로 전송
        if (currentLevel != previousLevel) {
            // 문자열 생성
            String comfort;
            switch (currentLevel) {
                case COMFORT_GOOD:
                    comfort = "good";
                    break;
                case COMFORT_FAIR:
                    comfort = "fair";
                    break;
                case COMFORT_BAD:
                    comfort = "bad";
                    break;
            }
            
            String path = String("/?var=T:") + temp.temperature + "_H:" + humidity.relative_humidity + "_C:" + comfort;
            WiFiClient c;
            HttpClient http(c);
            http.get("18.188.34.127", 5000, path.c_str(), NULL);
            http.stop();
            
            // 현재 상태를 이전 상태로 저장
            previousLevel = currentLevel;
        }
        
        Serial.print("Comfort Level: ");
        switch (currentLevel) {
            case COMFORT_GOOD:
                Serial.println("Good (Green)");
                break;
            case COMFORT_FAIR:
                Serial.println("Fair (Yellow)");
                break;
            case COMFORT_BAD:
                Serial.println("Bad (Red)");
                break;
        }
        Serial.println("------------------------");
    }
    
    delay(4000);
}
