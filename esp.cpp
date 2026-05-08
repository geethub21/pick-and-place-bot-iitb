// --- PIN DECLARATIONS ---

// L298N #1: Traction Motors (M2, M3, M4, M5)
const int ENA_1 = 27; 
const int IN1_1 = 26; 
const int IN2_1 = 25; 
const int IN3_1 = 19; 
const int IN4_1 = 21; 
const int ENB_1 = 22; 

// L298N #2: Up/Down Mechanism (M6)
const int ENA_2 = 14; 
const int IN1_2 = 12; 
const int IN2_2 = 13; 

// Servo Motor: Rack & Pinion Gripper (M1)
const int SERVO_PIN = 32; 
Servo gripperServo;
int gripperAngle = 90;    

// Limit Switch Protection (SW Push)
const int limitSwitchTop = 4; 

String buf = "";

void setup() {
  Serial.begin(115200);
  // UART communication with TX circuit
  Serial2.begin(115200, SERIAL_8N1, 16, 17);

  pinMode(IN1_1, OUTPUT); pinMode(IN2_1, OUTPUT);
  pinMode(IN3_1, OUTPUT); pinMode(IN4_1, OUTPUT);
  pinMode(ENA_1, OUTPUT); pinMode(ENB_1, OUTPUT);

  pinMode(IN1_2, OUTPUT); pinMode(IN2_2, OUTPUT);
  pinMode(ENA_2, OUTPUT);
  
  gripperServo.attach(SERVO_PIN);
  gripperServo.write(gripperAngle); 

 
// Change this:
// pinMode(limitSwitchTop, INPUT_PULLUP);

// To this:
pinMode(limitSwitchTop, INPUT);

  analogWrite(ENA_1, 0); analogWrite(ENB_1, 0);
  analogWrite(ENA_2, 0);

  Serial.println("ESP32 RX READY - Servo Mode Active...");
}

void drive(int v, int a, int b, int pwmPin) {
  int pwm = constrain(abs(v), 0, 255);

  if (v > 0) {
    digitalWrite(a, HIGH);
    digitalWrite(b, LOW);
    analogWrite(pwmPin, pwm);
  }
  else if (v < 0) {
    digitalWrite(a, LOW);
    digitalWrite(b, HIGH);
    analogWrite(pwmPin, pwm);
  }
  else {
    analogWrite(pwmPin, 0);
    digitalWrite(a, LOW);
    digitalWrite(b, LOW);
  }
}

void loop() {
  while (Serial2.available()) {
    char c = Serial2.read();

    if (c == '<') {
      buf = ""; 
    }
    else if (c == '>') {
      int c1 = buf.indexOf(',');
      int c2 = buf.indexOf(',', c1 + 1);
      int c3 = buf.indexOf(',', c2 + 1);
      int c4 = buf.indexOf(',', c3 + 1);
      int c5 = buf.indexOf(',', c4 + 1);

      if (c1 > 0 && c5 > 0) {
        int v_left  = buf.substring(0, c1).toInt();
        int v_right = buf.substring(c1 + 1, c2).toInt();
        int f_upDn  = buf.substring(c2 + 1, c3).toInt();
        int f_grab  = buf.substring(c3 + 1, c4).toInt();

        // Limit Switch Logic
        // int switchState = digitalRead(limitSwitchTop);
        // if (switchState == LOW && f_upDn > 0) {
        //     f_upDn = 0; 
        //     Serial.println("WARNING: Top limit switch triggered!");
        // }

        // --- Limit Switch Logic ---
int switchState = digitalRead(limitSwitchTop);

// Change LOW to HIGH
if (switchState == HIGH && f_upDn > 0) {
    f_upDn = 0; 
    Serial.println("WARNING: Top limit switch triggered!");
}

        // 1. Traction Motors
        drive(v_left, IN1_1, IN2_1, ENA_1);
        drive(v_right, IN3_1, IN4_1, ENB_1);

        // 2. Up/Down Timing Belt Motor
        drive(f_upDn, IN1_2, IN2_2, ENA_2);

        // 3. Gripper Servo (Incremental Control)
        if (f_grab > 50) {
          gripperAngle += 5; 
        } else if (f_grab < -50) {
          gripperAngle -= 5; 
        }
        
        gripperAngle = constrain(gripperAngle, 0, 180); 
        gripperServo.write(gripperAngle);
      }
      
      buf = ""; 
    }
    else if (isDigit(c) || c == '-' || c == ',') {
      buf += c;
    }
  }
}