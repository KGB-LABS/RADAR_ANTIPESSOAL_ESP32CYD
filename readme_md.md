# 🛰️ Submarine Radar & Sonar System (CYD + LD2410)

Um sistema visualizador de radar estilo **Sonar Submarino** desenvolvido para o ecossistema **CYD (Cheap Yellow Display / ESP32-2432S028)** integrado ao sensor de radar por ondas milimétricas **HLK-LD2410**.

O projeto realiza a varredura contínua em 360°, plotando a presença e a distância de alvos móveis com atualização em tempo real em um painel HUD lateral estilizado.

---

## 📸 Funcionalidades

- **Interface Estilo Sonar:** Animação fluida de feixe de varredura com grade de radar e HUD retro-futurista.
- **Detecção de Alvo Móvel:** Captura de distância e porcentagem de energia/presença do movimento via comunicação UART.
- **Memória de Ponto Fixado:** Atualiza a posição do alvo no display ao final de cada giro completo (360°).
- **HUD Lateral:** Exibe o status atual (`CONTATO!` / `SEM CONTATO`), distância em centímetros e nível de energia do alvo.

---

## 🛠️ Hardware Necessário

1. **CYD (ESP32 Cheap Yellow Display)** — Placa ESP32 com tela TFT touchscreen integrada.
2. **HLK-LD2410 (mmWave Radar)** — Módulo de radar de presença de 24GHz.
3. **Cabos Jumper (JST-SH / Dupont)**.

---

## 🔌 Esquema de Ligação (Pinagem)

A conexão entre a placa CYD e o módulo LD2410 é realizada através do conector expansão/serial lateral (porta UART2 do ESP32):

| Módulo LD2410 | ESP32 CYD (Conector Ext / UART2) | Função |
| :--- | :--- | :--- |
| **VCC** | **5V / VIN** | Alimentação do Radar |
| **GND** | **GND** | Terra Comum |
| **TX** | **GPIO 35 (RXD2)** | Recepção de Dados do Radar |
| **RX** | **GPIO 22 (TXD2)** | Transmissão de Configuração (Opcional) |

> ⚠️ **Atenção:** O sensor LD2410 requer alimentação de **5V** estável para funcionar corretamente. Certifique-se de conectar o pino VCC na saída de 5V do CYD. Os pinos de sinal TX/RX operam em nível lógico de 3.3V, compatíveis diretamente com o ESP32.

---

## 💻 Bibliotecas Requeridas

Instale as seguintes bibliotecas através do **Library Manager** da Arduino IDE:

1. **TFT_eSPI** (por Bodmer)
   - *Nota:* Certifique-se de configurar o arquivo `User_Setup.h` ou `User_Setup_Select.h` correto para a sua variante da placa CYD.
2. **ld2410** (por npredictable / cpyne)

---

## 🚀 Como Compilar e Enviar

1. Abra o projeto na **Arduino IDE**.
2. Selecione a placa: **ESP32 Dev Module** (ou equivalente de acordo com o seu CYD).
3. Configure a velocidade de Upload para `921600`.
4. Conecte o CYD via cabo USB-C e faça o upload do código.

---

## ⚙️ Configurações do Código

Você pode ajustar as constantes principais diretamente no código fonte:

```cpp
#define LIMIAR_ENERGIA_MINIMA 35 // Nível mínimo de energia de movimento para considerar contato
#define MAX_DIST_CM 600           // Distância máxima do radar em cm (padrão até 6 metros)
```

---

## 📜 Licença

Este projeto está sob a licença MIT. Sinta-se à vontade para modificar e compartilhar!