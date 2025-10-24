import cv2
import mediapipe as mp
# Import drawing utilities and styles
from mediapipe.python.solutions import drawing_utils as mp_drawing
from mediapipe.python.solutions import hands as mp_hands
from mediapipe.python.solutions import drawing_styles
from mediapipe.framework.formats import landmark_pb2
import serial # For sending data to Arduino
import time
from picamera2 import Picamera2
import numpy as np
import os # For checking file existence

# --- 1. SETUP MEDIAPIPE TASKS ---
BaseOptions = mp.tasks.BaseOptions
GestureRecognizer = mp.tasks.vision.GestureRecognizer
GestureRecognizerOptions = mp.tasks.vision.GestureRecognizerOptions
GestureRecognizerResult = mp.tasks.vision.GestureRecognizerResult
VisionRunningMode = mp.tasks.vision.RunningMode

# --- 2. GLOBAL VARIABLES & CALLBACK ---
latest_gesture = "None"
latest_landmarks = None
recognition_running = True
last_sent_gesture = "None" # Keep track of the last sent gesture

def save_result(result: GestureRecognizerResult, output_image: mp.Image, timestamp_ms: int):
    """Callback function to save the latest gesture result."""
    global latest_gesture, latest_landmarks
    latest_landmarks = None # Clear previous landmarks

    if result.gestures:
        gesture = result.gestures[0][0]
        latest_gesture = gesture.category_name
    else:
        latest_gesture = "None"

    if result.hand_landmarks:
        latest_landmarks = result.hand_landmarks[0] # Get landmarks of the first hand

# --- 3. CONFIGURE AND CREATE THE GESTURE RECOGNIZER ---
model_path = 'gesture_recognizer.task' # Using the default Google model
if not os.path.exists(model_path):
    print(f"ERROR: Model file not found at {model_path}")
    exit()

options = GestureRecognizerOptions(
    base_options=BaseOptions(model_asset_path=model_path),
    running_mode=VisionRunningMode.LIVE_STREAM,
    result_callback=save_result,
    num_hands=1,
    min_hand_detection_confidence=0.5, # Adjusted confidence
    min_tracking_confidence=0.5        # Adjusted confidence
)

try:
    recognizer = GestureRecognizer.create_from_options(options)
    print("Gesture Recognizer created successfully.")
except Exception as e:
    print(f"Error creating Gesture Recognizer: {e}")
    exit()

# --- 4. DEFINE CUSTOM DRAWING STYLE (Plain White) ---
landmark_drawing_spec = mp_drawing.DrawingSpec(color=(255, 255, 255), thickness=2, circle_radius=2)
connection_drawing_spec = mp_drawing.DrawingSpec(color=(255, 255, 255), thickness=2)

# --- 5. SERIAL COMMUNICATION SETUP ---
arduino = None
try:
    

    
    arduino_port = '/dev/ttyACM0'
    baud_rate = 9600
    # --------------------------------------
    arduino = serial.Serial(port=arduino_port, baudrate=baud_rate, timeout=0.1)
    print(f"Arduino connected successfully on port {arduino_port}.")
    time.sleep(2) # Give Arduino time to reset after connection
except serial.SerialException as e:
    print(f"Error connecting to Arduino on {arduino_port}: {e}")
    print("Running without Arduino connection. Gestures will only be shown on screen.")
except Exception as e:
    print(f"An unexpected error occurred during serial setup: {e}")


# --- 6. PICAMERA2 SETUP ---
picam2 = Picamera2()

camera_width = 480
camera_height = 360
try:
    picam2.configure(picam2.create_preview_configuration(main={"format": 'RGB888', "size": (camera_width, camera_height)}))
    picam2.start()
    print(f"Picamera2 started successfully at {camera_width}x{camera_height}.")
except Exception as e:
    print(f"Error starting Picamera2: {e}")
    if recognizer: recognizer.close()
    if arduino: arduino.close()
    exit()
