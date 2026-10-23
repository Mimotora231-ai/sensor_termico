#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESPmDNS.h>

// 🔹 Configuración Wi-Fi
const char* ssid = "INFINITUMFFB5";     
const char* password = "vgdF4FTfYu";    

// 🔹 Configuración del DHT11
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 🔹 Servidor web
AsyncWebServer server(80);

// 🔹 Variables globales
float temperatura = 0.0;
float humedad = 0.0;

unsigned long tiempoPrevio = 0;
const long intervalo = 2000;

// 🔹 Página web principal
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <title>ESP32 - Monitor DHT11</title>
  <style>
    body { font-family: Arial; text-align: center; background:#f0f2f5; }
    .card { background:#fff; padding:20px; margin:20px auto; border-radius:10px; width:300px; box-shadow:0 4px 8px rgba(0,0,0,0.1);}
    .value { font-size:24px; font-weight:bold; }
    .temp { color:#dc3545; }
    .hum { color:#0dcaf0; }
  </style>
</head>
<body>
  <h1>Monitor DHT11</h1>
  <div class="card">
    🌡️ Temperatura: <div class="value temp"><span id="temp-val">0.0</span> °C</div>
  </div>
  <div class="card">
    💧 Humedad: <div class="value hum"><span id="hum-val">0.0</span> %</div>
  </div>
  <script>
    setInterval(() => {
      fetch('/data')
        .then(res => res.json())
        .then(data => {
          document.getElementById('temp-val').innerText = data.temperatura;
          document.getElementById('hum-val').innerText = data.humedad;
        });
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  dht.begin();

  // 🔹 Conexión Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado!");
  Serial.print("🌐 IP asignada: ");
  Serial.println(WiFi.localIP());

  // 🔹 mDNS para acceder con nombre
  if (MDNS.begin("esp32")) {
    Serial.println("Puedes entrar también en: http://esp32.local/");
  }

  // 🔹 Página principal
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // 🔹 Endpoint JSON con datos
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"temperatura\":" + String(temperatura) +
                  ",\"humedad\":" + String(humedad) + "}";
    request->send(200, "application/json", json);
  });

  // 🔹 Solo temperatura
  server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(temperatura) + " °C");
  });

  // 🔹 Solo humedad
  server.on("/hum", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(humedad) + " %");
  });

  server.begin();
}

void loop() {
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoPrevio >= intervalo) {
    tiempoPrevio = tiempoActual;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      temperatura = t;
      humedad = h;
    }

    Serial.printf("Temp: %.1f °C | Hum: %.1f %%\n", temperatura, humedad);
  }
}
