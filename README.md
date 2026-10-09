# 📡 Sonar Submarino CYD + Radar LD2410

Projeto de interface gráfica em estilo **Sonar Submarino** utilizando uma placa **ESP32 Cheap Yellow Display (CYD / ESP32-2432S028)** e o sensor de radar de ondas milimétricas (mmWave) **HLK-LD2410**.

O sistema realiza a varredura contínua de movimento em um mostrador circular analógico, com radar estilo varredura militar, exibindo dados de contato, distância e nível de energia em um painel HUD lateral.

---

## 🧰 Componentes Necessários

| Componente | Quantidade | Observação |
| :--- | :---: | :--- |
| **ESP32 CYD** (*Cheap Yellow Display*) | 1 | Display TFT ILI9341 320x240 com touchscreen |
| **Sensor Radar LD2410** (LD2410B/C) | 1 | Sensor de presença e movimento mmWave 24 GHz |
| **Cabo conector JST / Jumper female-female** | 4 | Conexão com os pinos de expansão do CYD |
| **Fonte Micro-USB ou USB-C (5V / 1A+)** | 1 | Alimentação do conjunto |

---

## 🔌 Esquema de Ligação (Wiring Diagram)

A conexão do sensor **LD2410** deve ser feita na porta de expansão UART2 do ESP32 CYD:
