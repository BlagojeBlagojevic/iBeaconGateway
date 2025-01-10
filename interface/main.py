import paho.mqtt.client as paho
import numpy as np
import time
import threading
import PySimpleGUI as sg

import asyncio
import platform
from pyray import *
from raylib import *

def on_connect(client, userdata, flags, rc):
   print('CONNACK received with code %d.' % (rc))



#client = paho.Client()
#client.on_connect = on_connect
#client.on_publish = on_publish
#client.on_connect = on_connect
#client.on_subscribe = on_subscribe
#client.connect('192.168.0.20', 1883)

#client.loop_forever()
#client.loop_start()
#client.subscribe('temperature')
#while 1:
 #       (rc, mid) = client.publish('temperature', "10", qos=1)
 #       #print(f'rc = {rc}, mid = {mid}')
 #       time.sleep(0.4)
#client.disconnect()

global recive
global message
global topic


def on_subscribe(client, userdata, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))
    global recive    
    recive = msg.payload
    print(recive)

def mqtt_init():
    client = paho.Client()
    client.on_subscribe = on_subscribe
    client.on_message = on_message
    client.connect('192.168.0.20', 1883)
    #client.subscribe('encyclopedia/#', qos=1)
    t1 = threading.Thread(target = client.loop_forever, name="Nesto")
    t1.start()
    return client

def subscribe_to_mqtt(client, str):
    client.subscribe(str, qos=1)


async def send_data_to_topic():
    pass

async def procces(txt):
    key = GetCharPressed()
    if (key >= 32) & (key <= 125):
        txt = txt + chr(key)
    elif IsKeyPressed(KEY_BACKSPACE):
        txt = txt[:-1]
    return txt

async def draw_text_box(x, y, width, height, colorBox, txt, colorTxt):
  mouseOnText = False
  if (CheckCollisionPointRec(GetMousePosition(), [x, y, width, height])): 
        mouseOnText = True
  else: 
        mouseOnText = False
  draw_rectangle(x, y, width, height, colorBox)
  if mouseOnText == True:
    txt = await procces(txt)
  draw_text(txt, x, y, 14, colorTxt)
  return txt
   
async def draw_button(text, x, y, width, height, color):
   # draw_rectangle(x, y, width, height, color)
   # gui_label_button([x, y, width, height], text)
    val = gui_button([x, y, width, height], text)
    return val

client = mqtt_init()
subscribe_to_mqtt(client, 'parking/#')
async def main():   # You MUST have an async main function
    init_window(800, 640, "Hello")
    global recive
    global topic
    global message
    recive = ""
    topic = ""
    message = ""
    while not window_should_close():
        begin_drawing()
        clear_background(BLACK)
        topic = await draw_text_box(10, 10, 300, 20, WHITE, topic, BLACK)
        isSub = await draw_button("Sub", 30, 30, 100, 30, GREEN)
        if(isSub == True):
            subscribe_to_mqtt(client, topic)
            topic = ""
        message = await draw_text_box(10 + 350, 10, 300, 20, WHITE, message, BLACK)
        isMes = await draw_button("Publish", 30 + 350, 30, 100, 30, GREEN)
        if(isMes == True):
            #subscribe_to_mqtt(client, topic)
            (rc, mid) = client.publish(topic, message, qos=1)
            message = ""
        end_drawing()
        await asyncio.sleep(0) 
    close_window()

asyncio.run(main())