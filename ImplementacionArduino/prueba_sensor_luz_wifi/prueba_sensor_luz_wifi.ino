#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Credenciales WiFi
const char* ssid = "SSID";
const char* password = "PASSWORD";

// URL del servidor Python
const char *serverUrl = "http://PYTHON_SERVER_IP:5000/api/endpoint";

// LCD: dirección I2C y tamaño (16 columnas x 2 filas)
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define LDR1 12
#define LDR2 13

void setup() {
  Serial.begin(115200);

  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  lcd.setCursor(0, 1);
  lcd.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi conectado");
  delay(1000);
  lcd.clear();
}

void loop() {
  int ldr1_val = analogRead(LDR1);
  int ldr2_val = analogRead(LDR2);

  // Mostrar en LCD
  lcd.setCursor(0, 0);
  lcd.print("LDR1:");
  lcd.print(ldr1_val);
  lcd.setCursor(0, 1);
  lcd.print("LDR2:");
  lcd.print(ldr2_val);

  // Enviar a servidor
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    String jsonPayload = "{\"ldr1\":" + String(ldr1_val) + ", \"ldr2\":" + String(ldr2_val) + "}";
    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Respuesta:");
      Serial.println(response);
    } else {
      Serial.print("Error POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  delay(2000);
}
