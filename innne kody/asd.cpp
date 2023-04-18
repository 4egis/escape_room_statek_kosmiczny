#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    100
#define LED_PIN2    7 //losowo migajace ledy nad siedzeniami
#define NUM_LEDS2   100 //losowo migajace ledy nad siedzeniami
#define LED_PIN3    8
#define NUM_LEDS3   100 

#define IN_PIN1      2
#define IN_PIN2      3
#define IN_PIN3      4
#define IN_PIN4      5

#define MAINLIGHTPIN   9
#define VIBRATIONSPIN 11

//variables for randomLeds function
unsigned long int lastRandomLeds = 0; //last time randomLeds was fully executed

//variables for vibrations function
bool vibrations = false;
unsigned long int lastVibrations = 0; //last time vibrations state was changed

int barSpeed = 30; // variable for function floatingBarSmouthSpdChange

//variables for allPulsatingSpdChange function
int pulsingSpeed = 8;
int offTime = 2000;

// variables for sixPartsRandom function
int lastPart = 0; 
long int lastSixPartsRandom = 0; //last time sixPartsRandom was fully executed


//Stage 2 variables
const unsigned int stage2Phase1Duration = 35000;
const unsigned long int stage2Phase2Duration = stage2Phase1Duration + 19000;
const unsigned long int stage2Phase3Duration = stage2Phase2Duration + 41000;
const unsigned long int stage2Phase4Duration = stage2Phase3Duration + 32000;
bool stage2Started = false;
unsigned long int stage2StartTime = 0;

CRGB leds[NUM_LEDS];
CRGB leds3[NUM_LEDS3];
CRGB leds2[NUM_LEDS2]; //losowo migajace ledy nad siedzeniami

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812, LED_PIN2, GRB>(leds2, NUM_LEDS2);
  FastLED.addLeds<WS2812, LED_PIN3, GRB>(leds3, NUM_LEDS3);
  pinMode(IN_PIN1, INPUT);
  pinMode(IN_PIN2, INPUT);
  pinMode(IN_PIN3, INPUT);
  pinMode(IN_PIN4, INPUT);

  Serial.begin(9600);

}

void pulsatingVibrations(int duration, int pause){
  if (vibrations == false){
    if (millis() - lastVibrations < pause)
      return;
    
    vibrations = true;  
    digitalWrite(VIBRATIONSPIN, LOW); //odwrocona logika - niski stan wlaczenie
    
    lastVibrations = millis();
  }
  else{
    if (millis() - lastVibrations < duration)
      return;
    
    vibrations = false;
    digitalWrite(VIBRATIONSPIN, HIGH); //odwrocona logika - wysoki stan wylaczenie
    
    lastVibrations = millis();
  }
}

