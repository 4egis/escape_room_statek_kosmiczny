#include <FastLED.h>

#define LED_PIN_42   6
#define LED_42_NUMBER 42

#define LED_PIN1     7
#define LED_PIN2     8

#define IN_PIN1      2
#define IN_PIN2      3
#define IN_PIN3      4
#define IN_PIN4      5

#define MAINLIGHTPIN   9
#define STROBEPIN        10
#define VIBRATIONSPIN 11
#define PIN_ELO2      12

//tutaj poki co globalnie dla krotszego paska
#define NUM_LEDS2    113
double scale = double(273)/double(113);
CRGB leds2[NUM_LEDS2];

CRGB leds_42[LED_42_NUMBER];

int strobe_counter = 0;

bool stage1_on = true;
bool stage2_on = false;
bool stage3_on = false;
bool stage4_on = false;

unsigned long int currentStageStart;

class TimeObject {
  protected:
    unsigned long last_computed;
    unsigned int compute_pause;
    bool is_on;
    int PIN;

  public:
    TimeObject(int pin, unsigned int _compute_pause) {
      PIN = pin;
      compute_pause = _compute_pause;
      is_on = false;
    }

    virtual void compute(int stage, int phase) = 0;

    void loop(int stage, int phase) {
      if (is_on == false)
        return;

      if (millis() - last_computed < compute_pause)
        return;

      compute(stage, phase);
      last_computed = millis();
    }
};


template<int pin>
class LedStripe : public TimeObject {
  private:
    bool bar_returning;
    int bar_length;
    int bar_index;

    int pulsing_speed; // for pulsation
    int index; // for pulsation
    bool returning; // for pulsation

    int lastPart;
    
    int min_pause;
    int NUM_LEDS;
    
    CRGB *leds; // Change this to a pointer
    CRGB background_color;
    CRGB color;

  public:
     LedStripe(unsigned int _compute_pause, int led_number, CRGB _color, CRGB _background_color) : TimeObject(pin, _compute_pause) {
      bar_returning = false;
      is_on = true;
      bar_length = 25; // Initialize bar_length to a desired value
      bar_index = 0;
      index = 0;
      lastPart = 0;
      pulsing_speed = 1;
      NUM_LEDS = led_number;
      background_color = _background_color;
      color = _color;
      leds = new CRGB[NUM_LEDS]; // Allocate memory for the leds array
      min_pause = 50;
      FastLED.addLeds<WS2812, pin, GRB>(leds, NUM_LEDS); // Pass the pin parameter to the addLeds() function
      FastLED.addLeds<WS2812, LED_PIN2, GRB>(leds2, NUM_LEDS2); // Pass the pin parameter to the addLeds() function
    }

    ~LedStripe() {
      delete[] leds; // Free memory when the object is destroyed
    }

