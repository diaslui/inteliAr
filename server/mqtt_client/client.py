import paho.mqtt.client as mqtt

BROKER = 'localhost'  
PORT = 1883
TOPIC = 'iot/dev'

def on_connect(client, userdata, flags, rc):
    print("\033[0;32mconnected in mqtt broker.\nCODE :", str(rc) + " \033[0m")
    client.subscribe(TOPIC)

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    print(f"new msg via mqtt: {payload}")

def start_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    try:
        client.connect(BROKER, PORT, 60)
    except Exception as e:
        print(f"\033[0;31mfailed to connect to MQTT broker: {e} \033[0m")


    client.loop_forever()

