
import sys
import tkinter as tk
import RPi.GPIO as GPIO

# --- GPIO Setup ---
GPIO.setmode(GPIO.BCM)  
GPIO.setwarnings(False)

RED_PIN = 17
BLUE_PIN = 27
GREEN_PIN = 22

# set pins as outputs
GPIO.setup(RED_PIN, GPIO.OUT)
GPIO.setup(BLUE_PIN, GPIO.OUT)
GPIO.setup(GREEN_PIN, GPIO.OUT)

def set_led(color):
    """Turn on one LED and turn off the others."""
    if color == 'red':
        GPIO.output(RED_PIN, GPIO.HIGH)
        GPIO.output(BLUE_PIN, GPIO.LOW)
        GPIO.output(GREEN_PIN, GPIO.LOW)
    elif color == 'green':
        GPIO.output(GREEN_PIN, GPIO.HIGH)
        GPIO.output(RED_PIN, GPIO.LOW)
        GPIO.output(BLUE_PIN, GPIO.LOW)
    elif color == 'blue':
        GPIO.output(BLUE_PIN, GPIO.HIGH)
        GPIO.output(GREEN_PIN, GPIO.LOW)
        GPIO.output(RED_PIN, GPIO.LOW)
    else:
        # all off
        GPIO.output(RED_PIN, GPIO.LOW)
        GPIO.output(GREEN_PIN, GPIO.LOW)
        GPIO.output(BLUE_PIN, GPIO.LOW)

def on_exit():
    """Cleanup and close app."""
    set_led('off')     # switch all LEDs off
    GPIO.cleanup()     # release GPIO pins
    root.destroy()
    sys.exit(0)

# --- Tkinter GUI ---
root = tk.Tk()
root.title("RPi LED Controller - SIT210 Task 5.1P")
root.resizable(False, False)

var = tk.StringVar(value='none')

frame = tk.Frame(root, padx=12, pady=12)
frame.pack()

tk.Label(frame, text="Select an LED (others will be OFF):").pack(anchor='w')

tk.Radiobutton(frame, text="Red",   variable=var, value='red',   command=lambda: set_led('red')).pack(anchor='w')
tk.Radiobutton(frame, text="Green", variable=var, value='green', command=lambda: set_led('green')).pack(anchor='w')
tk.Radiobutton(frame, text="Blue",  variable=var, value='blue',  command=lambda: set_led('blue')).pack(anchor='w')

tk.Button(frame, text="Exit", width=10, command=on_exit).pack(pady=(10,0))

root.protocol("WM_DELETE_WINDOW", on_exit)
root.mainloop()
