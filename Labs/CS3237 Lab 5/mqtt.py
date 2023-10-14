import paho.mqtt.client as mqtt
from time import sleep
def on_connect(client, userdata, flags, rc):
	print("Connected with result code: " + str(rc))
	# client.subscribe("hello/esp32")
	client.subscribe("weather/temp")
	# client.subscribe("weather/humidity")

def on_message(client, userdata, message):
	if message.topic == "weather/temp":
		print(f"Received message: {message.payload.decode()} °C on topic: {message.topic}")
		float_temp = float(message.payload.decode('utf-8'))
		reply = classify_temp(float_temp)
		if reply:
			client.publish("classification/temp", reply)
	# else:
	# 	print(f"Received message: {message.payload.decode()} % on topic: {message.topic}")
	# if message.topic == "hello/esp32":
	# 	print(f"Received message: {message.payload.decode()} on topic: {message.topic}")

def classify_temp(temp):
	reply = None
	if temp > 30:
		print("Too hot! Opening window...")
		reply = 0
	elif temp < 25:
		print("Too cold! Closing window...")
		reply = 1
	else:
		print("Between 25-30. Partially opening window...")
		reply = 2

	return reply

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
