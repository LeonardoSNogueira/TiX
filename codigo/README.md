# Como Rodar o Código

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
---

### Como executar o programa Python (`main.py`)

Este programa é responsável por se comunicar com o ESP32 via **Bluetooth (porta COM)** e exibir a interface gráfica do sistema.

#### Passo 1: Instalar as bibliotecas necessárias

Abra o terminal (Prompt de Comando, PowerShell ou terminal do VS Code) e digite:

```bash
pip install pygame pyserial
```

---

#### Passo 2: Emparelhar o ESP32 via Bluetooth

1. Ligue o ESP32.
2. No seu PC com Windows, vá em **Configurações > Dispositivos > Bluetooth e outros dispositivos**.
3. Clique em **"Adicionar dispositivo Bluetooth"** e selecione o dispositivo chamado **TiX** (ou o nome que aparecer).
4. Aguarde até que ele esteja emparelhado.

---

#### Passo 3: Descobrir a porta COM do Bluetooth do ESP32

1. Conecte-se na rede Bluetooth do ESP32.
2. Aperte `Win + X` e selecione **Gerenciador de Dispositivos**.
3. Vá até a seção **Ação** e selecione **Dispositivos e Impressoras**.
4. Selecione o dispositivo que está conectado via Bluetooth e vá em **Propriedades**
5. Selecione a aba **Hardware** e anote a porta COM (exemplo: `COM5`, `COM7`, etc.).

---

#### Passo 4: Atualizar o código com a porta COM correta

Abra o arquivo `main.py` no seu editor de código.

1. Procure a linha (perto do final do código):

   ```python
   SERIAL_PORT_NAME = 'COM3'
   ```

2. Substitua `'COM3'` pela COM correta que você viu no passo anterior.

   **Exemplo:**  
   Se a porta do seu ESP32 for `COM7`, a linha deve ficar assim:

   ```python
   SERIAL_PORT_NAME = 'COM7'
   ```

---

#### Passo 5: Executar o programa

No terminal, dentro da pasta onde está o `main.py`, rode o comando:

```bash
python main.py
```

A interface gráfica do sistema será aberta, e ele começará a se comunicar com o ESP32.

---

**Se algo não funcionar**:

- Verifique se a COM está correta e se o ESP32 está emparelhado.
- Confira se o Bluetooth está ativado.
- Veja se o driver USB/Bluetooth do seu ESP32 está instalado corretamente.

Se tiver dúvidas ou encontrar erros durante a instalação ou execução, verifique:
- Se os drivers USB do ESP32 estão corretamente instalados.
- Se a porta COM correta está selecionada.
- Se as bibliotecas foram corretamente incluídas.
