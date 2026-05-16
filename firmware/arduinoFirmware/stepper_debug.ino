// /*
//     O              -> open claw
//     C              -> close claw
//     P <degA> <degB> -> move base (A) to degA and pivot (B) to degB, absolute, 0-180
// */

// #include <AccelStepper.h>
// #include <Servo.h>

// // ===== PIN CONFIG =====
// const uint8_t BASE_IN1 = 4;
// const uint8_t BASE_IN2 = 5;
// const uint8_t BASE_IN3 = 6;
// const uint8_t BASE_IN4 = 7;

// const uint8_t PIVOT_IN1 = 9;
// const uint8_t PIVOT_IN2 = 10;
// const uint8_t PIVOT_IN3 = 11;
// const uint8_t PIVOT_IN4 = 12;

// const uint8_t SERVO_PIN = 3;

// // ===== MOTION CONFIG =====
// const long  STEPS_PER_REV     = 4096L;
// const float STEPS_PER_DEGREE  = STEPS_PER_REV / 360.0f;

// const float BASE_MAX_SPEED  = 520.0;
// const float BASE_ACCEL      = 180.0;
// const float PIVOT_MAX_SPEED = 360.0;
// const float PIVOT_ACCEL     = 120.0;

// const int MAX_ANGLE = 180;

// const int CLAW_OPEN_ANGLE   = 25;
// const int CLAW_CLOSED_ANGLE = 85;

// // 28BYJ-48 + ULN2003 with AccelStepper: pin order IN1, IN3, IN2, IN4
// AccelStepper baseStepper(AccelStepper::HALF4WIRE, BASE_IN1, BASE_IN3, BASE_IN2, BASE_IN4);
// AccelStepper pivotStepper(AccelStepper::HALF4WIRE, PIVOT_IN1, PIVOT_IN3, PIVOT_IN2, PIVOT_IN4);
// Servo claw;

// // ===== STATE =====
// int  posA = 0;          // last commanded angle, base
// int  posB = 0;          // last commanded angle, pivot
// bool clawOpen = false;  // last commanded claw state

// char line[48];
// uint8_t lineLength = 0;

// // ===== HELPERS =====
// long degToSteps(long degrees) {
//   if (degrees >= 0) return (long)(degrees * STEPS_PER_DEGREE + 0.5f);
//   return (long)(degrees * STEPS_PER_DEGREE - 0.5f);
// }

// int clampAngle(int deg) {
//   if (deg < 0) return 0;
//   if (deg > MAX_ANGLE) return MAX_ANGLE;
//   return deg;
// }

// void moveBothTo(int targetA, int targetB) {
//   targetA = clampAngle(targetA);
//   targetB = clampAngle(targetB);

//   // Non-blocking: just set the new target. The actual stepping happens in loop().
//   baseStepper.moveTo(degToSteps(targetA));
//   pivotStepper.moveTo(degToSteps(targetB));

//   posA = targetA;
//   posB = targetB;

//   Serial.print(F("OK P "));
//   Serial.print(posA);
//   Serial.print(F(" "));
//   Serial.println(posB);
// }

// void setClaw(bool open) {
//   if (open == clawOpen) {
//     Serial.println(F("OK CLAW nochange"));
//     return;
//   }
//   claw.write(open ? CLAW_OPEN_ANGLE : CLAW_CLOSED_ANGLE);
//   clawOpen = open;
//   Serial.print(F("OK CLAW "));
//   Serial.println(open ? F("open") : F("closed"));
// }

// // ===== SERIAL DECODING =====
// void handleLine(char *cmd) {
//   // Trim leading whitespace
//   while (*cmd == ' ' || *cmd == '\t') cmd++;
//   if (*cmd == '\0') return;

//   char first = cmd[0];

//   if (first == 'O' || first == 'o') { setClaw(true);  return; }
//   if (first == 'C' || first == 'c') { setClaw(false); return; }

//   if (first == 'P' || first == 'p') {
//     int degA = 0, degB = 0;
//     if (sscanf(cmd, " %*c %d %d", &degA, &degB) == 2) {
//       moveBothTo(degA, degB);
//       return;
//     }
//     Serial.println(F("ERR format"));
//     return;
//   }

//   Serial.print(F("ERR unknown: "));
//   Serial.println(cmd);
// }

// void readSerialLine() {
//   while (Serial.available() > 0) {
//     char c = (char)Serial.read();

//     if (c == '\r' || c == '\n') {
//       if (lineLength > 0) {
//         line[lineLength] = '\0';
//         handleLine(line);
//         lineLength = 0;
//       }
//       return;
//     }

//     if (c >= 32 && c <= 126 && lineLength < sizeof(line) - 1) {
//       line[lineLength++] = c;
//     }
//   }
// }

// // ===== ARDUINO ENTRY POINTS =====
// void setup() {
//   Serial.begin(115200);
//   while (!Serial) { ; }

//   baseStepper.setMaxSpeed(BASE_MAX_SPEED);
//   baseStepper.setAcceleration(BASE_ACCEL);
//   pivotStepper.setMaxSpeed(PIVOT_MAX_SPEED);
//   pivotStepper.setAcceleration(PIVOT_ACCEL);

//   claw.attach(SERVO_PIN);
//   claw.write(CLAW_CLOSED_ANGLE);

//   Serial.println(F("READY"));
// }

// void loop() {
//   readSerialLine();
//   baseStepper.run();
//   pivotStepper.run();
// }