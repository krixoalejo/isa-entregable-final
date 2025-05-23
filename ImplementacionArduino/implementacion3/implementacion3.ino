#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// Configuración WiFi
const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* serverUrl = "http://PYTHON_SERVER_IP:5000/api/data";

// Hardware
LiquidCrystal_I2C lcd(0x27, 20, 4);
#define LDR1 12
#define LDR2 13
#define CO2 14
#define P1 1
#define P2 2
#define CNY1 42
#define CNY2 41
#define CNY3 40
#define CNY4 39
#define CNY5 38
#define CNY6 37
#define LR1 5
#define LY1 4
#define LG1 6
#define LR2 7
#define LY2 15
#define LG2 16

// Variables de estado
volatile bool buttonPressed = false;
unsigned long emergencyStartTime = 0;
bool emergencyMode = false;
volatile int changeCount = 0;
bool nightMode = false;
unsigned long lastChangeTime = 0;
unsigned long phaseStartTime = millis();
String currentMessage = "";
unsigned long messageStartTime = 0;

// Duraciones del ciclo
const unsigned long verdeS1 = 3000;
const unsigned long amarilloS1 = 1000;
const unsigned long rojoS1 = 3000;
const unsigned long verdeS2 = 3000;
const unsigned long amarilloS2 = 1000;

enum Estado { VERDE, AMARILLO, ROJO };
Estado estadoS1 = VERDE;
Estado estadoS2 = ROJO;

// Variables sensores
int vLDR1, vLDR2, vCO2;
int vP1, vP2;
int vCNY1, vCNY2, vCNY3, vCNY4, vCNY5, vCNY6;
unsigned long lastSensorUpdate = 0;

void IRAM_ATTR handleButton1() {
  if(digitalRead(P1) == LOW && !emergencyMode) {
    buttonPressed = true;
  }
}

