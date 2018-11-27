//-------------------------------------------------------------------------------
//  TinyCircuits ST BLE TinyShield UART Example Sketch
//  Last Updated 2 March 2016
//
//  This demo sets up the BlueNRG-MS chipset of the ST BLE module for compatiblity 
//  with Nordic's virtual UART connection, and can pass data between the Arduino
//  serial monitor and Nordic nRF UART V2.0 app or another compatible BLE
//  terminal. This example is written specifically to be fairly code compatible
//  with the Nordic NRF8001 example, with a replacement UART.ino file with
//  'aci_loop' and 'BLEsetup' functions to allow easy replacement. 
//
//  Written by Ben Rose, TinyCircuits http://tinycircuits.com
//
//-------------------------------------------------------------------------------


#include <SPI.h>
#include <STBLE.h>
#include <TinyScreen.h>
#include <Wire.h>
TinyScreen display = TinyScreen(0);

const int isObstaclePin = 12; // This is our input pin
int isObstacle = HIGH;  // HIGH MEANS NO OBSTACLE
int AMMO = 15;          // Initial ammo count
int curAmmo = AMMO;     // Ammo to decrement
int irState = 0;         // current state of the ir
int lastIrState = 0;     // previous state of the ir
int currSelection = 0;
String selection[6] = {"", "Rush A", "Rush B", "Plant", "Defend", "BREACH"};
int selSize = 6;

//Debug output adds extra flash and memory requirements!
#ifndef BLE_DEBUG
#define BLE_DEBUG true
#endif

#if defined (ARDUINO_ARCH_AVR)
#define SerialMonitorInterface Serial
#elif defined(ARDUINO_ARCH_SAMD)
#define SerialMonitorInterface SerialUSB
#endif


uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_connection_state = false;
#define PIPE_UART_OVER_BTLE_UART_TX_TX 0

#define BLACK           0x00
#define BLUE            0xE0
#define RED             0x03
#define YELLOW          0x1F
#define WHITE           0xFF

void setup() {
  BLEsetup();
  Wire.begin();
  pinMode(isObstaclePin, INPUT);
  Serial.begin(9600);
  display.begin();
  display.setFont(liberationSans_14ptFontInfo);
  display.setCursor(30, 20);
  display.fontColor(WHITE, BLACK);
  display.print(15);
  display.setCursor(0, 00);
  display.setFont(liberationSans_8ptFontInfo);
  display.fontColor(BLUE, BLACK);
  display.print("  AMMO ");
  display.fontColor(YELLOW, BLACK);
  display.print(" COUNTER");
}


void loop() {
  aci_loop();//Process any ACI commands or events from the NRF8001- main BLE handler, must run often. Keep main loop short.
  
  if (ble_rx_buffer_len)//Get messages from the bluetooth buffer
  {
    int input = (int)ble_rx_buffer[0]-48;
    if(input < selSize && input > 0)
    {
      display.setFont(liberationSans_10ptFontInfo);
      display.setCursor(0, 0);
      display.fontColor(RED, BLACK);
      display.print("                                   ");
      display.setCursor(0, 0);
      display.print(selection[((int)ble_rx_buffer[0]-48)]);
    }
    ble_rx_buffer_len = 0;//clear afer reading
  }

  if (display.getButtons(TSButtonUpperLeft)) //Reload Function
  {
    curAmmo = 15;
    display.setCursor(23, 50);
    display.print("                      ");
  }
  if (curAmmo > 0) {
    irState = digitalRead(isObstaclePin);
    display.setFont(liberationSans_14ptFontInfo);
    display.setCursor(30, 20);
    display.fontColor(WHITE, BLACK);
    if (irState != lastIrState) 
    {
      if (irState == LOW) // Dart detected
      {
        // AMMO COUNT
        curAmmo = curAmmo - 1;
      //display.print("     ");
      //display.setCursor(30, 20);
      }
    }
    if (curAmmo > 9)
    {
      display.print(curAmmo);
    } 
    else 
    {
      display.print("0");
      display.print(curAmmo);
    }
    display.setFont(liberationSans_10ptFontInfo);
    display.setCursor(50, 30);
    display.print("/");
    display.print(AMMO);
    lastIrState = irState;
  }
  // Out of AMMO, prompt reload
  if (curAmmo < 1) {
    display.setFont(liberationSans_10ptFontInfo);
    display.setCursor(23, 50);
    display.fontColor(WHITE, BLACK);
    display.print("RELOAD");
  }
  if(display.getButtons(TSButtonLowerLeft)) //Change command sellection
  {
    delay(300);
    currSelection++;
    currSelection = currSelection % selSize;
    display.setCursor(0, 0);
    display.fontColor(BLUE, BLACK);
    display.print("                                   ");
    display.setCursor(0, 0);
    if (currSelection !=0)
    {
    display.setFont(liberationSans_10ptFontInfo);
    display.print(">" + selection[currSelection]);
    }
    else
    {
      display.setFont(liberationSans_8ptFontInfo);
      display.print("  AMMO ");
      display.fontColor(YELLOW, BLACK);
      display.print(" COUNTER");
    }
  }
  if (display.getButtons(TSButtonLowerRight)) { //Send Command via bluetooth
    delay(500);//should catch input
    if (currSelection != 0)
    {
    uint8_t sendBuffer[2];
    uint8_t sendLength = 2;
    sendBuffer[0] = currSelection + 48;
    sendBuffer[1] = '\0';
      if (!lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, (uint8_t*)sendBuffer, sendLength))
      {
      }
    }
  }
}
