int button = 4;
int buttonNew;
int buttonOld=1;
int isShowingMAc=0;

void setup() {
  Serial.begin(115200);
  pinMode(button, INPUT_PULLUP);
}

void loop() {

  buttonNew=digitalRead(button);
  if(buttonOld==0 && buttonNew==1){
    if (isShowingMAc==0){
      Serial.println("MAC");
      isShowingMAc=1;
    }
    else{
      Serial.println("NO MAC");
      isShowingMAc=0;
    }
  }
  buttonOld=buttonNew;
  delay(100);
}
