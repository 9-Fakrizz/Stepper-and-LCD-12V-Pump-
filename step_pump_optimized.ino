#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define WATER_SENSOR 3
#define STEPPER_PIN_1 11
#define STEPPER_PIN_2 10
#define STEPPER_PIN_3 9
#define STEPPER_PIN_4 8
int step_number = 0;
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int buttonCount = 5;
int button_pin[6] = {0, 4, 5, 6, 7, 12};
int buttonState[6];
bool laststate5;

int analogInput = A0;
float vout = 0.0;
float vin = 0.0;
float R1 = 30000.0; // 30k
float R2 = 7500.0;  // 7500 ohm resistor
int value = 0;

int sel_arrow = 0;
int manual_mmhg = 50;
int mmhg_sel;
String mode_sel_name[5] = {"0", "Manual", "small child", "older child", "adult"};

////////////////////////////////////////////////////////////////
////////////////      Configurations   ////////////////////////
//////////////////////////////////////////////////////////////

String mmhg_sel_name[5] = {"0", "50-200", "70", "100", "115"};
int step_setup_arr[4] = {1900, 2360, 2360, 2360};  // fix for manual , smch, oldch, adult
int step_mmhg_each_5[31] = 
                          {0, 20 , 90 , 100 , 180 , 200 , 220 , 240 , 270 , 290 ,

                            300 , 350 , 370 , 380 , 390 , 400 , 410 , 430 , 460 , 480 ,
                            
                            490 , 500 , 510 , 520 , 540 , 550 , 560 , 570 , 590 , 600, 610, };

int motor_active;
int menu;
float batt_map;
int start;

int water;

// New Update
int i=0;
int count_step = 0;
int get_step = 20;
bool dir = true;
bool task = false;


// Function Declarations
void readButtons();
void printButtons();
void readBattery();
void checkWaterSensor();
void clearArrow();
void displayData();
void selectMenu();
void setZeroStepper();
void oneStep(bool dir);

//////////////////////////////////////////////////////////////////////////
///////////////////////      Main Setup     //////////////////////////////
//////////////////////////////////////////////////////////////////////////

void setup() {
  lcd.init();
  Serial.begin(115200);
  lcd.backlight();

  pinMode(STEPPER_PIN_1, OUTPUT);
  pinMode(STEPPER_PIN_2, OUTPUT);
  pinMode(STEPPER_PIN_3, OUTPUT);
  pinMode(STEPPER_PIN_4, OUTPUT);

  for (int i = 1; i <= 5; i++) {
    pinMode(button_pin[i], INPUT_PULLUP);
  }

  // Relay for 12v pump
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  // Water
  pinMode(A1, INPUT);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Setting to Zero ...");
  setZeroStepper();
  lcd.clear();
  lcd.print("Set-Zero Complete!");
 
}
////////////////////////////////////////////////////////////////////////////
/////////////////          Main Loop              //////////////////////////
////////////////////////////////////////////////////////////////////////////

void loop() {
  readButtons();
  readBattery();
  checkWaterSensor();
  displayData();
  selectMenu();
  water = analogRead(A1);
  Serial.println("water :" + String(water));
  Serial.println("Fix Step : " + String(step_setup_arr[sel_arrow]));
  Serial.print("Manual Step : " + String(step_mmhg_each_5[i]));
  Serial.println("  Index : " + String(i));
  if (task && analogRead(A1) < 5) {
    Serial.println("Starting...");
    lcd.setCursor(0, 0);
    lcd.print("Start.....");
    setZeroStepper();

    // Fix Stepper
    Serial.println("Fix step .....");
    for (int i = 0; i <= step_setup_arr[sel_arrow]; i++) {
      oneStep(!dir);
      delayMicroseconds(3000);
    }
    //Serial.println("Fix Stepper Complete !");
    //lcd.clear();
    //lcd.print("Fix Stepper Complete !");
    //delay(500);

    // Manual
    Serial.println("Manual Step .....");
    if (sel_arrow == 0) {
      for (int j = 0; j < step_mmhg_each_5[i]; j++) {
        oneStep(!dir);
        delayMicroseconds(3000);
      }
     //Serial.println("Manual Complete!");
     //lcd.clear();
     //lcd.print("Manual Complete!");
     //delay(500);
    }

    lcd.clear();
    lcd.print("Started 12V Pump !");
    Serial.println("Start 12v Pump .....");
    
    digitalWrite(2, LOW);
    while (digitalRead(button_pin[4]) != LOW && menu != 0) {
       checkWaterSensor();
    }
    Serial.println("Out of While loop() ");
    digitalWrite(2, HIGH);
    
  

     //Set Zero
    setZeroStepper();
    Serial.println("Set-Zero Complete!");
    lcd.clear();
    lcd.print("Set-Zero Complete!");
    delay(500);

    task = false;
    lcd.clear();
  }
  if(analogRead(A1) > 8){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Water is FULL !");
    delay(1000);
    lcd.clear();
  }

  delay(10);
}


//////////////////////////////////////////////////////////////////////////
////////////////     All Functions are below here  ///////////////////////
/////////////////////////////////////////////////////////////////////////


// Function Definitions

