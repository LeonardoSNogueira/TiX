# Xadrez 1D com ESP32

Este projeto implementa um protótipo funcional de um jogo de Xadrez 1D utilizando um ESP32. A comunicação é feita via Bluetooth, e as partidas são registradas automaticamente na memória do dispositivo para análise posterior.

## Como rodar o código do ESP32 (`main.cpp`)

### 1. Conexões de Hardware
- Faça todas as conexões conforme ilustrado no arquivo:  
  `conexoes/conexoes_prototipo`  
  Isso inclui a ligação dos resistores, jumpers, botões, LEDs e quaisquer periféricos descritos.

### 2. Preparação do Ambiente de Desenvolvimento
Você pode usar **PlatformIO** (recomendado) no **Visual Studio Code** ou a **Arduino IDE**.

#### Opção 1: Usando PlatformIO
1. Instale o [Visual Studio Code](https://code.visualstudio.com/).
2. Instale a extensão **PlatformIO IDE**.
3. Abra a pasta do projeto no VS Code.
4. Certifique-se de que todas as bibliotecas listadas no início do `main.cpp` estejam instaladas (PlatformIO geralmente instala automaticamente com base no `platformio.ini`).

#### Opção 2: Usando Arduino IDE
1. Instale a [Arduino IDE](https://www.arduino.cc/en/software).
2. Adicione o suporte à placa ESP32 (via Gerenciador de Placas).
3. Instale manualmente todas as bibliotecas listadas no início do `main.cpp`.
4. Abra o arquivo `main.cpp` como um novo sketch (ou converta para `.ino` se necessário).

### 3. Configuração da Porta Serial e Upload
1. Conecte seu **ESP32** à porta USB do computador.
2. Abra o **Gerenciador de Dispositivos** (no Windows) e identifique a porta **COM** associada ao ESP32.
3. No PlatformIO ou Arduino IDE:
   - Selecione a placa: `ESP32 Dev Module` ou equivalente.
   - Defina a velocidade da porta serial (**baudrate**): `9600`.
   - Configure a **porta COM** correta.
4. Compile e faça o upload do código para o ESP32.

### 4. Teste e Execução
Após o upload:
- O ESP32 iniciará o sistema automaticamente.
- Será exibido um menu interativo no display LCD, permitindo a navegação e seleção de opções por meio dos botões conectados.
- Uma rede Bluetooth com o nome **TiX** estará disponível para emparelhamento com o computador ou dispositivo móvel.

---

Se tiver dúvidas ou encontrar erros durante a instalação ou execução, verifique:
- Se os drivers USB do ESP32 estão corretamente instalados.
- Se a porta COM correta está selecionada.
- Se as bibliotecas foram corretamente incluídas.
