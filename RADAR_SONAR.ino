#include <TFT_eSPI.h>
#include <ld2410.h>

TFT_eSPI tft = TFT_eSPI();
ld2410 radar;

// Pinos do conector do CYD (ESP32 Cheap Yellow Display)
#define RXD2 35
#define TXD2 22

#define RADAR_X 110
#define RADAR_Y 120
#define RADAR_R 95
#define MAX_DIST_CM 600

// Cores Estilo Sonar Submarino
#define COR_FUNDO       TFT_BLACK
#define COR_VARREDURA   TFT_GREEN
#define COR_GRADE       0x03E0
#define COR_TEXTO_GLOW  TFT_GREEN
#define COR_ALERTA_MOV  TFT_RED
#define COR_HUD_BORDAS  0x03E0

#define LIMIAR_ENERGIA_MINIMA 35

float sweepAngle = 0;
float lastAngle = 0;

// Memória do Alvo no Sonar
bool alvoDetectadoNesteGiro = false;
bool exibirAlvoNaTela = false;
int alvoDistanciaCm = 0;
int alvoEnergia = 0;
int alvoX = -1;
int alvoY = -1;

void desenharGradeRadar() {
  for (int r = 16; r <= RADAR_R; r += 16) {
    tft.drawCircle(RADAR_X, RADAR_Y, r, COR_GRADE);
  }
  tft.drawFastHLine(RADAR_X - RADAR_R - 5, RADAR_Y, (RADAR_R * 2) + 10, COR_GRADE);
  tft.drawFastVLine(RADAR_X, RADAR_Y - RADAR_R - 5, (RADAR_R * 2) + 10, COR_GRADE);

  // HUD Lateral
  tft.fillRect(220, 0, 100, 240, COR_FUNDO); 
  tft.drawFastVLine(220, 0, 240, COR_HUD_BORDAS);

  tft.setTextColor(COR_TEXTO_GLOW, COR_FUNDO);
  tft.setTextSize(1);
  tft.drawString("SONAR SUB", 235, 10);
  tft.drawString("=================", 223, 20);

  tft.drawString("STATUS:", 225, 40);
  tft.drawString("DISTANCIA:", 225, 105);
  tft.drawString("ENERGIA:", 225, 170);

  tft.drawRect(223, 53, 93, 38, COR_HUD_BORDAS);
  tft.drawRect(223, 118, 93, 38, COR_HUD_BORDAS);
  tft.drawRect(223, 183, 93, 38, COR_HUD_BORDAS);
}

void atualizarPainelHUD(bool ativo, int distCm, int energia) {
  tft.setTextSize(1);

  if (ativo) {
    tft.fillRect(224, 54, 91, 36, COR_FUNDO);
    tft.setTextColor(COR_ALERTA_MOV, COR_FUNDO);
    tft.drawString(" CONTATO! ", 235, 68);

    tft.fillRect(224, 119, 91, 36, COR_FUNDO);
    tft.setTextColor(COR_TEXTO_GLOW, COR_FUNDO);
    tft.setTextSize(2);
    tft.drawString(String(distCm) + "cm", 230, 130);

    tft.fillRect(224, 184, 91, 36, COR_FUNDO);
    tft.setTextColor(COR_TEXTO_GLOW, COR_FUNDO);
    tft.drawString(String(energia) + "%", 245, 195);
  } else {
    tft.fillRect(224, 54, 91, 36, COR_FUNDO);
    tft.setTextColor(COR_TEXTO_GLOW, COR_FUNDO);
    tft.drawString(" SEM ", 250, 62);
    tft.drawString(" CONTATO ", 240, 74);

    tft.fillRect(224, 119, 91, 36, COR_FUNDO);
    tft.fillRect(224, 184, 91, 36, COR_FUNDO);
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(256000, SERIAL_8N1, RXD2, TXD2);

  tft.init();
  tft.invertDisplay(true);
  tft.setRotation(1);
  tft.fillScreen(COR_FUNDO);

  desenharGradeRadar();
  radar.begin(Serial2);
}

void loop() {
  radar.read();

  // 1. CAPTURA DE DADOS DO RADAR
  if (radar.presenceDetected() && radar.movingTargetDetected()) {
    int energia = radar.movingTargetEnergy();
    if (energia >= LIMIAR_ENERGIA_MINIMA) {
      alvoDetectadoNesteGiro = true;
      alvoDistanciaCm = radar.movingTargetDistance();
      alvoEnergia = energia;
    }
  }

  // 2. DETECÇÃO DE FIM DE GIRO COMPLETO (360°)
  if (sweepAngle < lastAngle) {
    if (alvoDetectadoNesteGiro) {
      // Apaga a posição visual antiga do alvo se ele mudou de lugar
      if (alvoX != -1) {
        tft.fillCircle(alvoX, alvoY, 6, COR_FUNDO);
      }

      // Calcula as coordenadas X,Y baseadas na distância atual
      int targetR = map(constrain(alvoDistanciaCm, 0, MAX_DIST_CM), 0, MAX_DIST_CM, 0, RADAR_R);
      
      // Plota o alvo no topo do radar (270°)
      float anguloPlot = 4.71239; 
      alvoX = RADAR_X + cos(anguloPlot) * targetR;
      alvoY = RADAR_Y + sin(anguloPlot) * targetR;

      exibirAlvoNaTela = true;
    } else {
      // Se deu a volta sem contatos novos, limpa o alvo anterior
      if (alvoX != -1) {
        tft.fillCircle(alvoX, alvoY, 6, COR_FUNDO);
        alvoX = -1;
      }
      exibirAlvoNaTela = false;
    }

    // Atualiza o painel lateral
    atualizarPainelHUD(exibirAlvoNaTela, alvoDistanciaCm, alvoEnergia);
    alvoDetectadoNesteGiro = false;
  }
  lastAngle = sweepAngle;

  // 3. ANIMAÇÃO DO FEIXE DE VARREDURA
  int px = RADAR_X + cos(sweepAngle) * RADAR_R;
  int py = RADAR_Y + sin(sweepAngle) * RADAR_R;

  // Desenha a linha verde do feixe
  tft.drawLine(RADAR_X, RADAR_Y, px, py, COR_VARREDURA);
  delay(8);
  
  // Apaga a linha verde (desenha em preto por cima)
  tft.drawLine(RADAR_X, RADAR_Y, px, py, COR_FUNDO);

  // 4. RESTAURACÃO DA GRADE DE FUNDO
  // Redesenha os eixos para reparar trechos apagados pelo feixe
  tft.drawFastHLine(RADAR_X - RADAR_R - 5, RADAR_Y, (RADAR_R * 2) + 10, COR_GRADE);
  tft.drawFastVLine(RADAR_X, RADAR_Y - RADAR_R - 5, (RADAR_R * 2) + 10, COR_GRADE);
  
  // Redesenha os círculos da grade
  for (int r = 16; r <= RADAR_R; r += 16) {
    tft.drawCircle(RADAR_X, RADAR_Y, r, COR_GRADE);
  }

  // 5. REDESENHA O ALVO
  // Sempre desenhado por ÚLTIMO para garantir que o feixe ou a grade não o apaguem
  if (exibirAlvoNaTela && alvoX != -1) {
    tft.fillCircle(alvoX, alvoY, 4, COR_ALERTA_MOV);
    tft.drawCircle(alvoX, alvoY, 6, COR_VARREDURA);
  }

  // Avança o ângulo do feixe
  sweepAngle += 0.07;
  if (sweepAngle >= 6.28318) {
    sweepAngle = 0;
  }
}