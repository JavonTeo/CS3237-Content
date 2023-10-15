"""
This is used for task 8.
"""

import paho.mqtt.client as mqtt
from time import sleep
count = 0
def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    print("Waiting for 2 seconds")
    sleep(2)
    client.subscribe("weather/temp")

def on_message(client, userdata, message):
    print(f"Received message: {message.payload.decode()} °C on topic: {message.topic}")
    float_temp = float(message.payload.decode('utf-8'))
    reply = classify_temp(float_temp)
    if reply != None:
        print(reply)
        client.publish("classification/temp", reply)

def classify_temp(temp):
	reply = None
	if temp > 30:
		print("Too hot! Opening window...")
		reply = "HOT"
	elif temp < 25:
		print("Too cold! Closing window...")
		reply = "COLD"
	else:
		print("Between 25-30. Partially opening window...")
		reply = "BETWEEN"

	return reply

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
