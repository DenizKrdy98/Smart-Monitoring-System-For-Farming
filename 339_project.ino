//SMART MONITORING SYSTEM FOR FARMING
//This is the code for 339 Project written by Kaan Toyaksi and Deniz Karadayı.
//First objective: Temp sensor.
//Second objective: Photoresistor.
//Third objective: Humidity sensor.
//Fourth objective: Bluetooth module.

// We define the pins that we are going to use in this section

#define tempPin A0
#define humPin A1
#define lightPin A5

const int buzzer = 11; 
const int redPin = 10;
const int yellowPin = 9;
const int whitePin = 8;
const int R = 6;
const int G = 4;
const int B = 5;

//////////////////////////////////////////
// Constants going to be use in the main code.

String waterPref;               // water preference type
String seasonType;              // season type
String winter = "winter";       // season type I
String summer = "summer";       // season type II
String normal = "normal";       // season type III
String waterLover = "loves";    // water preference type I
String waterHater = "hates";    // water preference type II

int checkTemp, checkLight, checkHum;    // checkers for optimal conditions

int night = 0;              // night/day(1/0) indicator
int pressed = 0;            // button press indicator
int updated = night;        // checks if the pressed button value is new.

/////////////////////////////////////////
// Temperature-related variables and constants

double temp;                // temperature value in Celcius
int i;   
boolean cold ;              // interval for cold temperature  
boolean hot ;               // interval for hot temperature 
boolean degree7_13;         // interval from 7 to 13 degrees
boolean degree13_18 ;       // interval from 13 to 18 degrees
boolean degree18_24 ;       // interval from 18 to 24 degrees
boolean degree24_30 ;       // interval from 24 to 30 degrees

/////////////////////////////////////////
// Light-related variables and constants

float Rldr;                 // Resistance value of LDR
float Vr_cons;              // Voltage of constant R value connected to LDR
float Rcons = 1000;         // Resistance of constant R value connected to LDR
float lux;                  // Lux value of light intensity
float I;                    // Current passed through the LDR circuit

//////////////////////////////////////////
// Humidity-related variables and constants

float hum;                  // Humidty value in terms of percentage

//////////////////////////////////////////

// INTERRUPT SERVICE ROUTINE//////////////
// When button is pressed, it changes the polarity of pressed value.
// It is used to indicate a shift demand.

void nightShifter() {
    if(pressed ==1){
        pressed = 0;
    }else{
        pressed = 1;
    }
}

// FUNCTIONS //////////////////////////////

// Alarm type for cold situations, buzzer sounds
void coldAlarm(){
  Serial.println("Cold");
  for (i=0;i<4;i++){
    tone(buzzer, 900); 
    delay(500);        
    noTone(buzzer);     
    delay(500); 
  }
  checkTemp=0;
}

// Alarm type for hot situations, buzzer sounds
void hotAlarm(){
  Serial.println("Hot");
  for (i=0;i<4;i++){
    tone(buzzer, 2000); 
    delay(500);        
    noTone(buzzer);     
    delay(500); 
  }
  checkTemp=0;
}

// Alarm type for non-optimal light situations, LED blinks
void lightAlarm(int pin){
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
    delay(500);
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
}

// Turns the all colors of RGB-LED off
void closeRGB(){
    digitalWrite(R, LOW);
    digitalWrite(B, LOW);
    digitalWrite(G, LOW);
}

// Bits to percentage value for humidity measurement
double cnvToPerc(int bits){
    double percentage;
    percentage = map(bits, 1023, 310, 0, 100);
    if(percentage>100){
        percentage=100;
    }
    return percentage;
}

// Bits to lux value for LDR measurement
float cnvToLux(int bits){
    Vr_cons = bits/204.6;           // bits to voltage value conversion
    I = Vr_cons / Rcons;            // current passes through the LDR circuit
    Rldr = (5.0 - Vr_cons) / I ;    // for 1 k ohm resistor, the resistance calculation of the LDR  
    lux= (pow( Rldr, (1/-0.8616)))/(pow( 10, (5.118/-0.8616)));   // resistance to lux calculation of the LDR
    return lux;
}