    void setAllLeds(CRGB color) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
      }
      for (int i = 0; i < NUM_LEDS2; i++) {
        leds2[i] = color;
      }
    }

    

    void setLeds(int start, int end, CRGB color) {
      for (int i = start; i < end; i++) {
        leds[i] = color;
      }
    }

    void setLeds2(int start, int end, CRGB color) {
      for (int i = start; i < end; i++) {
        leds2[i] = color;
      }
    }

    void floatingBar(){
      if(bar_returning == false){
            if (bar_index < NUM_LEDS - bar_length){
                leds[bar_index] = background_color;
                leds[bar_index + bar_length] = color;
                
                leds2[int( bar_index / scale )] = background_color;
                leds2[int( (bar_index + bar_length) / scale )] = color;

                bar_index++;
            }
            else{
                bar_returning = true;
            }   
        }
        else{
            if (bar_index > 0){
                leds[bar_index] = color;
                leds[bar_index + bar_length] = background_color;
            
                leds2[int( bar_index / scale )] = color;
                leds2[int( (bar_index + bar_length) / scale )] = background_color;
                
                bar_index--;
            }
            else{
                bar_returning = false;
            }
        }
    }

    void allPulsating() {
      if(returning == false){
        if (index < 255){
          FastLED.setBrightness(index);
          index += int( pulsing_speed / 10 ) + 1;
          if( index > 255 ) index = 255;
        }
        else{
            returning = true;
        }   
      }
      else{
        if (index > 0){
          FastLED.setBrightness(index);
          index -= int( pulsing_speed / 10 ) + 1;
          if( index < 0 ) index = 0;
        }
        else{
            returning = false;
        }   
      }
    }
  
  
    void sixPartsRandom(CRGB color)
  {

    FastLED.setBrightness(255);
    setAllLeds(CRGB::Black);
    int part = random8(6);
    
    while(lastPart == part){
      part = random8(6);
    }
    setLeds(NUM_LEDS/6*part, NUM_LEDS/6*(part+1), color);
    setLeds2(NUM_LEDS2/6*part, NUM_LEDS2/6*(part+1), color);
    lastPart = part;
  }

    void stage1Setup()
    {
      for(int i = 0; i < NUM_LEDS; i++){
        leds[i] = background_color;
      }
      for(int i = 0; i < bar_length; i++){
        leds[i] = color;
      }

      for(int i = 0; i < NUM_LEDS2; i++){
        leds2[i] = background_color;
      }
      for(int i = 0; i < int( bar_length / scale); i++){
        leds2[i] = color;
      }
      FastLED.setBrightness(250);
    }

    void stage1() {
        //floating bar:
        floatingBar();
    }

    void stage2Setup() {
      index = 0;
      color = CRGB::White;
      returning = false;
      compute_pause = 80;
      min_pause = 20;
      setAllLeds(color);
      digitalWrite(STROBEPIN, HIGH);
      digitalWrite(VIBRATIONSPIN, HIGH);
      digitalWrite(MAINLIGHTPIN, HIGH);
    }

    void stage2(int phase) {
      if( phase == 1 ){
        Serial.println("stage 2 phase 1");
        // pasek led daje lekkie ambientowe swiatlo
        setAllLeds(CRGB::Blue);
        FastLED.setBrightness(60);
      }
      else if( phase == 2 ){
        Serial.println("stage 2 phase 2");
        //Po 35 sekundach  załącza się wibracja ( pulsacyjne) i 
        //zaczyna się  miganie białego światła ( do ciemności ) i 
        //to przyspiesza, przez 19 sekund.

        allPulsating();
        if( pulsing_speed < 860) pulsing_speed++;
        if( compute_pause > min_pause ) compute_pause -= 1;
      }
      else if( phase == 3 ){
        Serial.println("stage 2 phase 3");
        //Potem światło całkowicie gaśnie i tylko wibracje zostają
        // ( tu najmocniejsze wibracje musza być )  

        setAllLeds(CRGB::Black);
      }
      else if( phase == 4 ){
        Serial.println("stage 2 phase 4");
        compute_pause = 500;
        //od 95 sekundy trwania tego trybu zaczyna się miganie na zmianę
        //z przerwami leda podzielonego na 3 czesci. Miga to białym światłem.
        //+ strobo uruchamia się 3 razy po 5 z przerwami 5 sekundowymi.
        sixPartsRandom(CRGB::Red);

      }
      
      
    }

    void start(){
      is_on = true;
    }

    void stop(){
      is_on = false;
    }

    bool is_on_getter(){
      return is_on;
    }

    void stage3Setup(){
      return;
    }

    void stage3(){
      return;
    }

    void stage4Setup(){
      digitalWrite(STROBEPIN, HIGH);
      digitalWrite(VIBRATIONSPIN, HIGH);
      digitalWrite(MAINLIGHTPIN, HIGH);
      color = CRGB::Red;
      pulsing_speed = 5;
      compute_pause = 50;
    }

    void stage4(){
      allPulsating();
    }

    void compute(int stage, int phase) {
      switch( stage )
        {
            case 1:
                stage1();
                Serial.println("stage 1");
                break;
            
            case 2:
                stage2(phase);
                break;
            
            case 3:
                stage3();
                Serial.println("stage 3");
                break;

            case 4:
                stage4();
                Serial.println("stage 4");
                break;
        }
    }
};


class TempObject {
  private:
    unsigned long last_computed;
    unsigned int on_time;
    unsigned int off_time;
    int counter;
    bool is_on;
    int PIN;

  public:
    TempObject(int pin, unsigned int _on_time, unsigned int _off_time) {
      PIN = pin;
      counter = 0;
      on_time = _on_time;
      off_time = _off_time;
      last_computed = 0;
      is_on = false;
    }

    void set_on_off_time(unsigned int on, unsigned int off){
      on_time = on;
      off_time = off;
    }

    int get_counter(){
      return counter;
    }

    void loop() {
      unsigned long int mil = millis();
      if (is_on == false && mil - last_computed >= off_time){
        digitalWrite(PIN, LOW);
        is_on = true;
        counter++;
        last_computed = millis();
      }
      else if (is_on == true && mil - last_computed >= on_time){
        digitalWrite(PIN, HIGH);
        is_on = false;
        last_computed = millis();  
      }
    }

};


