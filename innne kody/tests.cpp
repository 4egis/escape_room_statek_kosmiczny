#include <FastLED.h>

#define LED_PIN1     7
#define LED_PIN2     8

#define IN_PIN1      2
#define IN_PIN2      3
#define IN_PIN3      4
#define IN_PIN4      5

#define MAINLIGHTPIN   9
#define VIBRATIONSPIN 11

bool stage1_on = false;
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
      bar_length = 10; // Initialize bar_length to a desired value
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
    }

    ~LedStripe() {
      delete[] leds; // Free memory when the object is destroyed
    }

    void setAllLeds(CRGB color) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
      }
      FastLED.show();
    }

    void setLeds(int start, int end, CRGB color) {
      for (int i = start; i < end; i++) {
        leds[i] = color;
      }
    }

    void floatingBar(){
      if(bar_returning == false){
            if (bar_index < NUM_LEDS - bar_length){
                leds[bar_index] = background_color;
                leds[bar_index + bar_length] = color;
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
                bar_index--;
            }
            else{
                bar_returning = false;
            }
        }
    }

    void allPulsating() {
      Serial.println(index);
      if(returning == false){
        if (index < 255){
          setAllLeds(color);
          FastLED.setBrightness(index);
          index += pulsing_speed;
          if( index > 255 ) index = 255;
        }
        else{
            returning = true;
        }   
      }
      else{
        if (index > 0){
          setAllLeds(color);
          FastLED.setBrightness(index);
          index -= pulsing_speed;
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
    }

    void stage1() {
        //floating bar:
        floatingBar();
    }

    void stage2Setup() {
      index = 0;
      color = CRGB::WhiteSmoke;
      returning = false;
      compute_pause = 100;
    }

    void stage2(int phase) {
      if( phase == 1 ){
        // pasek led daje lekkie ambientowe swiatlo
        setAllLeds(CRGB::Aqua);
        FastLED.setBrightness(80);
      }
      else if( phase == 2 ){
        //Po 35 sekundach  załącza się wibracja ( pulsacyjne) i 
        //zaczyna się  miganie białego światła ( do ciemności ) i 
        //to przyspiesza, przez 19 sekund.

        allPulsating();
        if( pulsing_speed < 10) pulsing_speed++;

        //dodac wibracje
      }
      else if( phase == 3 ){
        //Potem światło całkowicie gaśnie i tylko wibracje zostają
        // ( tu najmocniejsze wibracje musza być )  

        setAllLeds(CRGB::Black);
        Serial.println("jestem w phase 3");
      }
      else if( phase == 4 ){
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

    void stage3(){
      return;
    }

    void stage4(){
      return;
    }

    void compute(int stage, int phase) {
      switch( stage )
        {
            case 1:
                stage1();
                break;
            
            case 2:
                stage2(phase);
                break;
            
            case 3:
                stage3();
                break;

            case 4:
                stage4();
                break;
        }
    }
};

/*
class Buttons
{
  private:
  bool was_pressed;
  int pin;
  unsigned int pause;
  unsigned long int last_millis;

  public:
  Button(int _pin1, int _pin2, int _pin3, int _pin4){
    was_pressed = false;
    pause = 50;
    pin = _pin1;
  }

  bool get_was_pressed(){
    return was_pressed;
  }

  bool read_state(){
    if( millis() - last_millis < pause )
      return false;

    bool state = digitalRead(pin);
    if( state && !was_pressed) was_pressed = true;
    return state;
  }


};
*/


LedStripe<LED_PIN1> longLedStripe(50, 270, CRGB::Blue, CRGB::Black); // Declare ledStripe as an object of class LedStripe
LedStripe<LED_PIN2> shortLedStripe(50, 113, CRGB::Blue, CRGB::Black);

void setup() {
  pinMode(MAINLIGHTPIN, INPUT);
  pinMode(IN_PIN2, INPUT);
  pinMode(IN_PIN3, INPUT);
  pinMode(IN_PIN4, INPUT);
  
  Serial.begin(9600);
}

void stage1Setup(){
  digitalWrite(MAINLIGHTPIN, LOW);
  longLedStripe.start();
  longLedStripe.stage1Setup();
  stage1_on = true;
}

void stage1(){
  longLedStripe.loop(1, 0);
}

void stage2Setup(){
  longLedStripe.start();
  longLedStripe.stage2Setup();
  stage2_on = true;
  currentStageStart = millis();
}

void stage2(){
  unsigned long time = millis();
  //tutaj zmienic na < 35000
  if( time - currentStageStart < 5000 ){
    longLedStripe.loop(2, 1);
  }
  //tutaj zmienic na < 35000 + 19000
  else if( time - currentStageStart < 5000 + 5000){
    longLedStripe.loop(2, 2);
  } 
  // tutaj zmienic na < 95000
  else if( time - currentStageStart < 25000 ){
    Serial.println("zaczeto phase 3");
    longLedStripe.loop(2, 3);
  }
  else{
    Serial.println("zaczeto phase 4");
    longLedStripe.loop(2, 4);
  }
}

void loop() {

  longLedStripe.setAllLeds(CRGB::Green);
  shortLedStripe.setAllLeds(CRGB::Blue);

/*
  if( !stage2_on ) {
    stage2Setup();
  }




  if( stage1_on ) stage1();
  if( stage2_on ) stage2();
  */
 /*if( stage3_on ) stage3();
  if( stage4_on ) stage4();
  */
  
  FastLED.show();
}