void setLeds(int start, int end, CRGB color) {
  for (int i = start; i < end; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

void setAllLeds(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
    leds2[i] = color;
    leds3[i] = color;
  }
  FastLED.show();
}

void randomLeds(int minSpace){
  if (millis() - lastRandomLeds < minSpace)
    return;

  for (int i = 0; i < NUM_LEDS2; i++) {
    leds2[i] = CRGB::Black;
  }
  
  for (int i = 0; i < NUM_LEDS2; i++) {
    if (random8(0, 100) < 30)
      leds2[i] = CRGB(random8(15, 200), random8(15, 200), random8(15, 200));
  }
}

void floatingBar(int barLength, CRGB barColor, CRGB backgroundColor, int speed)
{
  setAllLeds(backgroundColor);
  setLeds(0, barLength, barColor);
  
  for (int i = 0; i < NUM_LEDS - barLength; i++)
  {
    leds[i] = backgroundColor;
    leds[i + barLength] = barColor;
    FastLED.show();
    delay(speed);
  }

  for(int i = NUM_LEDS - barLength; i > 0; i--)
  {
    leds[i + barLength] = backgroundColor;
    leds[i] = barColor;
    FastLED.show();
    delay(speed);
  }
}

void floatingBarSmouthSpdChange(int barLength, CRGB barColor, CRGB backgroundColor, int maxSpeed, int SpdChange)
{
  setAllLeds(backgroundColor);
  setLeds(0, barLength, barColor);
  
  for (int i = 0; i < NUM_LEDS - barLength; i++)
  {
    leds[i] = backgroundColor;
    leds[i + barLength] = barColor;
    FastLED.show();
    delay(barSpeed);
  }

  if(barSpeed > maxSpeed)
    barSpeed -= SpdChange;
  
  for(int i = NUM_LEDS - barLength; i > 0; i--)
  {
    leds[i + barLength] = backgroundColor;
    leds[i] = barColor;
    FastLED.show();
    delay(barSpeed);
  }

  if(barSpeed > maxSpeed)
    barSpeed -= SpdChange;

}

void allPulsating(CRGB color, int speed) {
  for (int i = 0; i < 255; i += speed) {
    setAllLeds(color);
    FastLED.setBrightness(i);
    FastLED.show();
    delay(10);
  }
  for (int i = 255; i >= 0; i -= speed/2) {
    setAllLeds(color);
    FastLED.setBrightness(i);
    FastLED.show();
    delay(10);
  }

  FastLED.setBrightness(0);
  FastLED.show();
  delay(1500);
  
}

void allPulsatingSpdChange(CRGB color, int SpdChange, int maxSpeed, int minOffTime, int offChange) {
  
  for (int i = 0; i < 255; i += pulsingSpeed) {
    setAllLeds(color);
    FastLED.setBrightness(i);
    FastLED.show();
    delay(7);
  }
  for (int i = 255; i >= 0; i -= pulsingSpeed/2) {
    setAllLeds(color);
    FastLED.setBrightness(i);
    FastLED.show();
    delay(7);
  }

  FastLED.setBrightness(0);
  FastLED.show();
  delay(offTime);
  Serial.print("pulsingSpeed: ");
  Serial.println(pulsingSpeed);
  Serial.print("offTime: ");
  Serial.println(offTime);

  if(pulsingSpeed < maxSpeed)
    pulsingSpeed += SpdChange;

  if(offTime > minOffTime){
    offTime -= offChange;
    if(offTime < minOffTime)
      offTime = minOffTime;
  }
    
}



void threeParts(CRGB color, unsigned int speed) {
  setAllLeds(CRGB::Black);
  setLeds(0, NUM_LEDS/3, color);
  FastLED.show();
  delay(speed);
  setLeds(NUM_LEDS/3, NUM_LEDS/3*2, color);
  setLeds(0, NUM_LEDS/3, CRGB::Black);
  FastLED.show();
  delay(speed);
  setLeds(NUM_LEDS/3*2, NUM_LEDS, color);
  setLeds(NUM_LEDS/3, NUM_LEDS/3*2, CRGB::Black);
  FastLED.show();
  delay(speed);
  setLeds(NUM_LEDS/3, NUM_LEDS/3*2, color);
  setLeds(NUM_LEDS/3*2, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(speed);
  
}

void sixPartsRandom(CRGB color, int speed)
{
  if(millis() - lastSixPartsRandom < speed)
    return;

  lastSixPartsRandom = millis();
  FastLED.setBrightness(255);
  setAllLeds(CRGB::Black);
  int part = random8(6);
  
  while(lastPart == part){
    part = random8(6);
  }
  setLeds(NUM_LEDS/6*part, NUM_LEDS/6*(part+1), color);
  lastPart = part;
}

void stage1() {
  digitalWrite(MAINLIGHTPIN, LOW);
  floatingBar(15, CRGB::Red, CRGB::Blue, 30);  
}

void stage2() {
  digitalWrite(MAINLIGHTPIN, HIGH);
  if (!stage2Started) {
    Serial.println("Stage 2 started");
    stage2StartTime = millis();
    stage2Started = true;
  }
  Serial.println(millis() - stage2StartTime);
  if (millis() - stage2StartTime < stage2Phase1Duration){
    Serial.println("Stage 2 phase 1");
    FastLED.setBrightness(100);
    setAllLeds(CRGB::Blue);
  }
  else if (millis() - stage2StartTime < stage2Phase2Duration){
    Serial.println("Stage 2 phase 2");
    FastLED.setBrightness(255);
    pulsatingVibrations(500, 1000);
    allPulsatingSpdChange(CRGB::Blue, 7, 150, 20, 125);
  }
  else if (millis() - stage2StartTime < stage2Phase3Duration){
    FastLED.setBrightness(0);
    pulsatingVibrations(300, 300);
  }
  else if (millis() - stage2StartTime < stage2Phase4Duration){
    FastLED.setBrightness(255);
    //strobo uruchamia się 3 razy po 5 z przerwami 5 sekundowymi.
    sixPartsRandom(CRGB::Blue, 300);
  }
  else{
    Serial.println("Stage 2 last phase");
    sixPartsRandom(CRGB::Blue, 300);
  }
}

void stage3()
{
  //tutaj ma jakis nowy pomysl
  return;
}

void stage4()
{
  allPulsating(CRGB::Red, 5);
}

void loop() {
  FastLED.setBrightness(100);
  setAllLeds(CRGB::Blue);
  /*if (digitalRead(IN_PIN1) == HIGH){
    Serial.println("Stage 1");
    stage1();
  }
  else if (digitalRead(IN_PIN2) == HIGH){
    Serial.println("Stage 2");
    stage2();
  }
  else if (digitalRead(IN_PIN3) == HIGH){
    Serial.println("Stage 3");
    stage3();
  }
  else if (digitalRead(IN_PIN4) == HIGH){
    Serial.println("Stage 4");
    stage4();
  }*/

  
  
}

/*
2 tryb :
P1:
Pasek led daje lekkie ambientowe światło 

P2:
Po 35 sekundach  załącza się wibracja ( pulsacyjne) i zaczyna się  miganie białego światła ( do ciemności )
 i to przyspiesza, przez 19 sekund. 

P3:
Potem światło całkowicie gaśnie i tylko wibracje zostają ( tu najmocniejsze wibracje musza być )

P4:
 od 95 sekundy trwania tego trybu zaczyna się miganie na zmianę z przerwami leda podzielonego na 3 czesci. 
 Miga to białym światłem. + strobo uruchamia się 3 razy po 5 z przerwami 5 sekundowymi. 

P5:
Potem już zostaje tylko to miganie leda.
*/