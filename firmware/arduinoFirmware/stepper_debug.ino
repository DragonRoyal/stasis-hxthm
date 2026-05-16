/*
  Robot Arm Hardware Debug Sketch

  Tests two 28BYJ-48 steppers and one servo from Serial Monitor.

  Current wiring:
  - Base ULN2003 IN1 -> Arduino D4
  - Base ULN2003 IN2 -> Arduino D5
  - Base ULN2003 IN3 -> Arduino D6
  - Base ULN2003 IN4 -> Arduino D7

  - Pivot ULN2003 IN1 -> Arduino A0
  - Pivot ULN2003 IN2 -> Arduino A1
  - Pivot ULN2003 IN3 -> Arduino A2
  - Pivot ULN2003 IN4 -> Arduino A3

  - Servo signal -> Arduino D9

  Arduino Serial Monitor:
  - Baud: 115200
  - Line ending: Newline

  Stepper commands:
  - B L 90     base left 90 degrees
  - B R 45     base right 45 degrees
  - B G 180    base go to absolute 180 degrees from HOME
  - P U 30     pivot up 30 degrees
  - P D 15     pivot down 15 degrees
  - P G 45     pivot go to absolute 45 degrees from HOME

  Servo commands:
  - S 25       servo to 25 degrees
  - OPEN       servo to open angle
  - CLOSE      servo to closed angle

  Utility commands:
  - HOME       set both stepper current positions to 0 degrees
  - WHERE      print current estimated stepper angles and servo angle
  - STOP       stop both steppers
  - HELP       print command list

  Use whole-number degrees only, like B L 45 or S 90.
*/

#include <AccelStepper.h>
#include <Servo.h>

const uint8_t BASE_IN1 = 4;
const uint8_t BASE_IN2 = 5;
const uint8_t BASE_IN3 = 6;
const uint8_t BASE_IN4 = 7;

const uint8_t PIVOT_IN1 = 9;
const uint8_t PIVOT_IN2 = 10;
const uint8_t PIVOT_IN3 = 11;
const uint8_t PIVOT_IN4 = 12;

const uint8_t SERVO_PIN = 3;

// Most 28BYJ-48 motors are 4096 half-steps per output shaft revolution.
// If 90 degrees moves too far/short, adjust this value.
const long STEPS_PER_REV = 4096L;
const float STEPS_PER_DEGREE = STEPS_PER_REV / 360.0f;

const float BASE_MAX_SPEED = 520.0;
const float BASE_ACCEL = 180.0;
const float PIVOT_MAX_SPEED = 360.0;
const float PIVOT_ACCEL = 120.0;

const int SERVO_OPEN_DEG = 25;
const int SERVO_CLOSED_DEG = 85;

// For 28BYJ-48 + ULN2003, AccelStepper usually needs this order:
// IN1, IN3, IN2, IN4.
AccelStepper baseStepper(AccelStepper::HALF4WIRE, BASE_IN1, BASE_IN3, BASE_IN2, BASE_IN4);
AccelStepper pivotStepper(AccelStepper::HALF4WIRE, PIVOT_IN1, PIVOT_IN3, PIVOT_IN2, PIVOT_IN4);
Servo gripperServo;

char line[48];
uint8_t lineLength = 0;
int servoAngle = SERVO_OPEN_DEG;

long degToSteps(long degrees) {
  if (degrees >= 0) {
    return (long)(degrees * STEPS_PER_DEGREE + 0.5f);
  }
  return (long)(degrees * STEPS_PER_DEGREE - 0.5f);
}

float stepsToDeg(long steps) {
  return steps / STEPS_PER_DEGREE;
}

void printHelp() {
  Serial.println(F(""));
  Serial.println(F("Robot arm debug commands:"));
  Serial.println(F("  B L 90   base left 90 degrees"));
  Serial.println(F("  B R 45   base right 45 degrees"));
  Serial.println(F("  B G 180  base go to absolute 180 degrees"));
  Serial.println(F("  P U 30   pivot up 30 degrees"));
  Serial.println(F("  P D 15   pivot down 15 degrees"));
  Serial.println(F("  P G 45   pivot go to absolute 45 degrees"));
  Serial.println(F("  S 25     servo to 25 degrees"));
  Serial.println(F("  OPEN     servo to open angle"));
  Serial.println(F("  CLOSE    servo to closed angle"));
  Serial.println(F("  HOME     set both stepper positions to 0 degrees"));
  Serial.println(F("  WHERE    print current estimates"));
  Serial.println(F("  STOP     stop both steppers"));
  Serial.println(F("  HELP     print this help"));
  Serial.println(F(""));
}

