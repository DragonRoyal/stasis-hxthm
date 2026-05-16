import cv2
import mediapipe as mp
import serial
import time
import threading
import tkinter as tk
from PIL import Image, ImageTk

# ===== CONFIG =====
COOLDOWN_SECONDS = 0.3
SHOW_PREVIEW = True

SERIAL_PORT = "/dev/cu.usbserial-1440"
SERIAL_BAUD = 115200
ARDUINO_BOOT_DELAY = 2.0
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


serial_lock = threading.Lock()


def send_line(ser, line):
    if ser is None:
        return
    with serial_lock:
        try:
            ser.write((line + "\n").encode())
            reply = ser.readline().decode(errors="ignore").strip()
            if reply:
                print("Arduino:", reply)
        except serial.SerialException as e:
            print(f"Serial write failed: {e}")


def send_claw(ser, opening):
    send_line(ser, "O" if opening else "C")


def main():
    arduino = connect_arduino()
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Could not open webcam.")
        return

    hands = mp_hands.Hands(
        max_num_hands=1,
        min_detection_confidence=0.7,
        min_tracking_confidence=0.5,
    )

    # ----- Tkinter UI -----
    root = tk.Tk()
    root.title("Hand Detector + Barcode Input")

    video_label = tk.Label(root)
    video_label.pack(padx=8, pady=8)

    tk.Label(root, text="Scan or type barcode, then press Enter:").pack(pady=(4, 0))
    entry = tk.Entry(root, width=40)
    entry.pack(pady=4)
    entry.focus_set()
    status = tk.Label(root, text="", fg="gray")
    status.pack(pady=(0, 8))

    def submit(event=None):
        text = entry.get().strip()
        if not text:
            return
        send_line(arduino, text)
        status.config(text=f"Sent: {text}")
        entry.delete(0, tk.END)

    entry.bind("<Return>", submit)
    tk.Button(root, text="Send", command=submit).pack(pady=(0, 8))

    # ----- State for hand detection -----
    state = {"prev": None, "last_open": 0.0, "last_close": 0.0}

    def update_frame():
        ok, frame = cap.read()
        if not ok:
            root.after(15, update_frame)
            return

        frame = cv2.flip(frame, 1)
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        results = hands.process(rgb)

        current_state = None
        if results.multi_hand_landmarks:
            hand_landmarks = results.multi_hand_landmarks[0]
            current_state = "open" if is_hand_open(hand_landmarks.landmark) else "closed"
            mp_draw.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)
            cv2.putText(
                frame, current_state.upper(), (10, 40),
                cv2.FONT_HERSHEY_SIMPLEX, 1.2,
                (0, 255, 0) if current_state == "open" else (0, 0, 255), 3,
            )

        # Independent cooldowns per direction so a fast O->C->O sequence isn't blocked
        now = time.time()
        if (current_state is not None and state["prev"] is not None
                and current_state != state["prev"]):
            if (state["prev"] == "closed" and current_state == "open"
                    and now - state["last_open"] > COOLDOWN_SECONDS):
                send_claw(arduino, True)
                print("Triggered: open")
                state["last_open"] = now
            elif (state["prev"] == "open" and current_state == "closed"
                    and now - state["last_close"] > COOLDOWN_SECONDS):
                send_claw(arduino, False)
                print("Triggered: close")
                state["last_close"] = now

        if current_state is not None:
            state["prev"] = current_state

        if SHOW_PREVIEW:
            display = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            img = Image.fromarray(display)
            imgtk = ImageTk.PhotoImage(image=img)
            video_label.imgtk = imgtk
            video_label.configure(image=imgtk)

        root.after(15, update_frame)

    def on_close():
        cap.release()
        hands.close()
        if arduino is not None:
            arduino.close()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_close)
    update_frame()
    root.mainloop()


if __name__ == "__main__":
    main()