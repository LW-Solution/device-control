#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"

String uid;

char *ssid = "nao tem wifi";
char *pwd = "40028922";

char *mqttServer = "test.mosquitto.org";

char *ntpServer = "br.pool.ntp.org";
long gmtOffset = -3;
int daylight = 0;
time_t now;
struct tm timeinfo;

TaskHandle_t task1;
TaskHandle_t task2;

SemaphoreHandle_t mutex;

float temp = 0.0;
float umi = 0.0;

void minhaTask1(void *pvParameters)
{
  Serial.println("Comecou a Task1");
  while(true){
    //regiao crítica
    xSemaphoreTake(mutex, portMAX_DELAY);
    temp = temp +0.24;
    umi = umi + 0.51;
    xSemaphoreGive(mutex);
    delay(1300);
  }
}

void minhaTask2(void *pvParameters)
{
  Serial.println("Comecou a Task2");
  while(true){
    xSemaphoreTake(mutex, portMAX_DELAY);
    temp = temp - 0.09;
    umi = umi - 0.23;
    xSemaphoreGive(mutex);
    delay(800);
  }
}

typedef struct __attribute__((packed)){
  int16_t temperatura;
  int16_t umidade;
  int16_t vel_vento;
  int16_t dir_vento;
} payload_t;

payload_t pay;

WiFiClient wclient;
PubSubClient mqttClient(wclient);

void connectWiFi()
{
  Serial.print("Conectando ");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado com sucesso, com o IP ");
  Serial.println(WiFi.localIP());
}

void connectMqtt()
{
  if (!mqttClient.connected())
  {
    if (mqttClient.connect(uid.c_str()))
    {
      Serial.println("Conectou no MQTT");
    }
    else
    {
      Serial.println("MQTT offline");
      delay(500);
    }
  }
}

void setup() {
  Serial.begin(115200);
  uid = WiFi.macAddress();
  uid.replace(":", "");
  WiFi.begin(ssid, pwd);
  connectWiFi();
  configTime(gmtOffset, daylight, ntpServer);

  mutex = xSemaphoreCreateMutex();

  if (mutex == NULL)
    Serial.println("Erro ao criar o mutex");

  Serial.println("Criando a task 1");
  xTaskCreatePinnedToCore(
    minhaTask1, //funcao da task
    "MinhaTask1", //nome da task
    1000, //tamanho da task
    NULL, //parametros task
    1, //prioridade da task
    &task1, //task handle
    0 //core (loop = 1)
    );

  Serial.println("Criando a task 2");
  xTaskCreatePinnedToCore(
    minhaTask2, //funcao da task
    "MinhaTask2", //nome da task
    1000, //tamanho da task
    NULL, //parametros task
    1, //prioridade da task
    &task2, //task handle
    1 //core (loop = 1)
    );

  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Erro ao acessar o servidor NTP"); 
  }
  else
  {
    Serial.print("A hora agora eh ");
    Serial.println(time(&now));
  }
  mqttClient.setServer(mqttServer, 1883);
}

void sincronizaTempo(void)
{
  //Configurando o tempo
  configTime(gmtOffset, daylight, ntpServer);
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Erro ao acessar o servidor NTP"); 
  }
  else
  {
    Serial.print("Configurado Data/Hora ");
    Serial.println(time(&now));
  }
}

void loop() {
  if (!mqttClient.connected())
  {
    connectMqtt();  
  }

    pay.temperatura = random(20, 35);
    pay.umidade = random(10, 90);
    pay.vel_vento = random(0, 100);
    pay.dir_vento = random(0, 359);

    String json = "{\"uid\":"+ uid + " , \"temperatura\":" + String(pay.temperatura)
    + ", \"umidade\":" + String(pay.umidade)
    + ", \"vel_vento\":" + String(pay.vel_vento)
    + ", \"dir_vento\":" + String(pay.dir_vento)
    + "}";
  
  if ((time(&now) % 120) == 0)
  {
    sincronizaTempo();
    Serial.println("Enviar dados pelo MQTT");
    Serial.println(json);
    Serial.println(json.c_str());
    mqttClient.publish("fatec/lw/dados/", json.c_str());
  }
  xSemaphoreTake(mutex, portMAX_DELAY);
  Serial.print("Temperatura ");
  Serial.println(temp);
  Serial.print("Umidade ");
  Serial.println(umi);
  xSemaphoreGive(mutex);
  delay(1000);
}