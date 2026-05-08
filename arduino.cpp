const int pinVRx1 = A0; // Controls Turning (Left/Right)
const int pinVRy1 = A1; // Controls Forward/Reverse
const int pinSW1  = 2;  // Digital Switch 1

// Right Joystick (Mechanism - Pick & Place)
const int pinVRx2 = A2; // Controls Gripper Grab/Release
const int pinVRy2 = A3; // Controls Gripper Up/Down
const int pinSW2  = 3;  // Digital Switch 2

void setup() {
  // Initialize UART communication to ESP32 at 115200 baud
  Serial.begin(115200);

  // Initialize switch pins with internal pull-up resistors
  // This prevents floating signals; unpressed = HIGH (1), pressed = LOW (0)
  pinMode(pinSW1, INPUT_PULLUP);
  pinMode(pinSW2, INPUT_PULLUP);
}

// --- DEADBAND & MAPPING FUNCTION ---
// Converts raw 0-1023 analog readings to a -255 to +255 speed range, 
// while ignoring small resting movements.
int convert(int v) {
  int mid = 512;
  if (v > mid + 30) return map(v, mid + 30, 1023, 0, 255);
  if (v < mid - 30) return map(v, mid - 30, 0, 0, -255);
  return 0; // Inside deadband, return 0
}

void loop() {
  // 1. Read Left Joystick (Locomotion)
  int rawX1 = analogRead(pinVRx1);
  int rawY1 = analogRead(pinVRy1);
  int s1_state = (digitalRead(pinSW1) == LOW) ? 1 : 0; 

  // 2. Read Right Joystick (Mechanism)
  int rawX2 = analogRead(pinVRx2);
  int rawY2 = analogRead(pinVRy2);
  int s2_state = (digitalRead(pinSW2) == LOW) ? 1 : 0; 

  // 3. Map Left Joystick to Speed Vectors
  int forward = convert(rawY1); 
  int turn    = convert(rawX1); 

  // 4. Differential Drive Math (Mixing X and Y)
  int leftSpeed  = forward + turn;
  int rightSpeed = forward - turn;

  // Constrain Motor Speeds to valid PWM limit (-255 to 255)
  leftSpeed  = constrain(leftSpeed, -255, 255);
  rightSpeed = constrain(rightSpeed, -255, 255);

  // 5. Right Joystick (Mechanism Logic)
  // F1 handles the Y-axis (Up/Down) via DC Motor
  // F2 handles the X-axis (Grab/Release)
  int F1 = convert(rawY2); 
  int F2 = convert(rawX2); 

  // 6. UART Packet Transmission
  // Formatting the string: <L,R,F1,F2,S1,S2>
  Serial.print("<");
  Serial.print(leftSpeed);
  Serial.print(",");
  Serial.print(rightSpeed);
  Serial.print(",");
  Serial.print(F1);
  Serial.print(",");
  Serial.print(F2);
  Serial.print(",");
  Serial.print(s1_state);
  Serial.print(",");
  Serial.print(s2_state);
  Serial.println(">"); // println adds the invisible newline marker at the end

  // 7. Serial Monitor Debugging
  
//   Serial.print(leftSpeed);
//    Serial.print(rightSpeed);
//  Serial.print("\t| Arm (Y/F1): "); Serial.print(F1);
//  Serial.print("\t| Grip (X/F2): "); Serial.print(F2);
//  Serial.print("\t| SW1: "); Serial.print(s1_state);
//  Serial.print("\t| SW2: "); Serial.println(s2_state);
  

  // Short delay for stability 
  delay(100); 
}