void IRAM_ATTR handleButton2() {
  if(digitalRead(P2) == LOW && !emergencyMode) {
    buttonPressed = true;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Configurar pines
  pinMode(LR1, OUTPUT); pinMode(LY1, OUTPUT); pinMode(LG1, OUTPUT);
  pinMode(LR2, OUTPUT); pinMode(LY2, OUTPUT); pinMode(LG2, OUTPUT);
  pinMode(P1, INPUT_PULLUP); pinMode(P2, INPUT_PULLUP);
  pinMode(CNY1, INPUT);
  pinMode(CNY2, INPUT);
  pinMode(CNY3, INPUT);
  pinMode(CNY4, INPUT);
  pinMode(CNY5, INPUT);
  pinMode(CNY6, INPUT);
  attachInterrupt(digitalPinToInterrupt(P1), handleButton1, FALLING);
  attachInterrupt(digitalPinToInterrupt(P2), handleButton2, FALLING);

  // Iniciar LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Conectar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  lcd.print("Sistema Activo");
  delay(1000);
}

void controlLuces() {
  digitalWrite(LG1, (estadoS1 == VERDE));
  digitalWrite(LY1, (estadoS1 == AMARILLO));
  digitalWrite(LR1, (estadoS1 == ROJO));
  
  digitalWrite(LG2, (estadoS2 == VERDE));
  digitalWrite(LY2, (estadoS2 == AMARILLO));
  digitalWrite(LR2, (estadoS2 == ROJO));
}

void manejarCiclos() {
  unsigned long tiempoActual = millis();
  unsigned long tiempoFase = tiempoActual - phaseStartTime;

  switch(estadoS1) {
    case VERDE:
      if(tiempoFase >= verdeS1) {
        estadoS1 = AMARILLO;
        phaseStartTime = tiempoActual;
        changeCount++;
        currentMessage = "Cambiando Semaforos";
        messageStartTime = millis();
      }
      break;
      
    case AMARILLO:
      if(tiempoFase >= amarilloS1) {
        estadoS1 = ROJO;
        estadoS2 = VERDE;  // S2 se pone verde 1 segundo después que S1 se pone rojo
        phaseStartTime = tiempoActual;
        changeCount++;
        currentMessage = "Cambiando Semaforos";
        messageStartTime = millis();
      }
      break;
      
    case ROJO:
      if(tiempoFase >= rojoS1) {
        estadoS1 = VERDE;
        estadoS2 = ROJO;   // S2 vuelve a rojo cuando S1 se pone verde
        phaseStartTime = tiempoActual;
        changeCount++;
        currentMessage = "Cambiando Semaforos";
        messageStartTime = millis();
      }
      break;
  }
}

void manejarModoNoche() {
  vLDR1 = analogRead(LDR1);
  vLDR2 = analogRead(LDR2);
  bool nuevoModoNoche = (vLDR1 < 100 || vLDR2 < 100);

  if(nuevoModoNoche != nightMode) {
    nightMode = nuevoModoNoche;
    currentMessage = nightMode ? "Modo Noche" : "Modo Dia";
    messageStartTime = millis();
    
    if(nightMode) {
      digitalWrite(LG1, LOW);
      digitalWrite(LG2, LOW);
      digitalWrite(LR1, LOW);
      digitalWrite(LR2, LOW);
    } else {
      phaseStartTime = millis();
    }
  }

  if(nightMode) {
    static bool amarilloEstado = false;
    if(millis() - lastChangeTime >= 500) {
      amarilloEstado = !amarilloEstado;
      digitalWrite(LY1, amarilloEstado);
      digitalWrite(LY2, amarilloEstado);
      lastChangeTime = millis();
    }
  }
}

void mostrarValoresLCD() {
  lcd.clear();
  
  // Fila 1: LDR1 y CNY1-3
  lcd.setCursor(0, 0);
  lcd.print("L1:");
  lcd.print(vLDR1);
  lcd.setCursor(10, 0);
  lcd.print("C1:");
  lcd.setCursor(10, 1);
  lcd.print(vCNY1);
  lcd.setCursor(14, 0);
  lcd.print("C2:");
  lcd.setCursor(14, 1);
  lcd.print(vCNY2);
  lcd.setCursor(18, 0);
  lcd.print("C3:");
  lcd.setCursor(18, 1);
  lcd.print(vCNY3);

  // Fila 2: LDR2
  lcd.setCursor(0, 1);
  lcd.print("L2:");
  lcd.print(vLDR2);

  // Fila 3: CO2 y CNY4-6
  lcd.setCursor(0, 2);
  lcd.print("CO2:");
  lcd.print(vCO2);
  lcd.setCursor(10, 2);
  lcd.print("C4:");
  lcd.setCursor(10, 3);
  lcd.print(vCNY4);
  lcd.setCursor(14, 2);
  lcd.print("C5:");
  lcd.setCursor(14, 3);
  lcd.print(vCNY5);
  lcd.setCursor(18, 2);
  lcd.print("C6:");
  lcd.setCursor(18, 3);
  lcd.print(vCNY6);

  // Fila 4: Botones
  lcd.setCursor(0, 3);
  lcd.print("B1:");
  lcd.print(vP1);
  lcd.setCursor(5, 3);
  lcd.print("B2:");
  lcd.print(vP2);
}

void loop() {
  manejarModoNoche();
  
  // Manejo de emergencia por botón
  if(buttonPressed && !emergencyMode) {
    emergencyMode = true;
    emergencyStartTime = millis();
    
    // Apagar todos los semáforos excepto rojos
    digitalWrite(LG1, LOW);
    digitalWrite(LY1, LOW);
    digitalWrite(LR1, HIGH);
    digitalWrite(LG2, LOW);
    digitalWrite(LY2, LOW);
    digitalWrite(LR2, HIGH);
    
    currentMessage = "Emergencia!";
    messageStartTime = millis();
    buttonPressed = false;
  }

  if(emergencyMode) {
    if(millis() - emergencyStartTime >= 5000) {
      emergencyMode = false;
      // Reiniciar ciclo normal
      estadoS1 = VERDE;
      estadoS2 = ROJO;
      phaseStartTime = millis();
      controlLuces();
    }
  }
  else if(!nightMode) {
    manejarCiclos();
    controlLuces();
  }

  // Actualizar sensores cada segundo
  if(millis() - lastSensorUpdate > 1000) {
    vCO2 = analogRead(CO2);
    vP1 = digitalRead(P1);
    vP2 = digitalRead(P2);
    vCNY1 = digitalRead(CNY1);
    vCNY2 = digitalRead(CNY2);
    vCNY3 = digitalRead(CNY3);
    vCNY4 = digitalRead(CNY4);
    vCNY5 = digitalRead(CNY5);
    vCNY6 = digitalRead(CNY6);
    lastSensorUpdate = millis();
  }

  // Mostrar en LCD
  mostrarValoresLCD();

  // Enviar datos
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    String payload = String("{") +
      "\"cambios\":" + changeCount + "," +
      "\"modo_noche\":" + (nightMode ? "true" : "false") + "," +
      "\"ldr1\":" + vLDR1 + "," +
      "\"ldr2\":" + vLDR2 + "," +
      "\"co2\":" + vCO2 + "," +
      "\"boton1\":" + vP1 + "," +
      "\"boton2\":" + vP2 + "," +
      "\"cny1\":" + vCNY1 + "," +
      "\"cny2\":" + vCNY2 + "," +
      "\"cny3\":" + vCNY3 + "," +
      "\"cny4\":" + vCNY4 + "," +
      "\"cny5\":" + vCNY5 + "," +
      "\"cny6\":" + vCNY6 + "}";
    http.POST(payload);
    http.end();
  }

  // Consulta modo a Python
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://192.168.1.4:5000/get-mode");
    int httpCode = http.GET();
    if (httpCode == 200) {
      String response = http.getString();
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, response);
      if (!error) {
        const char* variable = doc["variable"];
        int valor = doc["valor"];
        // Actualiza la variable correspondiente
        if (strcmp(variable, "vP1") == 0) {
          vP1 = valor;
          buttonPressed = true;
        }
        else if (strcmp(variable, "vP2") == 0) {
          vP2 = valor;
          buttonPressed = true;
        }
        /*else if (strcmp(variable, "vLDR1") == 0) {
          vLDR1 = valor;
        }
        else if (strcmp(variable, "vLDR2") == 0) {
          vLDR2 = valor;
        }*/
        else if (strcmp(variable, "vCO2") == 0) {
          vCO2 = valor;
        }
        mostrarValoresLCD();
        // Confirma a Python que recibió el cambio
        HTTPClient ack;
        ack.begin("http://192.168.1.4:5000/ack-mode");
        ack.POST(""); // o ack.POST(""), no necesita body
        ack.end();
      }
    }
    http.end();
  }
  
  delay(2000);
}
