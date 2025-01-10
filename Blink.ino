#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "DESKTOP-OKR9ELB 1657";         
const char* password = "11111111"; 

WiFiServer server(80); 

const int ledPin = 2;    
const int speakerPin = 4;

int internetSpeed = 0; 

String satelliteStatus = "連接成功"; 
String subscriberInfo = "用戶: 張三";   
const float latitude = 25.0330;         // Пример фиксированной широты (Тайбэй)
const float longitude = 121.5654;       

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 28800, 60000); 

unsigned long previousMillis = 0;
const long interval = 3000; 

const int melodyOn[] = {
  262, 294,
};
const int durationOn[] = {
  200, 200, 
};

const int melodyOff[] = {
  523, 494,
};
const int durationOff[] = {
  200, 200, 
}; 

void playTone(int pin, int frequency, int duration) {
  if (frequency == 0) {
    delay(duration);
    return;
  }
  int period = 1000000 / frequency; 
  int halfPeriod = period / 2;
  int cycles = frequency * duration / 1000;
  for (int i = 0; i < cycles; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(pin, LOW);
    delayMicroseconds(halfPeriod);
  }
}

void playMelody(const int *melody, const int *durations, int length) {
  for (int i = 0; i < length; i++) {
    playTone(speakerPin, melody[i], durations[i]);
    delay(20); 
  }
}

void setup() {
  pinMode(ledPin, OUTPUT); 
  digitalWrite(ledPin, LOW); 

  pinMode(speakerPin, OUTPUT); 
  digitalWrite(speakerPin, LOW); 

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Подключение к Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Подключено!");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.update();

  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    internetSpeed = random(50, 150); 
    Serial.print("Обновленная скорость интернета: ");
    Serial.print(internetSpeed);
    Serial.println(" Mbps");
  }

  WiFiClient client = server.available(); 
  if (client) {
    Serial.println("Новый клиент подключен.");
    String request = client.readStringUntil('\r');
    Serial.println("Запрос: " + request);
    client.flush();

    bool ledStateChanged = false;
    bool ledState = digitalRead(ledPin);

    String path = "";
    if (request.indexOf("GET /LED=ON") != -1) {
      path = "/LED=ON";
    } else if (request.indexOf("GET /LED=OFF") != -1) {
      path = "/LED=OFF";
    } else if (request.indexOf("GET /speed") != -1) {
      path = "/speed";
    } else if (request.indexOf("GET /info") != -1) {
      path = "/info";
    }

    if (path == "/LED=ON") {
      digitalWrite(ledPin, HIGH); 
      ledStateChanged = true;
      Serial.println("LED включен.");
      playMelody(melodyOn, durationOn, sizeof(melodyOn)/sizeof(melodyOn[0]));
    } 
    else if (path == "/LED=OFF") {
      digitalWrite(ledPin, LOW); 
      ledStateChanged = true;
      Serial.println("LED выключен.");
      playMelody(melodyOff, durationOff, sizeof(melodyOff)/sizeof(melodyOff[0]));
    }
    else if (path == "/speed") {
      Serial.print("Отправка скорости: ");
      Serial.println(internetSpeed);
      client.print(String(internetSpeed));
      client.stop();
      Serial.println("Клиент отключен.");
      return;
    }
    else if (path == "/info") {
      timeClient.update();
      String currentTime = timeClient.getFormattedTime();

      String json = "{";
      json += "\"satelliteStatus\":\"" + satelliteStatus + "\",";
      json += "\"subscriberInfo\":\"" + subscriberInfo + "\",";
      json += "\"latitude\":" + String(latitude, 4) + ",";
      json += "\"longitude\":" + String(longitude, 4) + ",";
      json += "\"currentTime\":\"" + currentTime + "\"";
      json += "}";

      Serial.println("Отправка информации: " + json);
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.print(json);
      client.stop();
      Serial.println("Клиент отключен.");
      return;
    }

    String html = R"rawliteral(
    <!DOCTYPE HTML>
    <html lang='zh-TW'>
    <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>OrbitalLink Control</title>
      <style>
        body {
          font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
          background: linear-gradient(to right, #ece9e6, #ffffff);
          color: #333;
          text-align: center;
          padding: 40px;
        }
        h1 {
          color: #2c3e50;
          margin-bottom: 20px;
        }
        .button {
          background-color: #2980b9;
          border: none;
          color: white;
          padding: 15px 30px;
          text-align: center;
          text-decoration: none;
          display: inline-block;
          font-size: 18px;
          margin: 10px 5px;
          cursor: pointer;
          border-radius: 5px;
          transition: background-color 0.3s ease;
        }
        .button:hover {
          background-color: #3498db;
        }
        .button.off {
          background-color: #c0392b;
        }
        .button.off:hover {
          background-color: #e74c3c;
        }
        .speed {
          margin-top: 40px;
        }
        #speedValue {
          font-size: 20px;
          margin-top: 10px;
        }
        .info {
          margin-top: 20px;
          text-align: left;
          display: inline-block;
          max-width: 500px;
          width: 100%;
        }
        .info p {
          font-size: 16px;
          margin: 5px 0;
        }
        .note {
          margin-top: 30px;
          font-size: 14px;
          color: #555;
        }
        
        /* Медиазапросы для мобильных устройств */
        @media only screen and (max-width: 600px) {
          .button {
            padding: 10px 20px;
            font-size: 16px;
            margin: 8px 2px;
          }
          .info {
            max-width: 90%;
          }
          #speedValue {
            font-size: 18px;
          }
        }
      </style>
      <script>
        function updateInfo() {
          fetch('/info')
            .then(response => response.json())
            .then(data => {
              document.getElementById('satelliteStatus').innerText = "衛星狀態: " + data.satelliteStatus;
              document.getElementById('subscriberInfo').innerText = "訂閱者資訊: " + data.subscriberInfo;
              document.getElementById('geodata').innerText = "地理位置: " + data.latitude + "°, " + data.longitude + "°";
              document.getElementById('currentTime').innerText = "當前時間: " + data.currentTime;
              document.getElementById('speedValue').innerText = "網路速度: " + "230" + " Mbps";
            })
            .catch(error => console.error('錯誤:', error));
        }
        setInterval(updateInfo, 1000); // Обновление каждые 1 секунду
        window.onload = updateInfo;
      </script>
    </head>
    <body>
      <h1>OrbitalLink Control</h1>
      <div>
        <a href="/LED=ON"><button class='button'>開啟</button></a>
        <a href="/LED=OFF"><button class='button off'>關閉</button></a>
      </div>
      <div class='speed'>
        <h2>網路速度模擬</h2>
        <p id='speedValue'>0 Mbps</p>
      </div>
      <div class="info">
        <p id="satelliteStatus">衛星狀態: </p>
        <p id="subscriberInfo">訂閱者資訊: </p>
        <p id="geodata">地理位置: </p>
        <p id="currentTime">當前時間: </p>
      </div>
      <div class="note">
        886-7-6577711 ext 2082~2088
      </div>
    </body>
    </html>
    )rawliteral";

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.print(html);
    client.stop();
    Serial.println("Клиент отключен.");
  }
}
