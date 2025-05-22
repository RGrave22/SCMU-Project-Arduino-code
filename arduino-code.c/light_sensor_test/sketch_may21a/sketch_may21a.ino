const int LIGHT_SENSOR = 0;


// Definition of variables - values that can change
int analogValue;

void setup() {
  Serial.begin(9600);
  
}

void loop() {
  // read the input from the analog pin
  analogValue = analogRead(LIGHT_SENSOR);
  
  // Check if it's above a specific threshold and turn the LED on or off
  Serial.println(analogValue);
  if(analogValue < 500)
    Serial.println("ta baixo"); // turn on LED
  else
    Serial.println("ta alto");
  
  delay(1000);
}