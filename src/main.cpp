#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <FastLED.h>
#include <Adafruit_NeoPixel.h>
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Fonts/FreeMonoBoldOblique9pt7b.h>
#include <TimerEvent.h>

// start button = GPIO_NUM_10

#define START_BUTTON GPIO_NUM_10

//SSD1306 settings

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define I2C_ADDRESS 0x3C
#define SDA_PIN GPIO_NUM_5 //21
#define SCL_PIN GPIO_NUM_6 //22

#define OLED_RESET     -1

//Fastled settings
// How many leds are in the strip?
#define NUM_LEDS 12
#define DATA_PIN GPIO_NUM_8

// This is an array of leds.  One item for each led in your strip.
//Fastled
//CRGB leds[NUM_LEDS];
//neopixel
Adafruit_NeoPixel strip(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);
#define HUESTEP 1024
#define MAXBRIGHTNESS 70


TimerEvent Ledupdate;
TimerEvent StartGame;

//Paddle GPIO GPIO_NUM_2
#define PADDLE1_PIN GPIO_NUM_2
#define PADDLE2_PIN GPIO_NUM_3


// Paddle dimensions
#define PADDLE_WIDTH 2
#define PADDLE_HEIGHT 13
#define BALL_SIZE 2

// Ball properties
float ballX = SCREEN_WIDTH / 2;
float ballY = SCREEN_HEIGHT / 2;
float ballVelocityX = 1.0; // Adjust as needed
float ballVelocityY = 0.5; // Adjust as needed
float hoek;
float snelheid; 

// Player scores
int player1Score = 0;
int player2Score = 0;

//
bool start=false;
bool play=false;
bool wait_for_button=false;
bool game_over = false;

const uint16_t scoreColor[] {0,21845,21845,12288,6144,0};

// Initialize display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void intro() {
  int16_t  x1, y1;
  uint16_t w, h;
  display.clearDisplay();
  display.setTextSize(1); 
  display.setFont(&FreeSansBoldOblique12pt7b);
  display.getTextBounds("mini",0,0,&x1,&y1,&w,&h);
  display.setCursor((SCREEN_WIDTH-w)/2-x1,h);//(24-h)/2-y1);
  display.setTextColor(SSD1306_WHITE);
  display.print("mini");
  display.getTextBounds("pong",0,0,&x1,&y1,&w,&h);
  display.setCursor((SCREEN_WIDTH-w)/2-x1,h*2);//(40-h)/2-y1);
  display.print("pong");
  display.setFont();
  display.getTextBounds("Press X to start",0,0,&x1,&y1,&w,&h);
  display.setCursor((SCREEN_WIDTH-w)/2,SCREEN_HEIGHT-h);
  display.print("Press X to start");
  display.display();

  
}

