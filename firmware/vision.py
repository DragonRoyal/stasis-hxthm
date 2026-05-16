import cv2
import mediapipe as mp
import pyautogui
import time

# ===== CONFIG =====
TEXT_ON_OPEN = "hand opened"     # typed on closed -> open
TEXT_ON_CLOSE = "hand closed"    # typed on open -> closed
COOLDOWN_SECONDS = 1.0           # minimum delay between triggers
SHOW_PREVIEW = True              # set False to hide the camera window
# ==================

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

def is_hand_open(landmarks):
    """
    Returns True if the hand is open (fingers extended), False if closed (fist).
    """
    finger_pairs = [(8, 6), (12, 10), (16, 14), (20, 18)]
    extended = 0
    for tip, pip in finger_pairs:
        if landmarks[tip].y < landmarks[pip].y:
            extended += 1

    thumb_extended = abs(landmarks[4].x - landmarks[0].x) > abs(landmarks[2].x - landmarks[0].x)
    if thumb_extended:
        extended += 1

    return extended >= 4

def type_and_enter(text):
    pyautogui.typewrite(text, interval=0.01)
    pyautogui.press("enter")

def main():
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

            # Detect transitions in either direction
            now = time.time()
            if current_state is not None and prev_state is not None \
                    and current_state != prev_state \
                    and now - last_trigger_time > COOLDOWN_SECONDS:
                if prev_state == "closed" and current_state == "open":
                    type_and_enter(TEXT_ON_OPEN)
                    print(f"Triggered (open): typed '{TEXT_ON_OPEN}'")
                elif prev_state == "open" and current_state == "closed":
                    type_and_enter(TEXT_ON_CLOSE)
                    print(f"Triggered (close): typed '{TEXT_ON_CLOSE}'")
                last_trigger_time = now

            if current_state is not None:
                prev_state = current_state

            if SHOW_PREVIEW:
                cv2.imshow("Hand Detector (press q to quit)", frame)
                if cv2.waitKey(1) & 0xFF == ord("q"):
                    break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()