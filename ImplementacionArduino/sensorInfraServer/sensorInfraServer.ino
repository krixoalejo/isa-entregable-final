#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configuración WiFi
const char* ssid = "SSID";
const char* password = "PASSWORD";

// URL del servidor (ajustar IP)
const char *serverUrl = "http://PYTHON_SERVER_IP:5000/api/infrared";

// Configuración LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Dirección I2C común

// Pines sensores infrarrojos
#define CNY1 42
#define CNY2 41
#define CNY3 40
#define CNY4 39
#define CNY5 38
#define CNY6 37

void setup() {
  Serial.begin(115200);
  
  // Configurar pines como entradas
  pinMode(CNY1, INPUT);
  pinMode(CNY2, INPUT);
  pinMode(CNY3, INPUT);
  pinMode(CNY4, INPUT);
  pinMode(CNY5, INPUT);
  pinMode(CNY6, INPUT);

  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sistema IR Activo");

  // Conectar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(0, 1);
    lcd.print("Conectando...   ");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi OK!");
  delay(1000);
}

void loop() {
  // Leer valores de los sensores
  bool cny1 = digitalRead(CNY1);
  bool cny2 = digitalRead(CNY2);
  bool cny3 = digitalRead(CNY3);
  bool cny4 = digitalRead(CNY4);
  bool cny5 = digitalRead(CNY5);
  bool cny6 = digitalRead(CNY6);

  // Mostrar en LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("C1:");
  lcd.print(cny1);
  lcd.print(" C2:");
  lcd.print(cny2);
  lcd.print(" C3:");
  lcd.print(cny3);
  
  lcd.setCursor(0, 1);
  lcd.print("C4:");
  lcd.print(cny4);
  lcd.print(" C5:");
  lcd.print(cny5);
  lcd.print(" C6:");
  lcd.print(cny6);

  // Enviar datos al servidor
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    String payload = String("{") +
                     "\"cny1\":" + cny1 + "," +
                     "\"cny2\":" + cny2 + "," +
                     "\"cny3\":" + cny3 + "," +
                     "\"cny4\":" + cny4 + "," +
                     "\"cny5\":" + cny5 + "," +
                     "\"cny6\":" + cny6 + "}";

    int httpCode = http.POST(payload);
    
    if (httpCode > 0) {
      Serial.print("Datos enviados: ");
      Serial.println(payload);
    } else {
      Serial.print("Error HTTP: ");
      Serial.println(httpCode);
    }
    http.end();
  }
  delay(1000);  // Intervalo de lectura
}