//////////////////////////////////////////////////////////////
void readButtons() {
  for (int i = 1; i <= buttonCount; i++) {
    buttonState[i] = digitalRead(button_pin[i]);
  }
}
/////////////////////////////////////////////////////////////

void printButtons() {
  for (int i = 1; i <= buttonCount; i++) {
    Serial.print(buttonState[i]);
    Serial.print(" , ");
  }
  Serial.println("");
}

////////////////////////////////////////////////////////////////

void readBattery() {
  value = analogRead(analogInput);
  vout = (value * 4.796) / 1023.0;
  vin = vout / (R2 / (R1 + R2));
  batt_map = map(vin, 0, 5, 0, 100);

  if (batt_map > 100) {
    batt_map = 100;
  } else if (batt_map <= 0) {
    batt_map = 0;
  }
}

////////////////////////////////////////////////////////////////

void checkWaterSensor() {
  Serial.println("Water : "+String(analogRead(A1)));
  if(analogRead(A1) > 14){
    Serial.println("Water : "+String(analogRead(A1)));
    digitalWrite(2, HIGH); //HIGH is Turn off 
    menu = 0; 
    Serial.println("Waterrrrrrrrrrrrrrrrrrrrrrrrrrr");
  }
}
/////////////////////////////////////////////////////////////////////

void clearArrow() {
  for (int i = 0; i <= 3; i++) {
    lcd.setCursor(0, i);
    lcd.print(" ");
  }
}

////////////////////////////////////////////////////////////////////
void displayData() {
  lcd.setCursor(14, 0);

  if (batt_map >= 10 && batt_map <= 99) {
    lcd.print("DC" + String(batt_map, 0) + "% ");
  } else if (batt_map < 10) {
    lcd.print("DC" + String(batt_map, 0) + "%  ");
  } else {
    lcd.print("DC" + String(batt_map, 0) + "%");
  }

  if (menu == 0) {
    lcd.setCursor(1, 0);
    lcd.print("1:" + mode_sel_name[1] + "");
    lcd.setCursor(1, 1);
    lcd.print("2:" + mode_sel_name[2] + "");
    lcd.setCursor(1, 2);
    lcd.print("3:" + mode_sel_name[3] + "");
    lcd.setCursor(1, 3);
    lcd.print("4:" + mode_sel_name[4] + "");
  } else if (menu == 1) {
    lcd.setCursor(0, 0);
    lcd.print("" + mode_sel_name[sel_arrow + 1] + "");
    lcd.setCursor(0, 1);
    lcd.print("mmhg " + mmhg_sel_name[sel_arrow + 1]);

    if (sel_arrow == 0) {
      lcd.setCursor(0, 2);
      lcd.print(" [" + String(manual_mmhg) + " mmhg" + "]");
//      lcd.setCursor(0, 3);
//      lcd.print("Step : " + String(step_mmhg_each_5[i]));

    } else {
//      lcd.setCursor(0, 3);
//      lcd.print("Step : " + String(step_setup_arr[sel_arrow]));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void selectMenu() {
  delay(60);
  Serial.print("sel_arrow : ");
  Serial.println(sel_arrow);

  if (menu == 0) {
    sel_arrow = sel_arrow % 4;
    lcd.setCursor(0, sel_arrow);
    lcd.print(">");

    if (buttonState[1] == LOW) {
      clearArrow();
      sel_arrow += 1;
    }

    if (buttonState[2] == LOW) {
      if (sel_arrow == 0) sel_arrow = 4;
      clearArrow();
      sel_arrow -= 1;
    }

    if (buttonState[3] == LOW) {
      clearArrow();
      lcd.clear();
      menu = 1;
    }
  }

  if (menu == 1) {
    Serial.print("menu = 1 !");

    for (int i = 1; i <= buttonCount; i++) {
      buttonState[i] = digitalRead(button_pin[i]);
    }

    if (sel_arrow == 0) {
      if (buttonState[1] == LOW && manual_mmhg < 200) { // 200 mmhg adjust
        manual_mmhg += 5;
        i++;
      }
      if (buttonState[2] == LOW && manual_mmhg > 50) {
        manual_mmhg -= 5;
        i--;
      }
    }

    if (buttonState[3] == LOW) {
      Serial.print("confirm !!");
      lcd.clear();
      task = true;
    }

    if (buttonState[4] == LOW) {
      menu = 0;
      setZeroStepper();
      Serial.println("set Zero complete!");
      sel_arrow = 0;
      lcd.clear();
    }
  }
}


//////////////////////////////////// set zero ////////////////////////

void setZeroStepper() {
  buttonState[5] = digitalRead(button_pin[5]);

  if (buttonState[5] != LOW) {
    Serial.println("Start Setting to ZERO ...");
    while (buttonState[5] != LOW) {
      buttonState[5] = digitalRead(button_pin[5]);
      oneStep(true);
      delayMicroseconds(3000);
    }
    Serial.println("Set Zero Complete!");
  }
}

/////////////////////////////////////////////////////////////////////////////

void oneStep(bool dir) {
  if (!dir) {
    switch (step_number) {
      case 0:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 1:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 2:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 3:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
    }
  } else {
    switch (step_number) {
      case 0:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
      case 1:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 2:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 3:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
    }
  }

  step_number++;

  if (step_number > 3) {
    step_number = 0;
  }
}
/////////////////////////////////////////////////////////////