void ledanimations() {

static uint16_t step=0;

  if (!play) {

    for (int i = 0; i< 6; i++) {
 /* fastled 
     leds[i]=CHSV(step+i*43,255,255);
      leds[i+1]=CHSV(step+i*43+43,255,255);
      leds[NUM_LEDS-i-1]=CHSV(step+i*43,255,255);
      leds[NUM_LEDS-i-2]=CHSV(step+i*43+43,255,255);
    }
    step+=43;
    FastLED.show();
*/

//neopixel
      //strip.setPixelColor(i, strip.ColorHSV(step));  
      strip.setPixelColor(i, strip.ColorHSV(step+i*HUESTEP));  
      strip.setPixelColor(NUM_LEDS-i-1, strip.ColorHSV(step+i*HUESTEP,255,255));  
      strip.show();   
      step+=HUESTEP>>2;              
    }

  } else {
    //PlayerA
    static int8_t brightnessPlayerA; 
    static int8_t brightnessPlayerB;

    if(player1Score<4) brightnessPlayerA=MAXBRIGHTNESS;
    else { 
      static int brightnessStepA=5;
      if (brightnessPlayerA<20) brightnessStepA= 5;
      if (brightnessPlayerA>MAXBRIGHTNESS)  brightnessStepA= -5;

    }
    for (int i=0;i<player1Score;i++) {
    //leds[i]=CHSV(90, 255, 255);

    strip.setPixelColor(i,strip.ColorHSV(scoreColor[player1Score],255,brightnessPlayerA));

    }
  if(player2Score<4) brightnessPlayerB=MAXBRIGHTNESS;
    else { 
      static int brightnessStepB=5;
      if (brightnessPlayerB<20) brightnessStepB= 5;
      if (brightnessPlayerB>MAXBRIGHTNESS)  brightnessStepB= -5;

    }
    for (int i=0;i<player2Score;i++) {
    //leds[i]=CHSV(90, 255, 255);

    strip.setPixelColor(i+6,strip.ColorHSV(scoreColor[player2Score],255,brightnessPlayerB));

    }
    if (ballVelocityX<0) {
      strip.setPixelColor(5,strip.Color(0,0,255));
      strip.setPixelColor(11,strip.Color(0,0,0));
    } else {
      strip.setPixelColor(11,strip.Color(0,0,255));
      strip.setPixelColor(5,strip.Color(0,0,0));
      }
       

    strip.show();

  }
}


void cart_snelheid(float angle, float speed) {
  ballVelocityX = cos(angle)*speed;
  ballVelocityY= sin(angle)*speed;
}




void ballinit() {

  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  snelheid=1;
  
    if (millis()%1) {
    hoek=PI/180*10;
    } else {
     hoek=PI/180*170;
    }
    
  
}

void gameinit() {
  //FastLED.clear();
  //FastLED.show();
  strip.clear();
  strip.show();

  intro();
  player1Score = 0;
  player2Score = 0;
  ballinit();
  //play=true;
  wait_for_button=true;
}



void showScore(int playerA, int playerB) {
  for (int i=0;i<playerA;i++) {
    //leds[i]=CHSV(90, 255, 255);

    strip.setPixelColor(i,strip.ColorHSV(scoreColor[playerA],255,255));

  }
  for (int i=0;i<playerB;i++) {
    //leds[i+6]=CHSV(90, 255, 255);
    strip.setPixelColor(i+6,strip.ColorHSV(scoreColor[playerB],255,255));
  }
  //FastLED.show();
  strip.show();
  
}



void gameover() {
  int16_t  x1, y1;
  uint16_t w, h;



  //display.setCursor(16,8);
  display.setTextSize(1); 
  display.setFont(&FreeSansBoldOblique12pt7b);
  display.getTextBounds("Game",0,0,&x1,&y1,&w,&h);
  display.setCursor((SCREEN_WIDTH-w)/2-x1,h);//(24-h)/2-y1);
  display.setTextColor(SSD1306_WHITE);
  display.print("Game");
  display.getTextBounds("Over",0,0,&x1,&y1,&w,&h);
  display.setCursor((SCREEN_WIDTH-w)/2-x1,h*2);//(40-h)/2-y1);
  display.print("over");
  display.setFont();//&FreeMonoBoldOblique9pt7b);

    
        
  if (player1Score==5) {
    display.getTextBounds("Player 1 Won",0,0,&x1,&y1,&w,&h);
    display.setCursor((SCREEN_WIDTH-w)/2,SCREEN_HEIGHT-2*h);
    display.println("Player 1 Won");
    
  } else {
    display.getTextBounds("Player 2 Won",0,0,&x1,&y1,&w,&h);
    display.setCursor((SCREEN_WIDTH-w)/2,SCREEN_HEIGHT-2*h);
    display.println("Player 2 Won");
    
  }

  display.getTextBounds("Press X to start",0,0,&x1,&y1,&w,&h);
  display.setCursor((SCREEN_WIDTH-w)/2,SCREEN_HEIGHT-h);
  display.print("Press X to start");
 
  display.display();
  
}

