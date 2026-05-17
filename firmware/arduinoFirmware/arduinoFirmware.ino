/*
    O              -> open claw
    C              -> close claw
    P <degA> <degB> -> base (A) to degA, pivot (B) to degB, absolute, 0-180
                       A is the base servo, B is the pivot stepper
*/
#include <AccelStepper.h>
#include <Servo.h>

// ===== PIN CONFIG =====
const uint8_t BASE_SERVO_PIN = 9;   // <-- change if your base servo is on a different pin

const uint8_t PIVOT_IN1 = 4;
const uint8_t PIVOT_IN2 = 5;
const uint8_t PIVOT_IN3 = 6;
const uint8_t PIVOT_IN4 = 7;

const uint8_t CLAW_SERVO_PIN = 3;

// ===== MOTION CONFIG =====
const long  STEPS_PER_REV     = 4096L;
const float STEPS_PER_DEGREE  = STEPS_PER_REV / 360.0f;

const float PIVOT_MAX_SPEED = 360.0;
const float PIVOT_ACCEL     = 120.0;

const int MAX_ANGLE = 180;

const int CLAW_OPEN_ANGLE   = 25;
const int CLAW_CLOSED_ANGLE = 85;

AccelStepper pivotStepper(AccelStepper::HALF4WIRE, PIVOT_IN1, PIVOT_IN3, PIVOT_IN2, PIVOT_IN4);
Servo baseServo;
Servo claw;

// ===== STATE =====
int  posA = 0;          // base angle (servo)
int  posB = 0;          // pivot angle (stepper)
bool clawOpen = false;
bool servosAttached = false;
char line[48];
uint8_t lineLength = 0;

// ===== HELPERS =====
long degToSteps(long degrees) {
  if (degrees >= 0) return (long)(degrees * STEPS_PER_DEGREE + 0.5f);
  return (long)(degrees * STEPS_PER_DEGREE - 0.5f);
}

int clampAngle(int deg) {
  if (deg < 0) return 0;
  if (deg > MAX_ANGLE) return MAX_ANGLE;
  return deg;
}

bool stepperRunning() {
  return pivotStepper.distanceToGo() != 0;
}

void detachServosForStepping() {
  if (servosAttached) {
    baseServo.detach();
    claw.detach();
    servosAttached = false;
  }
}

void reattachServos() {
  if (!servosAttached) {
    baseServo.attach(BASE_SERVO_PIN);
    claw.attach(CLAW_SERVO_PIN);
    baseServo.write(posA);
    claw.write(clawOpen ? CLAW_OPEN_ANGLE : CLAW_CLOSED_ANGLE);
    servosAttached = true;
    delay(50);  // let the servos settle before stepper run() resumes
  }
}

void moveBothTo(int targetA, int targetB) {
  targetA = clampAngle(targetA);
  targetB = clampAngle(targetB);

  // Detach servos before the pivot stepper starts moving, to avoid timer conflict
  detachServosForStepping();

  // Drive the base servo, then start the pivot stepper toward its target
  // (We re-attach the base briefly to issue the command, then detach again.)
  baseServo.attach(BASE_SERVO_PIN);
  baseServo.write(targetA);
  delay(50);
  baseServo.detach();

  pivotStepper.moveTo(degToSteps(targetB));

  posA = targetA;
  posB = targetB;

  Serial.print(F("OK P "));
  Serial.print(posA);
  Serial.print(F(" "));
  Serial.println(posB);
}

void setClaw(bool open) {
  if (open == clawOpen && servosAttached) {
    Serial.println(F("OK CLAW nochange"));
    return;
  }

  reattachServos();

  claw.write(open ? CLAW_OPEN_ANGLE : CLAW_CLOSED_ANGLE);
  clawOpen = open;

  Serial.print(F("OK CLAW "));
  Serial.println(open ? F("open") : F("closed"));
}

void handleLine(char *cmd) {
  while (*cmd == ' ' || *cmd == '\t') cmd++;
  if (*cmd == '\0') return;

  char first = cmd[0];

  if (first == 'O' || first == 'o') { setClaw(true);  return; }
  if (first == 'C' || first == 'c') { setClaw(false); return; }

  if (first == 'P' || first == 'p') {
    int degA = 0, degB = 0;
    if (sscanf(cmd, " %*c %d %d", &degA, &degB) == 2) {
      moveBothTo(degA, degB);
      return;
    }
    Serial.println(F("ERR format"));
    return;
  }

  Serial.print(F("ERR unknown: "));
  Serial.println(cmd);
}

void readSerialLine() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r' || c == '\n') {
      if (lineLength > 0) {
        line[lineLength] = '\0';
        handleLine(line);
        lineLength = 0;
      }
      return;
    }
    if (c >= 32 && c <= 126 && lineLength < sizeof(line) - 1) {
      line[lineLength++] = c;
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }

  pivotStepper.setMaxSpeed(PIVOT_MAX_SPEED);
  pivotStepper.setAcceleration(PIVOT_ACCEL);

  baseServo.attach(BASE_SERVO_PIN);
  baseServo.write(0);
  claw.attach(CLAW_SERVO_PIN);
  claw.write(CLAW_CLOSED_ANGLE);
  servosAttached = true;

  Serial.println(F("READY"));
}

void loop() {
  readSerialLine();
  pivotStepper.run();

  // Once the pivot stepper stops, reattach the servos automatically
  static bool prevRunning = false;
  bool running = stepperRunning();
  if (prevRunning && !running) {
    reattachServos();
  }
  prevRunning = running;
}