void printWhere() {
  Serial.print(F("Base current="));
  Serial.print(stepsToDeg(baseStepper.currentPosition()), 1);
  Serial.print(F(" deg, target="));
  Serial.print(stepsToDeg(baseStepper.targetPosition()), 1);
  Serial.print(F(" deg | Pivot current="));
  Serial.print(stepsToDeg(pivotStepper.currentPosition()), 1);
  Serial.print(F(" deg, target="));
  Serial.print(stepsToDeg(pivotStepper.targetPosition()), 1);
  Serial.print(F(" deg | Servo="));
  Serial.print(servoAngle);
  Serial.println(F(" deg"));
}

void stopSteppers() {
  baseStepper.stop();
  pivotStepper.stop();
  Serial.println(F("Stopping both steppers."));
}

void homeHere() {
  baseStepper.setCurrentPosition(0);
  pivotStepper.setCurrentPosition(0);
  baseStepper.moveTo(0);
  pivotStepper.moveTo(0);
  Serial.println(F("Base and pivot current positions set to 0 degrees."));
}

void setServoAngle(long angle) {
  if (angle < 0) {
    angle = 0;
  }
  if (angle > 180) {
    angle = 180;
  }

  servoAngle = (int)angle;
  gripperServo.write(servoAngle);
  Serial.print(F("Servo target "));
  Serial.print(servoAngle);
  Serial.println(F(" degrees"));
}

void moveRelative(AccelStepper &stepper, long degrees, const __FlashStringHelper *label) {
  stepper.move(degToSteps(degrees));
  Serial.print(label);
  Serial.print(F(" relative "));
  Serial.print(degrees);
  Serial.println(F(" degrees"));
}

void moveAbsolute(AccelStepper &stepper, long degrees, const __FlashStringHelper *label) {
  stepper.moveTo(degToSteps(degrees));
  Serial.print(label);
  Serial.print(F(" absolute target "));
  Serial.print(degrees);
  Serial.println(F(" degrees"));
}

void normalizeLine(char *text) {
  for (uint8_t i = 0; text[i] != '\0'; i++) {
    if (text[i] >= 'a' && text[i] <= 'z') {
      text[i] = text[i] - 'a' + 'A';
    }
  }
}

void handleStepperCommand(char axis, char action, long degrees) {
  AccelStepper *stepper = NULL;
  const __FlashStringHelper *label = NULL;

  if (axis == 'B') {
    stepper = &baseStepper;
    label = F("Base");
  } else if (axis == 'P') {
    stepper = &pivotStepper;
    label = F("Pivot");
  } else {
    Serial.println(F("Unknown axis. Use B for base or P for pivot."));
    return;
  }

  if (action == 'L' || action == 'U') {
    moveRelative(*stepper, degrees, label);
  } else if (action == 'R' || action == 'D') {
    moveRelative(*stepper, -degrees, label);
  } else if (action == 'G') {
    moveAbsolute(*stepper, degrees, label);
  } else {
    Serial.println(F("Unknown action. Use L/R/U/D for relative moves or G for absolute go-to."));
  }
}

void handleCommand(char *command) {
  normalizeLine(command);

  if (strcmp(command, "HELP") == 0) {
    printHelp();
    return;
  }

  if (strcmp(command, "WHERE") == 0) {
    printWhere();
    return;
  }

  if (strcmp(command, "STOP") == 0) {
    stopSteppers();
    return;
  }

  if (strcmp(command, "HOME") == 0) {
    homeHere();
    return;
  }

  if (strcmp(command, "OPEN") == 0) {
    setServoAngle(SERVO_OPEN_DEG);
    return;
  }

  if (strcmp(command, "CLOSE") == 0) {
    setServoAngle(SERVO_CLOSED_DEG);
    return;
  }

  char servoCommand = 0;
  long servoDegrees = 0;
  if (sscanf(command, " %c %ld", &servoCommand, &servoDegrees) == 2 && servoCommand == 'S') {
    setServoAngle(servoDegrees);
    return;
  }

  char axis = 0;
  char action = 0;
  long degrees = 0;
  int parsed = sscanf(command, " %c %c %ld", &axis, &action, &degrees);

  if (parsed == 3) {
    handleStepperCommand(axis, action, degrees);
  } else {
    Serial.print(F("Could not parse command: "));
    Serial.println(command);
    printHelp();
  }
}

void readSerialLine() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\r' || c == '\n') {
      if (lineLength > 0) {
        line[lineLength] = '\0';
        handleCommand(line);
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
  while (!Serial) {
    ;
  }

  baseStepper.setMaxSpeed(BASE_MAX_SPEED);
  baseStepper.setAcceleration(BASE_ACCEL);
  pivotStepper.setMaxSpeed(PIVOT_MAX_SPEED);
  pivotStepper.setAcceleration(PIVOT_ACCEL);

  gripperServo.attach(SERVO_PIN);
  setServoAngle(SERVO_OPEN_DEG);

  Serial.println(F("Robot arm hardware debug ready."));
  printHelp();
}

void loop() {
  readSerialLine();
  baseStepper.run();
  pivotStepper.run();
}