void increase_velocity() {
  //ballVelocityX = ballVelocityX * 1.001;
  //ballVelocityY = ballVelocityY * 1.001;
  snelheid = snelheid*1.001;
}

void pongstep() {
  // Read potentiometer values for player input
  int player1Input = 4096-analogRead(PADDLE1_PIN);
  int player2Input = 4096-analogRead(PADDLE2_PIN);

  // Update paddle positions based on potentiometer values
  int player1PaddleY = map(player1Input, 200, 3900, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);
  int player2PaddleY = map(player2Input, 200, 3900, 0, SCREEN_HEIGHT - PADDLE_HEIGHT);

  // Update ball position
   cart_snelheid(hoek,snelheid);

  ballX += ballVelocityX;
  ballY += ballVelocityY;

  //check for y out of bound
  if (ballY < 2 || ballY > (SCREEN_HEIGHT-2)) {
    hoek=-hoek;
  }

  // Check for collisions with paddles
  if (ballX <= PADDLE_WIDTH && ballY >= player1PaddleY-BALL_SIZE && ballY <= player1PaddleY + PADDLE_HEIGHT+BALL_SIZE) {
    // Player 1 hit the ball
        //change the angle, i.e. return the ball
    hoek=PI-hoek;
    hoek=hoek+(ballY-player1PaddleY-PADDLE_HEIGHT/2)/180*PI;
   
  } else if (ballX >= SCREEN_WIDTH - PADDLE_WIDTH && ballY >= player2PaddleY-BALL_SIZE && ballY <= player2PaddleY + PADDLE_HEIGHT+BALL_SIZE) {
   // Player 2 hit the ball
    hoek=PI-hoek;
    // if lower half of paddle, angle + som degrees
    // if upperhalf of paddle - some degrees
    hoek=hoek-(ballY-player2PaddleY-PADDLE_HEIGHT/2)/180*PI;
  
  } else  if (ballX < 0 || ballX > SCREEN_WIDTH) { // Check for ball going out of bounds
    if (ballX<0) {
      player1Score++;
      
    } else {
      player2Score++;
    }
    showScore(player1Score, player2Score);
    if (player1Score==5 || player2Score==5) {
      game_over=true;
  
    } else  ballinit();
  }
  display.clearDisplay();
  display.fillRect(0, player1PaddleY, PADDLE_WIDTH, PADDLE_HEIGHT, SSD1306_WHITE);
  display.fillRect(SCREEN_WIDTH - PADDLE_WIDTH, player2PaddleY, PADDLE_WIDTH, PADDLE_HEIGHT, SSD1306_WHITE);
  display.drawCircle(ballX, ballY, BALL_SIZE, SSD1306_WHITE);
  display.display();
  increase_velocity();
  
}

void loop() {

if (!wait_for_button) { 

if (!start) {
  start=true;
  gameinit();
  wait_for_button=true;

} else {
  if (!play) {
    play=true;
    strip.clear();
    strip.show();
    //FastLED.clear();
    //FastLED.show();
  }
  
}

if (play) pongstep();

 
if (game_over) {
  start=false;
  play=false;
  wait_for_button=true;
  delay(1000);
  gameover();
  delay(2000);
  game_over=false;
}   

} else if (!digitalRead(START_BUTTON)) {
  wait_for_button= false;
  while (!digitalRead(START_BUTTON));
}
  

Ledupdate.update();
  
}

void setup() {
  Serial.begin(115200);
  pinMode(START_BUTTON,INPUT_PULLUP);
  randomSeed(random(314159));
  display.begin(SSD1306_SWITCHCAPVCC,I2C_ADDRESS); //SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS
  display.clearDisplay();
  display.display();
  //delay(100); // Wait for display to initialize
  //display.clearDisplay();

  //Fastled
  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  //FastLED.setBrightness(30);

  //Neopixel
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(MAXBRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)
  //gameinit();
  Ledupdate.set(40,ledanimations);

  }




