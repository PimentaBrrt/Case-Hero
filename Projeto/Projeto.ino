#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "Dados.h"

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

static const char htmlAntes[] PROGMEM = "<!DOCTYPE html><html lang=\"pt-br\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Case Hero</title><style type=\"text/css\">body {background: #333333;color: #ffffff;font: normal 16px sans-serif;}a {text-decoration: none;color: #ffffff;display: inline-block;background: #9900ff;padding: 10px 20px;border-radius: 5px;}a:hover {background: #8800ee;}a:active {background: #7700dd;}</style></head><body><p><a href=\"/iniciar\">Iniciar!</a></p>";
static const char htmlDepois[] PROGMEM = "</body></html>\r\n";

void enviarPaginaInicial() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(htmlAntes);
  server.sendContent(htmlDepois);
}

int pontos;
int rodadas;
int erros;
int valendo;
int terminado;

void loopJogo() {
  // Sorteia um número entre 0 e 2 (o 3 exclui)
  int sorteio = random(3);
  
  // Apaga todos os LED's e espera um pouco
  digitalWrite(D6, 0);
  digitalWrite(D7, 0);
  digitalWrite(D3, 0);
  delay(500);
  
  // Acende o LED sorteado
  int botaosorteado = D1;
  int ledsorteado = D6;
  if (sorteio == 1) {
    botaosorteado = D2;
    ledsorteado = D7;
  } else if (sorteio == 2) {
    botaosorteado = D5;
    ledsorteado = D3;
  }
  digitalWrite(ledsorteado, 1);
  
  // Começa a medir o tempo
  long tempoInicio = millis();
  long agora = millis();
  int acerto = 0;
  
  // Espera até 500 ms
  while ((agora - tempoInicio) < 500) {
    agora = millis();
    if (digitalRead(botaosorteado)) {
      acerto = 1;
      break;
    }
    delay(10);
  }
  
  // Avança para a próxima rodada
  rodadas = rodadas + 1;
  
  // Verifica se acertou ou não
  if (acerto) {
    pontos = pontos + 1;
    for (int i = 0; i < 5; i = i + 1) {
      digitalWrite(D6, 1);
      digitalWrite(D7, 1);
      digitalWrite(D3, 1);
      delay(150);
      digitalWrite(D6, 0);
      digitalWrite(D7, 0);
      digitalWrite(D3, 0);
      delay(150);
    }
  } else {
    for (int i = 0; i < 5; i = i + 1) {
      digitalWrite(ledsorteado, 1);
      delay(150);
      digitalWrite(ledsorteado, 0);
      delay(150);
    }
    erros = erros + 1;
  }
  
  if (rodadas > 15) {
    valendo = 0;
    terminado = 1;
  }
  
  // Monitora os pontos pela Serial
  Serial.print("Rodadas: ");
  Serial.println(rodadas);
  Serial.print("Erros: ");
  Serial.println(erros);
  Serial.print("Pontos: ");
  Serial.println(pontos);
}

void executarJogo() {
  pontos = 0;
  rodadas = 0;
  erros = 0;
  valendo = 1;
  terminado = 0;
  
  while (valendo != 0) {
    loopJogo();
    delay(10);
  }
}

void enviarJogo() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  executarJogo();

  server.sendContent(htmlAntes);
  String resultado = "<p><b>Pontos:</b> ";
  resultado += pontos;
  resultado += "</p><p><b>Erros:</b> ";
  resultado += erros;
  resultado += "</p>";
  server.sendContent(resultado);

  server.sendContent(htmlDepois);
}

void setup() {
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D5, INPUT);
  
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D3, OUTPUT);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) { Serial.println("MDNS responder started"); }

  server.on("/", enviarPaginaInicial);
  server.on("/iniciar", enviarJogo);
  server.onNotFound(enviarPaginaInicial);

  randomSeed(analogRead(0) + millis());

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  MDNS.update();
}
