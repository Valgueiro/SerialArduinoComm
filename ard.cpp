#include <SerialStream.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <time.h> 
#include <chrono>
#include <thread>
#define PORT "/dev/ttyACM0"

using namespace std;
using namespace LibSerial;

#define TIME 350

#define RED 1
#define GREEN 2
#define BLUE 3
#define BLANK 4
#define LOSE 5

#define BUTTON1 0
#define BUTTON2 1
#define BUTTON3 2
#define CONNECT '9'
#define SEND_DELAY 100

SerialStream serial_port;
int sequence[50] = {};
int sequenceCounter = 0;

int roundd = 0;
int mode = 0;
int pressedButton = 555;
bool lose = false;
bool played = false;
int first = 1;

bool initC = true;

void delay(int time){
  std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

void initComm() {
  serial_port.Open(PORT);
  cout << "opened";
  if (!serial_port.good()) {
    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] "
              << "Error: Could not open serial port." << std::endl;
    exit(1);
  }
  //
  // Set the baud rate of the serial port.
  //
  serial_port.SetBaudRate(SerialStreamBuf::BAUD_9600);
  if (!serial_port.good()) {
    std::cerr << "Error: Could not set the baud rate." << std::endl;
    exit(1);
  }
  //
  // Set the number of data bits.
  //
  serial_port.SetCharSize(SerialStreamBuf::CHAR_SIZE_8);
  if (!serial_port.good()) {
    std::cerr << "Error: Could not set the character size." << std::endl;
    exit(1);
  }
  //
  // Disable parity.
  //
  serial_port.SetParity(SerialStreamBuf::PARITY_NONE);
  if (!serial_port.good()) {
    std::cerr << "Error: Could not disable the parity." << std::endl;
    exit(1);
  }
  //
  // Set the number of stop bits.
  //
  serial_port.SetNumOfStopBits(1);
  if (!serial_port.good()) {
    std::cerr << "Error: Could not set the number of stop bits." << std::endl;
    exit(1);
  }
  //
  // Turn off hardware flow control.
  //
  serial_port.SetFlowControl(SerialStreamBuf::FLOW_CONTROL_NONE);
  if (!serial_port.good()) {
    std::cerr << "Error: Could not use hardware flow control." << std::endl;
    exit(1);
  }
  //
  // Do not skip whitespace characters while reading from the
  // serial port.
  //
  // serial_port.unsetf( std::ios_base::skipws ) ;
  //
  // Wait for some data to be available at the serial port.
  //
  //
  // Keep reading data from serial port and print it to the screen.
  //
  // Wait for some data to be available at the serial port.
  //
  while (serial_port.rdbuf()->in_avail() == 0) {
    usleep(100);
  }
  char out_buf = 110;
  serial_port << out_buf;  //<-- FIRST COMMAND
  delay(SEND_DELAY);
}

int recvSerial() {
  char next_byte = 0;
  while (next_byte == 0 || next_byte == CONNECT)
    serial_port >> next_byte;  // HERE I RECEIVE THE FIRST ANSWER
  next_byte -= '0';
  return next_byte;
}

char sendSerial(char out) {
  serial_port << out;
  delay(SEND_DELAY);
}

void recvSequence(int level) {
  cout << recvSerial();
  // usleep(100);
}

void defeatLeds() {  // luzes piscam quando o jogador perder
  cout << "DEFATED" << endl << endl;
}

void setLed(int led) {  
  if (led == 0) {
    cout << "RED";
    sendSerial(RED);
  } else if (led == 1) {
    cout << "GREEN";
    sendSerial(GREEN);
  } else if (led == 2) {
    cout << "BLUE";
    sendSerial(BLUE);
  } else {
    cout << "Out";
    sendSerial(BLANK);
  }
  delay(TIME);
  cout << endl;
}

void getNewColor() {  // adiciona cor aleatória a sequência
  if (mode == 1) {
    sequence[roundd++] = rand() % 3 +1;
  } else {
    for (int i = 0; i < roundd; i++) {
      sequence[i] = rand() % 3 +1;
    }
    roundd++;
  }
}

void colors() {  // pega cores aleatórias e adiciona na sequência
  getNewColor();

  for (int i = 0; i < roundd; i++) {
    setLed(sequence[i]);
    setLed(4);
  }
}
// --------------- Inserir leitura de Botao AQUI -----------------------
int readButton() {
  char op = 'e';

  while (op != 'a' && op != 's' && op != 'd') {
    cout << "your entry:";
    scanf(" %c", &op);
  }

  switch (op) {
    case 'a':
      return BUTTON1;
    case 's':
      return BUTTON2;
    case 'd':
      return BUTTON3;
    default:
      cout << "option not avaiable";
      break;
  }
}

void waitToPlay() {  // espera jogador jogar
  while (!played) {
    pressedButton = readButton();
    setLed(pressedButton);
    setLed(4);
    played = true;
  }
  played = false;
}

void check() {  
  if (sequence[sequenceCounter] != pressedButton) {
    lose = true;
  }
}

void waitPlayer() {  // espera jogador + verificação de sequência
    cout << "Player's " << " choice:" << endl;

  for (int i = 0; i < roundd; i++) {
    waitToPlay();
    check();
    if (lose) {
      sequenceCounter = 0;
      sendSerial(LOSE);
      break;
    }
    sequenceCounter++;
  }

  sequenceCounter = 0;
  cout << "Finished Player." << endl;
}

void firstMode() {  // primeiro modo de jogo , o padrão
  while (1) {
    if (lose == true) {
      defeatLeds();
      break;
    }

    colors();
    waitPlayer();
    //delay(200);
  }
}

void secondMode() {  // segundo modo de jogo
  while (1) {
    if (lose == true) {
      defeatLeds();
      break;
    }

    colors();
    waitPlayer();
  }
}

int main() {
  initComm();
  cout << "Server started" << endl;
  delay(1000);

  // ------------- Inserir leitura de Modo aqui --------------
  mode = 1;
  switch (mode) {
    case 1:
      cout << "First Mode" << endl;
      firstMode();
      break;
    case 2:
      cout << "Second Mode" << endl;
      secondMode();
      break;
    default:
      cout << "Wrong" << endl;
  }

  serial_port.Close();

  return 0;
}
