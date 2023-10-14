import paho.mqtt.client as mqtt
from time import sleep

def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    print("Waiting for 2 seconds")
    sleep(2)

    print("Sending message.")
    # client.publish("hello/world", "This is a test")
    client.publish("hello/world", "Good Morning, I am connected now!")
    client.subscribe("hello/esp")

def on_message(client, userdata, message):
    reply = f"Message received: {message.payload}"
    print(f"{reply} on {message.topic}")
    client.publish("hello/world", f"Message received at pub.py: {message.payload}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("localhost", 1883, 60)
client.loop_forever()
