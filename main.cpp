#include <Arduino.h>
#include <vector>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "BluetoothSerial.h"

#define VALOR_ANALOGICO_CAVALO 511 // 100OHN (COR RESISTOR = MPMD)
#define VALOR_ANALOGICO_REI 1424 // = 330 OHN (COR RESISTOR = LLMD)
#define VALOR_ANALOGICO_TORRE 1063 // = 220 OHN (COR RESISTOR = VVMD)
#define VALOR_ANALOGICO_VAZIO 4095 // = 0 OHN

#define TOLERANCIA 200 //Tolerância das leituras dos valores analógicos

#define PINO_CASA0 34
#define PINO_CASA1 35
#define PINO_CASA2 32
#define PINO_CASA3 33
#define PINO_CASA4 25
#define PINO_CASA5 26
#define PINO_CASA6 27
#define PINO_CASA7 14

#define PINO_BOTAO_ESQUERDA 15
#define PINO_BOTAO_CENTRO 4
#define PINO_BOTAO_DIREITA 5

#define PINO_BUZZER 21

#define PINO_SDA 23
#define PINO_SCL 22

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#erro Bluetooth nao esta habilitado! Por favor, execute "make menuconfig" e habilite-o
#endif

using namespace std;

LiquidCrystal_I2C lcd(0x27, 20, 4);
BluetoothSerial SerialBT;

vector<int> casas = {PINO_CASA0, PINO_CASA1, PINO_CASA2, PINO_CASA3, PINO_CASA4, PINO_CASA5, PINO_CASA6, PINO_CASA7};
vector<int> botoes = {PINO_BOTAO_ESQUERDA, PINO_BOTAO_CENTRO, PINO_BOTAO_DIREITA};
vector<char> estado_atual = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
vector<char> estado_anterior = {'V', 'V', 'V', 'V', 'V', 'V', 'V', 'V'}; // C = Cavalo, R = Rei, T = Torre, V = Vazio

void CapturaEstadoAtual();
void PrintaEstadoAtual();
void EnviaEstadoAtual();

void setup()
{
  Serial.begin(9600);
  SerialBT.begin("TiX");

  Wire.begin(PINO_SDA, PINO_SCL);

  for (int i = 0; i < casas.size(); i++)
    pinMode(casas.at(i), INPUT);

  for (int i = 0; i < botoes.size(); i++)
    pinMode(botoes.at(i), INPUT_PULLUP);

  pinMode(PINO_BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();
}

void loop()
{
  lcd.setCursor(8,1);
  lcd.print("TiX");

  CapturaEstadoAtual();
  
  if(estado_atual != estado_anterior)
  {
    PrintaEstadoAtual();
    estado_anterior = estado_atual;
    EnviaEstadoAtual();
  }
}

void CapturaEstadoAtual()
{
  for(int i=0; i<casas.size(); i++)
  {
    if (analogRead(casas.at(i)) > VALOR_ANALOGICO_CAVALO - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_CAVALO + TOLERANCIA)
      estado_atual.at(i) = 'C';
    else if (analogRead(casas.at(i)) > VALOR_ANALOGICO_REI - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_REI + TOLERANCIA)
      estado_atual.at(i) = 'R';
    else if (analogRead(casas.at(i)) > VALOR_ANALOGICO_TORRE - TOLERANCIA && analogRead(casas.at(i)) < VALOR_ANALOGICO_TORRE + TOLERANCIA)
      estado_atual.at(i) = 'T';
    else
      estado_atual.at(i) = 'V';
  }
}

void PrintaEstadoAtual()
{
  for(int i=0; i<estado_atual.size(); i++)
    Serial.print(estado_atual.at(i));

  Serial.println();
}

void EnviaEstadoAtual()
{
  String string_estado_atual = "";

  for(int i=0; i<estado_atual.size(); i++)
    string_estado_atual += estado_atual.at(i);
  
  SerialBT.println(string_estado_atual);
}