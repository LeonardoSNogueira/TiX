# TiX - Tabuleiro Inteligente de Xadrez

![Logo TiX](codigo/assets/images/logo.png)

Projeto desenvolvido como pré-projeto da disciplina **Redes sem Fio (DEC7563)** do curso de **Engenharia de Computação** da **Universidade Federal de Santa Catarina – Campus Araranguá**, sob orientação da professora **Dra. Analucia Schiaffino Morales**.

Autores: **Lucas Porto Ribeiro** e **Leonardo Silveira Nogueira**  
Ano: **2025**

---

## 🎯 Motivações

Como jogador de xadrez, a análise das partidas é essencial para evoluir no jogo. No entanto, durante partidas rápidas, a notação manual se torna impraticável, pois o tempo gasto anotando compromete o desempenho. Assim, surgiu a ideia de desenvolver um tabuleiro físico capaz de registrar automaticamente os lances e transmiti-los para um computador, eliminando a necessidade de anotações manuais.

---

## 💡 Inspiração

Embora existam soluções comerciais para esse problema, muitas delas utilizam técnicas diferentes e componentes de hardware sofisticados e caros. O **TiX** propõe uma alternativa de baixo custo e com maior flexibilidade, utilizando leitura analógica com resistores para identificação das peças, viabilizando futuras expansões para variantes mais complexas do jogo.

---

## 📘 Descrição Geral

O **TiX** é um tabuleiro inteligente para o **Xadrez Unidimensional (1D)** — uma versão simplificada do xadrez tradicional, composta por 8 casas em linha reta e 6 peças no total. O sistema permite a execução de partidas físicas com **registro automático dos lances** e **transmissão via Bluetooth** para uma aplicação desenvolvida em Python no computador, onde os dados podem ser armazenados, visualizados e analisados posteriormente.

### ⚙️ Como funciona:
- Cada peça possui um resistor com valor único.
- Ao posicionar a peça no tabuleiro, forma-se um divisor de tensão.
- O **ESP32** lê os valores de tensão por pinos analógicos e identifica as peças com base nesses valores.
- Os dados da jogada são transmitidos via Bluetooth para o computador.
- A interface no computador processa e registra as informações, permitindo verificação de legalidade do lance e controle da partida.

### 🖥 Interface do usuário:
- **Display LCD 20x4 (I2C)**: exibe o menu, status da conexão, e tempos dos jogadores.
- **Botões físicos**: utilizados para controle de tempo, seleção de menu e confirmação de jogadas.
- **Buzzer**: fornece feedback sonoro para ações do sistema.

---

## 🚀 Funcionalidades

- ✅ **Leitura analógica do estado do tabuleiro** via resistores.
- ✅ **Interface no LCD** com menu e navegação por botões.
- ✅ **Controle de tempo integrado** para ambos os jogadores.
- ✅ **Comunicação via Bluetooth** com aplicação no computador.
- ✅ **GUI em Python (Pygame)** para visualização e controle da partida.
- ✅ **Armazenamento automático** das partidas em arquivos.
- ✅ **Verificação de legalidade dos lances** e detecção de fim de jogo.
- ✅ **Modo de análise de partidas anteriores**.

---

## 📎 Documentos do Projeto

- 📄 [Relatório Final](relatorio/relatorio_final.pdf)  
- 🎞️ [Apresentação em Slides](apresentacao/apresentacao.pdf)

---
