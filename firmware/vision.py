import cv2
import mediapipe as mp
import pyautogui
import serial
import time

# ===== CONFIG =====
TEXT_ON_OPEN = "hand opened"
TEXT_ON_CLOSE = "hand closed"
COOLDOWN_SECONDS = 1.0
SHOW_PREVIEW = True

SERIAL_PORT = "COM3"        # Windows: "COM3" etc. Mac: "/dev/cu.usbmodem..." Linux: "/dev/ttyACM0"
SERIAL_BAUD = 9600
ARDUINO_BOOT_DELAY = 2.0    # Arduino resets when serial opens; wait for it
# ==================

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils


def is_hand_open(landmarks):
    finger_pairs = [(8, 6), (12, 10), (16, 14), (20, 18)]
    extended = sum(1 for tip, pip in finger_pairs if landmarks[tip].y < landmarks[pip].y)
    thumb_extended = abs(landmarks[4].x - landmarks[0].x) > abs(landmarks[2].x - landmarks[0].x)
    if thumb_extended:
        extended += 1
    return extended >= 4


def type_and_enter(text):
    pyautogui.typewrite(text, interval=0.01)
    pyautogui.press("enter")


def connect_arduino():
    try:
        ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=1)
        time.sleep(ARDUINO_BOOT_DELAY)
        while ser.in_waiting:
            print("Arduino:", ser.readline().decode(errors="ignore").strip())
        print(f"Connected to Arduino on {SERIAL_PORT}")
        return ser
    except serial.SerialException as e:
        print(f"WARNING: could not open {SERIAL_PORT} ({e}). Running without Arduino.")
        return None


def send_line(ser, line):
    """Send a newline-terminated command and print any reply."""
    if ser is None:
        return
    try:
        ser.write((line + "\n").encode())
        reply = ser.readline().decode(errors="ignore").strip()
        if reply:
            print("Arduino:", reply)
    except serial.SerialException as e:
        print(f"Serial write failed: {e}")


def send_claw(ser, opening):
    send_line(ser, "O" if opening else "C")


def send_position(ser, axis, degrees):
    """Move a stepper axis (e.g. from a barcode scan). axis is 'A' or 'B'."""
    axis = axis.upper()
    if axis not in ("A", "B"):
        print(f"Bad axis: {axis}")
        return
    degrees = max(0, min(180, int(degrees)))
    send_line(ser, f"P {axis} {degrees}")


def main():
    arduino = connect_arduino()
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Could not open webcam.")
        return

    prev_state = None
    last_trigger_time = 0.0

    with mp_hands.Hands(
        max_num_hands=1,
        min_detection_confidence=0.7,
        min_tracking_confidence=0.5,
    ) as hands:
        print("Running. Press 'q' in the preview window to quit.")
        while True:
            ok, frame = cap.read()
            if not ok:
                continue

            frame = cv2.flip(frame, 1)
            rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            results = hands.process(rgb)

            current_state = None
            if results.multi_hand_landmarks:
                hand_landmarks = results.multi_hand_landmarks[0]
                current_state = "open" if is_hand_open(hand_landmarks.landmark) else "closed"

                if SHOW_PREVIEW:
                    mp_draw.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)
                    cv2.putText(frame, current_state.upper(), (10, 40),
                                cv2.FONT_HERSHEY_SIMPLEX, 1.2,
                                (0, 255, 0) if current_state == "open" else (0, 0, 255), 3)

            now = time.time()
            if (current_state is not None and prev_state is not None
                    and current_state != prev_state
                    and now - last_trigger_time > COOLDOWN_SECONDS):
                if prev_state == "closed" and current_state == "open":
                    type_and_enter(TEXT_ON_OPEN)
                    send_claw(arduino, True)
                    print(f"Triggered (open): '{TEXT_ON_OPEN}'")
                elif prev_state == "open" and current_state == "closed":
                    type_and_enter(TEXT_ON_CLOSE)
                    send_claw(arduino, False)
                    print(f"Triggered (close): '{TEXT_ON_CLOSE}'")
                last_trigger_time = now

            if current_state is not None:
                prev_state = current_state

            if SHOW_PREVIEW:
                cv2.imshow("Hand Detector (press q to quit)", frame)
                if cv2.waitKey(1) & 0xFF == ord("q"):
                    break

    cap.release()
    cv2.destroyAllWindows()
    if arduino is not None:
        arduino.close()


if __name__ == "__main__":
    main()