# --- 7. MAIN LOOP ---
print("Starting gesture detection... Press 'q' to quit.")
frame_timestamp_ms = 0
frame_counter = 0
frame_skip_rate = 4 # Process every 4th frame

try:
    while recognition_running:
        frame_counter += 1
        rgb_frame_orig = picam2.capture_array()

        # --- Process only every Nth frame ---
        process_this_frame = (frame_counter % frame_skip_rate == 0)

        if process_this_frame:
            # --- Input Flip (Choose ONE) ---
            # Corrects image orientation FOR MEDIAPIPE based on camera inversion
            # rgb_frame_for_mediapipe = cv2.flip(rgb_frame_orig, 0)  # Vertical Only
            # rgb_frame_for_mediapipe = cv2.flip(rgb_frame_orig, 1)  # Horizontal Only
            rgb_frame_for_mediapipe = cv2.flip(rgb_frame_orig, -1) # Both Flips (Example)
            # rgb_frame_for_mediapipe = rgb_frame_orig               # No Flip
            # ----------------------------------------------------

            mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb_frame_for_mediapipe)
            frame_timestamp_ms = int(time.time() * 1000)
            recognizer.recognize_async(mp_image, frame_timestamp_ms)
        # ------------------------------------

        # --- Display Logic (Runs every frame) ---
        frame_display = cv2.cvtColor(rgb_frame_orig, cv2.COLOR_RGB2BGR)

        # --- Display Flip (Choose ONE) ---
        # Orients the FINAL image you see
        # frame_display = cv2.flip(frame_display, 0)  # Vertical Only
        # frame_display = cv2.flip(frame_display, 1)  # Horizontal Only (Mirror)
        frame_display = cv2.flip(frame_display, -1) # Both Flips (Mirror + Invert Correction)
        # -----------------------------------------------------------

        # --- Draw Landmarks ---
        if latest_landmarks:
            hand_landmarks_proto = landmark_pb2.NormalizedLandmarkList()
            hand_landmarks_proto.landmark.extend([
                landmark_pb2.NormalizedLandmark(x=landmark.x, y=landmark.y, z=landmark.z)
                for landmark in latest_landmarks
            ])
            mp_drawing.draw_landmarks(
                frame_display,
                hand_landmarks_proto,
                mp_hands.HAND_CONNECTIONS,
                landmark_drawing_spec,
                connection_drawing_spec
            )

        # Display gesture name
        # Only displays if it's not "None" or different from last frame's valid gesture
        display_gesture = latest_gesture if latest_gesture != "None" else ""
        if display_gesture: # Only draws text if a valid gesture is recognized
            cv2.putText(frame_display, display_gesture, (10, 50),
                        cv2.FONT_HERSHEY_SIMPLEX, 1.5, (0, 255, 0), 3, cv2.LINE_AA)

        # --- Send to Arduino (Only when gesture changes) ---
        current_valid_gesture = latest_gesture if latest_gesture != "None" else None
        if arduino and current_valid_gesture and current_valid_gesture != last_sent_gesture:
            message = f"{current_valid_gesture}\n"
            arduino.write(message.encode('utf-8'))
            print(f"Sent to Arduino: {current_valid_gesture}")
            last_sent_gesture = current_valid_gesture # Update last sent gesture
        elif latest_gesture == "None":
            last_sent_gesture = "None" # Reset if no gesture detected

        cv2.imshow('Gesture Recognition - Raspberry Pi', frame_display)

        # --- Exit Condition ---
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            recognition_running = False
        
        
finally:
    # --- Cleanup ---
    print("Stopping...")
    if 'recognizer' in locals():
        recognizer.close()
    if 'picam2' in locals():
        picam2.stop()
    cv2.destroyAllWindows()
    if arduino:
        arduino.close()
        print("Arduino connection closed.")
    print("Stream stopped and resources released.")
