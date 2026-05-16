#include <Servo.h>
#include <Stepper.h>

// ===== PIN CONFIG =====
const int SERVO_PIN = 3;

Stepper stepperA(2048, 4, 6, 5, 7);
Stepper stepperB(2048, 8, 10, 9, 11);

Servo claw;

// ===== MOTION CONFIG =====
const int STEPS_PER_REV = 2048;
const int MAX_ANGLE = 180;
const int STEPPER_RPM = 12;

const int CLAW_OPEN_ANGLE  = 120;
const int CLAW_CLOSED_ANGLE = 0;

// ===== STATE =====
int posA = 0;
int posB = 0;
bool clawOpen = false;

// ===== HELPERS =====
int angleToSteps(int deltaDeg) {
  return (long)deltaDeg * STEPS_PER_REV / 360;
}

int clampAngle(int deg) {
  if (deg < 0) return 0;
  if (deg > MAX_ANGLE) return MAX_ANGLE;
  return deg;
}

void moveBothTo(int targetA, int targetB) {
  targetA = clampAngle(targetA);
  targetB = clampAngle(targetB);

  int deltaA = targetA - posA;
  int deltaB = targetB - posB;

  // Move A first, then B. Both are blocking; see note below if you want them simultaneous.
  if (deltaA != 0) {
    stepperA.step(angleToSteps(deltaA));
    posA = targetA;
  }
  if (deltaB != 0) {
    stepperB.step(angleToSteps(deltaB));
    posB = targetB;
  }

  Serial.print("OK P ");
  Serial.print(posA); Serial.print(" ");
  Serial.println(posB);
}

void setClaw(bool open) {
  if (open == clawOpen) {
    Serial.println("OK CLAW nochange");
    return;
  }
  claw.write(open ? CLAW_OPEN_ANGLE : CLAW_CLOSED_ANGLE);
  clawOpen = open;
  Serial.print("OK CLAW "); Serial.println(open ? "open" : "closed");
}

// ===== SERIAL DECODING =====
void handleLine(String line) {
  line.trim();
  if (line.length() == 0) return;

  char first = line.charAt(0);

  if (first == 'O') { setClaw(true);  return; }
  if (first == 'C') { setClaw(false); return; }

  if (first == 'P') {
    // Expect "P <degA> <degB>"
    int sp1 = line.indexOf(' ');
    int sp2 = line.indexOf(' ', sp1 + 1);
    if (sp1 < 0 || sp2 < 0) { Serial.println("ERR format"); return; }
    int degA = line.substring(sp1 + 1, sp2).toInt();
    int degB = line.substring(sp2 + 1).toInt();
    moveBothTo(degA, degB);
    return;
  }

  Serial.print("ERR unknown: "); Serial.println(line);
}

void setup() {
  Serial.begin(9600);

  claw.attach(SERVO_PIN);
  claw.write(CLAW_CLOSED_ANGLE);

  stepperA.setSpeed(STEPPER_RPM);
  stepperB.setSpeed(STEPPER_RPM);

  Serial.println("READY");
}

void loop() {
  static String buf;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (buf.length() > 0) {
        handleLine(buf);
        buf = "";
      }
    } else {
      buf += c;
      if (buf.length() > 64) buf = "";
    }
  }
}