#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN  9

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS    56
const int NUMCELLS   =  32;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

boolean pSerialConnected, serialConnected, pBState, bState;
int steps, loadingIndex, preset = 0, loadingInterval = 60;
long timer, adjustHueTimer;
const int columns = 16, rows = 16;
const int buttonPin = 11;
uint32_t coord[columns][rows];
String textData = "";

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  strip.begin(); // This initializes the NeoPixel library.
  /* coord[2][2] = strip.Color(150, 0, 0);
    coord[3][3] = strip.Color(0, 0, 150);
    coord[4][4] = strip.Color(150, 0, 150);
    coord[5][4] = strip.Color(20, 0, 20);
    coord[5][5] = strip.Color(0, 0, 30);
    coord[6][5] = strip.Color(0, 0, 150);
    coord[6][6] = strip.Color(10, 10, 0);
    coord[7][6] = strip.Color(150, 150, 0);
    coord[8][6] = strip.Color(200, 60, 0);
    coord[12][4] = strip.Color(0, 150, 0);*/
  //Serial.setTimeout(500);
  // Serial.begin(19200);
  Serial.begin(38400);

  /*String c="0xFF998800";
    delay(1000);
    Serial.println("yo");*/

  //Serial.println(toHex(c));

  while (!Serial) {
    waitAndListenAnim();
  }
  comfirmAnim();
  pinMode(buttonPin, INPUT_PULLUP);

  /* pinMode(6, OUTPUT);
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);*/
}

void loop() {
  checkForPreset();

  if (!Serial) serialConnected = false;
  else serialConnected = true;
  if (Serial.available() > 0) {


    String index  = Serial.readStringUntil(',');
    String color  = Serial.readStringUntil('\0');
    //String color = Serial.readStringUntil(',');
    // strip.setPixelColor(index.toInt(), int(0xFF998800)); // textdata
    char array[10];
    color.toCharArray(array, 10);
    strip.setPixelColor(index.toInt(), hex2int(array)); // textdata
    //strip.setPixelColor(index.toInt(), 0xFF998800); // textdata
    strip.show();
    //delay(100);

    /*textData = Serial.readStringUntil(' ');
      if (textData.length() > 1) {
      String q[] = splitTokens(textData, ':');
      strip.setPixelColor(0, strip.Color(0, 0, 255)); // textdata
      strip.show();
      delay(100);
      }*/


    /*if (textData.length()> 1) {
      strip.setPixelColor(0, strip.Color(0, 0, 255)); // textdata
      strip.show();
      delay(100);
      } else {
      int inByte = Serial.read();
      strip.setPixelColor(inByte, strip.Color(255, 255, 255)); // Moderately bright green color.
      strip.show();
      delay(100);
      }*/
  }
  /*
    steps++;
    delay(300);
    for (int i = 0; i < NUMPIXELS; i++) {
      strip. setPixelColor(i, coord[i][steps]); // Moderately bright green color.
    }
    strip.show();
    steps = steps % (columns - 1);

  */
  if (!serialConnected && pSerialConnected) {
    disconnectAnim();
    while (!Serial) {
      waitAndListenAnim();
    }
  }
  if (serialConnected  && !pSerialConnected) {
    comfirmAnim();
  }
  pSerialConnected = serialConnected;
}

void disconnectAnim() {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip. setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
  delay(500);
  for (int i = 0; i < NUMPIXELS; i++) {
    strip. setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
  delay(300);

}

void comfirmAnim() {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip. setPixelColor(i, strip.Color(0, 255, 0));
  }
  strip.show();
  delay(500);
  for (int i = 0; i < NUMPIXELS; i++) {
    strip. setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
  delay(300);
}

void waitAndListenAnim() {
  //checkForPreset();
  if (timer + loadingInterval < millis()) {
    timer = millis();
    if (loadingIndex <= 56)loadingIndex++; else loadingIndex = 0;
    for (int i = 0; i < 56; i++) {
      if (loadingIndex == i) strip.setPixelColor(i, strip.Color(100, 100, 100)); // white
      else strip.setPixelColor(i, strip.Color(0, 0, 0)); // black
    }
    strip.show();
  }
}

unsigned int toHex(String x) {
  unsigned int y = 0;
  for (int i = 0; i < x.length(); i++) {
    char c = x.charAt(i);
    if (c < '0' || c > '9') break;
    y *= 10;
    y += (c - '0');
  }
  return y;
}

uint32_t hex2int(char *hex) {
  uint32_t val = 0;
  while (*hex) {
    // get current character then increment
    uint8_t byte = *hex++;
    // transform hex character to the 4bit equivalent number, using the ascii table indexes
    if (byte >= '0' && byte <= '9') byte = byte - '0';
    else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
    else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
    // shift 4 to make space for new digit, and add the 4 bits of the new digit
    val = (val << 4) | (byte & 0xF);
  }
  return val;
}

void checkForPreset() {
  bState = digitalRead(buttonPin);
  if (adjustHueTimer + 1000 < millis() && bState && !pBState) {
    adjustHueTimer = millis();
    delay(65);
    if (preset > 10) {
      preset = 0;
    }
    else {
      preset++;
    }
    for (int i = 41; i < 56; i++) {
      switch (preset) {
        case 0:
          strip.setPixelColor(i, strip.Color(255, 0, 0));
          break;
        case 1:
          strip.setPixelColor(i, strip.Color(255, 255, 0));
          break;
        case 2:
          strip.setPixelColor(i, strip.Color(0, 255, 0));
          break;
        case 3:
          strip.setPixelColor(i, strip.Color(0, 250, 240));
          break;
        case 4:
          strip.setPixelColor(i, strip.Color(0, 0, 255));
          break;
        case 5:
          strip.setPixelColor(i, strip.Color(255, 0, 255));
          break;
        case 6:
          strip.setPixelColor(i, strip.Color(240, 240, 240));
          break;
        case 7:
          strip.setPixelColor(i, strip.Color(100, 100, 100));
          break;
        case 8:
          strip.setPixelColor(i, strip.Color(50, 50, 50));
          break;
        case 9:
          strip.setPixelColor(i, strip.Color(10, 10, 10));
          break;
        case 10:
          strip.setPixelColor(i, strip.Color(0, 0, 0));
          break;
      }
      delay(15);
    }
    //strip.show();

    //Serial.println("change Ambient Color");
  }
  pBState = bState;

}