//////////////////////////////////////////////////


// In the setup we are determining the pinmodes, define the interrupt and service routine.
// In addition, we start serial communication as well.
void setup() {

  attachInterrupt(digitalPinToInterrupt(2), nightShifter, RISING); // interrupt initilazation
  
  Serial.begin(9600); 
      
  pinMode(buzzer, OUTPUT); 
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  
}

// In the main loop, first we want user to specify the plant so that the program can examine it accordingly.
// After the initializations, the program starts to an infinite monitoring loop, warns the user when it is needed, states the current conditions for each loop.
// The program starts to measurements at day mode as default. When the button is pressed during a loop execution, it updates the mode after finishing the current loop.
void loop() {
    
    Serial.println(" -- SMART FARMING CONTROL SYSTEM PROGRAM -- ");
    
    // Ask the user for input.
    //  - TYPE OF SEASON
    Serial.println("Welcome, please select the season type of the plant.");    
    Serial.println("Winter, summer or normal?: ");
    
    while(Serial.available()==0){}
    seasonType = Serial.readString();    
    seasonType.trim();

    // Handler for invalid input
    while(!seasonType.equalsIgnoreCase(winter) && !seasonType.equalsIgnoreCase(summer) && !seasonType.equalsIgnoreCase(normal)){
      Serial.println("The answer you entered is not valid. Enter one of the followings.");      
      Serial.println("Winter, summer or normal?: ");
      while(Serial.available()==0){}
      seasonType = Serial.readString();       
      seasonType.trim(); 
    }

    //  - WATER PREFERENCES
    Serial.println("What is the water preference of your plant?");    
    Serial.println("Hates or loves?: ");

    while(Serial.available()==0){}
    waterPref = Serial.readString();     
    waterPref.trim();

    // Handler for invalid input
    while(!waterPref.equalsIgnoreCase(waterLover) && !waterPref.equalsIgnoreCase(waterHater)){
      Serial.println("The answer you entered is not valid. Enter one of the followings.");
      Serial.print("Hates or loves?: ");
      while(Serial.available()==0){}
      waterPref = Serial.readString();       
      waterPref.trim(); 
    }
    
    // Anouncements related to the program execution before it gets started
    delay(1000);
    Serial.println("");    
    Serial.println("The plant is " + seasonType + " type, and it " + waterPref + " water.");
    delay(750);
    Serial.println("The optimal conditions has been prepared. ");
    Serial.println("");
    delay(750);
    Serial.println("Default mode is the day mode. Press the button to shift to the night mode.");
    delay(750);
    Serial.println("The program begins to measurements...");
    Serial.println("");
    delay(2000);

    // INFINITE MONITORING LOOP
    while(true){
      
        // Anouncer if a mode update has occur. "updated" holds the value of night variable at previous loop. It anounces if night has been changed.  
        if (updated!=night){
          if(night==1){
            Serial.println("Program has shifted to night mode.");
          }else{
            Serial.println("Program has shifted to day mode.");
          }
        }

    
      // -------------------------------- TEMP BEGINS ---------------------------------
      
      // Temperature Measurement 
      // We do the measurement for a couple time to be ensured that the value 
      // is not derived from the internal noise fluctuation.
      for(i=0;i<20;i++){
        temp = ((analogRead(tempPin)/1024.0) * 5.0 * 100.0);    // formula to obtain Celsius unit
      }
  
      // Prints The Current Temperature Value
      Serial.print("TEMP: \t");
      Serial.print(temp);     
      Serial.println(" Celsius");        
      delay(1000);
  
      // Interval Spesifications According to Temperature
      cold = temp<7;
      hot = temp>=30;
      degree7_13 = (temp>=7 && temp <13);
      degree13_18 = (temp>=13 && temp <18);
      degree18_24 = (temp>=18 && temp <24);
      degree24_30 = (temp>=24 && temp <30);
  
      // Analysis Of The Temperature 
      // We buzz the buzzer according to the temp.
      // If its higher or lower than the optimal range, the buzzer give sound at different frequencies. 
      // Optimal ranges for plant types are defined in the report.   
      Serial.print("Status:\t");
      if (cold){                     // -------------------------------- CASE I
          coldAlarm();        
      }else if(degree7_13){          // -------------------------------- CASE II
          if(night==1 && seasonType.equalsIgnoreCase(winter)){      
            Serial.println("Optimal.");
            checkTemp=1;
          }else{
            coldAlarm();
          }    
      }else if(degree13_18){         // -------------------------------- CASE III
          if(night==1 && seasonType.equalsIgnoreCase(normal)){
            Serial.println("Optimal.");
            checkTemp=1;
          }else if(night==0 && seasonType.equalsIgnoreCase(winter)){
            Serial.println("Optimal.");
            checkTemp=1;
          }else if(night==1 && seasonType.equalsIgnoreCase(winter)){
            hotAlarm();
          }else{
            coldAlarm();
          }    
      }else if(degree18_24){        // -------------------------------- CASE IV
          if(night==1 && seasonType.equalsIgnoreCase(summer)){
            Serial.println("Optimal.");
            checkTemp=1;
          }else if(night==0 && seasonType.equalsIgnoreCase(normal)){
            Serial.println("Optimal.");
            checkTemp=1;
          }else if(night==0 && seasonType.equalsIgnoreCase(summer)){
            coldAlarm();
          }else{
            hotAlarm();
          }
      }else if(degree24_30){        // -------------------------------- CASE V
          if(night==0 && seasonType.equalsIgnoreCase(summer)){
            Serial.println("Optimal.");
            checkTemp=1;
          }else{
            hotAlarm();
          }
      }else if(hot){                // -------------------------------- CASE VI
          hotAlarm();
      }    
      Serial.println("");
      
      //--------------------------------- TEMP ENDS ---------------------------------

  
      //--------------------------------- LIGHT BEGINS ----------------------------------
      
      // Light Measurement 
      // We do the measurement for a couple time to be ensured that the value is not derived from the internal noise fluctuation. 
      for(i=0;i<10;i++){
        lux = analogRead(lightPin);
      }
      
      lux = cnvToLux(lux);   

      // Prints The Current Light Intensity Value
      Serial.print("LIGHT:\t");
      Serial.print(lux);
      Serial.println(" lux");
      
      delay(1000);
  
      // Analysis Of The Light Intensity                 
      // We use LEDs to warn the farmer. Yellow LED is for low level (less important), and red LED is for high level (more important) emergencies.
      // As a counter to the low light intensity, we also activate an RGB led to temporarily minimize the damage. 
      // Optimal ranges for plant types are defined in the report. 
      Serial.print("Status:\t");
      if (night == 0){ // ----------------------------------------- OPTIMAL DAY MODE COND.
        
        if (lux<=20){       // ------------------------------------ Dark
          
          Serial.println("Dark, it is not OK.");           
          lightAlarm(redPin);          
          checkLight=0;   
          digitalWrite(B, HIGH); 
          digitalWrite(G, HIGH);
          digitalWrite(R, HIGH);  
          
        }else if (lux>20 && lux<=130 ){ // ------------------------ Low Light Intensity
          
          Serial.println("Low Light, optimal for winter plants!");
          if(lux<50) digitalWrite(R, HIGH);
            
          if(!seasonType.equalsIgnoreCase(winter)){
              checkLight=0;
              lightAlarm(yellowPin);
              digitalWrite(G, HIGH);  
          }else{
              checkLight=1;
          }        
          
        }else if (lux>130 && lux<=320){ // ----------------------- Normal Light Intensity
          
          Serial.println("Normal, optimal for all plant types.");
          checkLight=1; 
          
        }else if (lux >= 350) {    // ---------------------------- High Light Intensity
          
            Serial.println("High Light, optimal for non-winter plants.");
            
            if(seasonType.equalsIgnoreCase(winter)){
                checkLight=0;
                closeRGB();
                lightAlarm(yellowPin);
            }else{
                checkLight=1;
            }        
        }  
             
     }else{          // ----------------------------------------- OPTIMAL NIGHT MODE COND.      
        if(lux>20){             // ------------------------------ Lightened          
            Serial.println("Lightened, it is not OK."); 
            closeRGB();          
            lightAlarm(redPin);
            checkLight=0;          
        }else{                  // ------------------------------ Dark
            checkLight=1;
        }        
     }
        
      Serial.println("");
      //--------------------------------- LIGHT ENDS --------------------------------------
  
      //--------------------------------- HUMIDITY BEGINS ---------------------------------
  
      // Humidity sensor gives analog output at 2^10 bits resolution. 1023 is sensor reading for the driest case. 
      // When the water increases the sensor can drop down to 400. 
      // So the humidity can change between 400-1000 for a plant according to this sensor. By using that interval, 
      // we mapped it to 0-100 as percentage value of the humidity.

      // Humidity Measurement 
      // We do the measurement for a couple time to be ensured that the value is not derived from the internal noise fluctuation. 
      for(i=0;i<20;i++){
        hum = analogRead(humPin);
      }
      hum = cnvToPerc(hum);

      // Prints The Current Humidity Value
      Serial.print("Humidity of the soil:\t ");
      Serial.print(hum);
      Serial.println("%");
      delay(1000);

      // Analysis Of The Humidity
      // We use single white LED to warn the user. If it is turned on, it indicates low humidity. If it blinks, that indicates high humidity.
      // Optimal ranges for plant types are defined in the report.  
      Serial.print("Status:\t ");
      if(hum<40){   // --------------------------------------- Low Humidity
        
        Serial.println("Dry");
        checkHum=0;      
        digitalWrite(whitePin, HIGH);
        
        
      }else if(hum>=40 && hum<50){ // ------------------------ Normal I, for water-hater
          if(waterPref.equalsIgnoreCase(waterHater)){
              Serial.println("Optimal");
              checkHum=1;
              digitalWrite(whitePin, LOW);
          }else{
              Serial.println("Dry");
              checkHum=0;      
              digitalWrite(whitePin, HIGH);
          }
      }else if(hum>50 && hum<65){ // ------------------------ Normal II, for water-lover 
          if(waterPref.equalsIgnoreCase(waterLover)){
              Serial.println("Optimal");
              checkHum=1;
              digitalWrite(whitePin, LOW);
          }else{
              Serial.println("Too Humid");
              checkHum=0;
              for(i=0;i<4;i++){      
                  digitalWrite(whitePin, HIGH);
                  delay(250);
                  digitalWrite(whitePin, LOW);
              }
          }
          
      }else{    // ------------------------------------------- High Humidity
          Serial.println("Too Humid");
          checkHum=0;
          for(i=0;i<4;i++){      
             digitalWrite(whitePin, HIGH);
             delay(250);
             digitalWrite(whitePin, LOW);
          }
      } 
  
      Serial.println("");
  
      //--------------------------------- HUMIDITY ENDS ---------------------------------

  
      //--------------------------------- GENERAL WARNINGS AND UPDATES ---------------------------------  
      
      // We use a final check system to indicate that everything is fine or there is an issue.
      delay(1000);
      if ((checkTemp == 1) && (checkLight == 1) && (checkHum == 1)){   // -- OKAY    
          Serial.println("All Conditions are Optimal!");        
      }else {
          Serial.println("WARNINGS:");    // ------------------------------- NOT OKAY
          if(checkTemp == 0){
              Serial.println(" - Check the temperature!");
          }
          if(checkLight == 0){
              Serial.println(" - Check the light!");
          }
          if(checkHum == 0){
              Serial.println(" - Check the humidity!");
          }     
      }
      
      Serial.println("");
      Serial.println("");
      delay(5000);

      // Button updates related to the day/night mode
      updated = night;  // previous value of night before it takes the pressed variable's value
      
      if(pressed==1){   // chenges the mode according to the pressed variable
        night = 1;
      }else{
        night = 0;
      }

    }
   
}
