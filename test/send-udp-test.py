import socket
import keyboard
import time

udp_ip = "192.168.1.225"
udp_port = 12345
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

minSpeed = 1400
neutral = 1510
maxSpeed = 1650
currentSpeed = neutral

minAngle = 50
midAngle = 90
maxAngle = 135
currentAngle = midAngle

previousSpeed = currentSpeed
previousAngle = currentAngle

while True:
    if keyboard.is_pressed("r"):
        # This is for resetting the WiFi connection on the ESP32
        sock.sendto(f"r".encode(), (udp_ip, udp_port))

    if not (keyboard.is_pressed("up") or keyboard.is_pressed("down")):
        currentSpeed = neutral
    if not (keyboard.is_pressed("left") or keyboard.is_pressed("right")):
        currentAngle = midAngle

    if keyboard.is_pressed("down"):
        if currentSpeed > minSpeed:
            currentSpeed = int(currentSpeed) - 1
    elif keyboard.is_pressed("up"):
        if currentSpeed < maxSpeed:
            currentSpeed = int(currentSpeed) + 1
    if keyboard.is_pressed("left"):
        if currentAngle < midAngle:
            currentAngle = midAngle
        elif currentAngle < maxAngle:
            currentAngle = int(currentAngle) + 2
    elif keyboard.is_pressed("right"):
        if currentAngle > midAngle:
            currentAngle = midAngle
        elif currentAngle > minAngle:
            currentAngle = int(currentAngle) - 2
    print(f"Current angle: {currentAngle}\t Current speed: {currentSpeed}")

    if previousSpeed != currentSpeed or previousAngle != currentAngle:
        previousSpeed = currentSpeed
        previousAngle = currentAngle
        sock.sendto(f"{currentAngle},{currentSpeed}".encode(), (udp_ip, udp_port))

    time.sleep(0.01)