LedStripe<LED_PIN1> longLedStripe(30, 273, CRGB::Blue, CRGB::Black); // Declare ledStripe as an object of class LedStripe
TempObject vibrationsEngine(VIBRATIONSPIN, 500, 1000);
TempObject strobe(STROBEPIN, 5000, 5000);

void stage1Setup(){
  digitalWrite(MAINLIGHTPIN, LOW);
  
  longLedStripe.stage1Setup();
  stage1_on = true;
}

void stage1(){
  longLedStripe.loop(1, 0);
}

void stage2Setup(){
  longLedStripe.stage2Setup();
  currentStageStart = millis();
}

void stage2(){
  unsigned long time = millis();
  //tutaj zmienic na < 35000
  if( time - currentStageStart < 35000 ){
    longLedStripe.loop(2, 1);
  }
  //tutaj zmienic na < 35000 + 19000
  else if( time - currentStageStart < 35000 + 19600){
    longLedStripe.loop(2, 2);
    vibrationsEngine.loop();
  } 
  // tutaj zmienic na < 99000
  else if( time - currentStageStart < 100000 ){
    longLedStripe.loop(2, 3);
    vibrationsEngine.set_on_off_time(10000, 900);
    vibrationsEngine.loop();
    if( time - currentStageStart >= 86000){
      digitalWrite(VIBRATIONSPIN, HIGH);
    }
  }
  else{
    digitalWrite(VIBRATIONSPIN, HIGH);
    if(strobe.get_counter() < 3){
      strobe.loop();
    }
    else{
      digitalWrite(STROBEPIN, HIGH);
    }
    longLedStripe.loop(2, 4);
  }
}

void stage3Setup(){
  longLedStripe.stage3Setup();
  currentStageStart = millis();
}

void stage3(){
  longLedStripe.loop(3, 0);
}

void stage4Setup(){
  longLedStripe.stage4Setup();
  currentStageStart = millis();
}

void stage4(){
  longLedStripe.loop(4, 0);
}

unsigned long int buttons_last_millis = 0;

void buttons(){
  unsigned long int mil = millis();
  if( mil - buttons_last_millis > 200 ){
    if( digitalRead(IN_PIN1) ){
      if( stage2_on == false ){
        stage2Setup();
        stage2_on = true;

        stage1_on = false;
        stage3_on = false;
        stage4_on = false;
      
      }
      Serial.println("wcisnieto 1");
    }
    else if( digitalRead(IN_PIN2) ){
      if( stage3_on == false ){
        stage3Setup();
        stage3_on = true;

        stage1_on = false;
        stage2_on = false;
        stage4_on = false;
      
      }
      Serial.println("wcisnieto 2");
    }
    else if( digitalRead(IN_PIN3) ){
      if( stage4_on == false ){
        stage4Setup();
        stage4_on = true;

        stage1_on = false;
        stage2_on = false;
        stage3_on = false;
        
      
      }
      Serial.println("wcisnieto 3");
    }
    buttons_last_millis = mil;
  }
}

unsigned long int lastRandomLeds = 0;

void randomLeds(int minSpace){
  if (millis() - lastRandomLeds < minSpace)
    return;

  for (int i = 0; i < LED_42_NUMBER; i++) {
    leds_42[i] = CRGB::Black;
  }
  
  for (int i = 0; i < LED_42_NUMBER; i++) {
    if (random8(0, 100) < 35)
      leds_42[i] = CRGB(random8(0, 200), random8(0, 200), random8(0, 200));
  }
  lastRandomLeds = millis();
}


void setup() {
  pinMode(IN_PIN2, INPUT);
  pinMode(IN_PIN3, INPUT);
  pinMode(IN_PIN4, INPUT);
  pinMode(MAINLIGHTPIN, OUTPUT);
  pinMode(VIBRATIONSPIN, OUTPUT);
  pinMode(STROBEPIN, OUTPUT);
  pinMode(PIN_ELO2, OUTPUT);
  Serial.begin(9600);

  FastLED.addLeds<WS2812, LED_PIN_42, GRB>(leds_42, LED_42_NUMBER);
  digitalWrite(VIBRATIONSPIN, HIGH);
  digitalWrite(STROBEPIN, HIGH);

  stage1Setup();
}

void loop() {
  //longLedStripe.setAllLeds(CRGB::Green);
  //shortLedStripe.setAllLeds(CRGB::Blue);
  buttons();
  randomLeds(5000);
  if( stage1_on ) stage1();
  if( stage2_on ) stage2();
  if( stage3_on ) stage3();
  if( stage4_on ) stage4();
  
  FastLED.show();
}