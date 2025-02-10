# Tarefa de Comunicação Serial com Raspberry Pi Pico

Este projeto implementa uma comunicação serial utilizando o microcontrolador **Raspberry Pi Pico W**. O projeto é capaz de exibir caracteres em um display OLED SSD1306 e representar números em uma matriz de LEDs WS2812. Além disso, o controle de LEDs RGB é feito por meio de botões físicos.

## Funcionalidades

- **Exibição de caracteres**: Letras maiúsculas, minúsculas e números de 0 a 9 são exibidos no display OLED.
- **Representação de números na matriz de LEDs**: Números de 0 a 9 são representados visualmente na matriz de LEDs WS2812.
- **Controle de LEDs RGB**: Dois LEDs RGB podem ser ligados/desligados utilizando botões físicos.
- **Debounce**: Implementação de debounce para evitar leituras falsas nos botões.

## Requisitos

- **Hardware Necessário**:
  - BitDogLab

## Como Executar

1. **Clone o repositório:**
   ```bash
   git clone https://github.com/GabrielMenezesCarvalho/TarefaComunicacaoSerial.git
   cd TarefaComunicacaoSerial
   ```

2. **Abra o projeto no Visual Studio Code:**
   - Certifique-se de que a extensão do Raspberry Pi Pico está instalada.

3. **Compile o projeto:**
   - Utilize a extensão do Raspberry Pi Pico no VSCode para compilar o projeto.

4. **Execute o projeto:**
   - Conecte sua placa *BitDogLab* ao computador.
   - Clique em **Run** na extensão do VSCode para carregar o programa na placa.

5. **Utilize o monitor serial:**
   - Abra o **Monitor Serial** no VSCode.
   - Envie caracteres de **'a' a 'z'** (maiúsculos e minúsculos) e números de **0 a 9**.

## Funcionamento

- **Caracteres enviados via serial**:
  - Letras e números serão exibidos no display OLED.
  - Números de 0 a 9 também serão representados na matriz de LEDs WS2812.

- **Botões físicos**:
  - **Botão A**: Liga/Desliga o LED RGB verde e exibe o status no display.
  - **Botão B**: Liga/Desliga o LED RGB azul e exibe o status no display.



- **Veja o vídeo de demonstração do projeto:** [https://drive.google.com/file/d/1MepNEI0AccYyFrkIjzKdz3D7IWnfvjO_/view?usp=sharing](https://drive.google.com/file/d/1MepNEI0AccYyFrkIjzKdz3D7IWnfvjO_/view?usp=sharing)

Desenvolvido por Gabriel Menezes Carvalho. 🚀

