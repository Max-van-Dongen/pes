// Including the required Arduino libraries
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
// #define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

// Defining size, and output pins
#define MAX_DEVICES 4
#define CS_PIN 15

// Create a new instance of the MD_Parola class with hardware SPI connection
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void setup() {
  // Intialize the object
  myDisplay.begin();

  // Set the intensity (brightness) of the display (0-15)
  myDisplay.setIntensity(10);

  // Clear the display
  myDisplay.displayClear();

  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.setInvert(true);
  myDisplay.print("!INIT!");
  myDisplay.setInvert(false);
  delay(750);
  myDisplay.displayClear();
  // myDisplay.displayText("Joe mamma", PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);


  // float temp = 5;
  // while (temp <= 80) {
  //   displayTemp(temp);
  //   temp += 1;
  //   delay(100);
  // }
  // displayFire(true);
}

void displayFire(int state) {
  while (true) {
    myDisplay.setInvert(true);
    myDisplay.print("BRAND");
    delay(750);
    myDisplay.setInvert(false);
    myDisplay.print("BRAND");
    delay(750);
  }
}
void displayTemp(float temperature) {
  char tempDisplay[10];
  sprintf(tempDisplay, "%2.1f%cC", temperature, '\xB0');
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(tempDisplay);
}
void loop() {
  displayTemp(19.5);  // Example temperature value
}