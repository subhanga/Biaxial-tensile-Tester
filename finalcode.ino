#include "HX711.h"

#define DOUT  4
#define CLK  3

HX711 scale;

float calibration_factor = -51000; //roughly -51000 works for 10kg load cell at 0.5 to 0.7kg

long pos = 0;
long prevPos = 1;
long steps = 0;
long prevSteps = 0;
float conNum = 0.000286*3.045*8;
bool dir = 0;
int Speed = 0;
bool homeFlag;
unsigned long prevTimer = 0;
unsigned long lastStepTime = 0;
int trigDelay = 500;

      void updatePosition(void){
        if(dir ==1){
           pos=pos+steps;
          steps=0;
          }else{
            pos=pos-steps;
            steps=0;
            }
            }

            
    float convertToInches(long pos){
      return conNum*pos;
      }

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
  
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);

  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), countSteps, RISING);

  Serial.begin(9600);
}

void loop() {
  if(homeFlag == 0){homeActuator();}

  //Serial.println(digitalRead(8));
  //Serial.println(digitalRead(9));

  if(digitalRead(8) == HIGH & digitalRead(9) == LOW){
    //Retract actuator
    dir = 0;
    Speed = 255;
    analogWrite(10,0);
    analogWrite(11, Speed);
    if(millis() - prevTimer > 100){
        updatePosition();
        prevTimer = millis();
        if(pos == prevPos | pos == 0){ pos = 0;}
        else {prevPos = pos;}
        Serial.print("Displacement: ");
        Serial.print(convertToInches(pos));
        Serial.print(" inches         ");

        //Only read force immediately after displacement
        Serial.print("Force: ");
        Serial.print(scale.get_units()*0.453592*9.81, 2); //*9.81 = kg to Newton conversion
        Serial.println(" Newtons");
      }
  }
  else if(digitalRead(8) == LOW & digitalRead(9) == HIGH){
    //Extend Actuator
    dir = 1; 
    Speed = 255;
    analogWrite(10, Speed);
    analogWrite(11, 0);
    if(millis() - prevTimer > 100){
        updatePosition();
        prevTimer = millis();
        if(pos == prevPos | pos == 574){pos = 574;}
        else {prevPos = pos;}
        Serial.print("Displacement: ");
        Serial.print(convertToInches(pos));
        Serial.print(" inches         ");

        //Only read force immediately after displacement
        Serial.print("Force: ");
        Serial.print(scale.get_units()*0.453592*9.81, 2);  //*9.81 = kg to Newton conversion
        Serial.println(" Newtons");
    }
  }
  else{
    Speed = 0;
    analogWrite(10, 0);
    analogWrite(11, 0);
  }

    scale.set_scale(calibration_factor); //Adjust to this calibration factor
  

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 100;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 100;
  }
}

void countSteps(void){
  if(micros()-lastStepTime >trigDelay){
    steps++;
    lastStepTime =micros();
    }
  }

  void homeActuator(void){
    prevTimer = millis();
    while(homeFlag == 0){
      Speed =255;
      analogWrite(10,0);
      analogWrite(11,Speed);
      if(prevSteps ==steps){
        if(millis() -prevTimer >100){
          analogWrite(10,0);
      analogWrite(11,0);
      steps=0;
      Speed=0;
      homeFlag=1;
      }
        }else{
          prevSteps=steps;
          prevTimer=millis();
        }
      }
      }
