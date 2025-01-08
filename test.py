# -*- coding: utf-8 -*-
"""
Created on Fri Nov 22 13:12:42 2024

@author: BLAGI
"""

import requests
import random
import time

def generateTag():
    tag = f"""{'{'}"msg": "advData", "obj": [
{'{'}
	"dmac": "51DC0EA4AE30", 
	"refpower": {random.randint(-100, -34)}, 
	"uuid": "FB349B5F80000080001000003CFE0000", 
	"majorID": {random.randint(10, 1000)}, 
	"rssi": {random.randint(-100, -34)}, 
	"minorID": {random.randint(10, 1000)},
	"type": 4, 
	"time": "2019-09-02 09:47:42" 
{'}'}
],"gmac": "D03304002122" {'}'}
""" 

    return tag

def generateParkingTriger():
    parking = f"""{'{'}
        "event": "Parking Space Detection",
        "device": "Network Camera",
        "time": "2021-03-30 13:51:56",
        "report_type": "trigger",
        "resolution_w": 3840,
        "resolution_h": 2160,
        "parking_area": "{chr(65 + random.randint(0, 1))}",
        "index_number": {random.randint(2, 10)},
        "occupancy": {random.randint(0, 1)},
        "coordinate_x1": 3,
        "coordinate_y1": 220,
        "coordinate_x2": 13,
        "coordinate_y2": 220,
        "coordinate_x3": 3,
        "coordinate_y3": 330,
        "coordinate_x4": 13,
        "coordinate_y4": 330
{'}'}
"""
    return parking    

def generateParkingInterval():
    parking = """
    {
"event": "Parking Space Detection",
"device": "Network Camera",
"time": "2021-03-30 13:51:56",
"report_type": "interval",
"total_occupied": 217,
"total_available": 12,
"parking_detail":
[
{"area_name": "A",
"numbering_scheme": [2,3,4,5,6,7,8,9,10],
"occupancy": [1,0,0,1,0,1,1,0,0]
},
{"area_name": "B",
"numbering_scheme": [1,2,3,4,5,6,7,8,9,10],
"occupancy": [1,1,0,1,0,1,1,1,0,1]
}
]
}
"""
    return parking



tag = []
tag.append(generateTag())
tag.append(generateTag())
tag.append(generateTag())
timer = 0
ipBle = 'http://192.168.50.165/postBle'
ipCamera = 'http://192.168.50.165/postCamera'

while(1):
    if(timer%3600 == 0):
        print("parking inerval ")
        r = requests.post(ipCamera, str(generateParkingInterval()))
    if(timer%10 == 0):
        print("parking triger ")
        r = requests.post(ipCamera, str(generateParkingTriger()))
    if(timer%1== 0):
        index = random.randint(0, len(tag) - 1)
        print(f"send {index}")
        r = requests.post(ipBle, str(tag[index]))
    temp = random.randint(0, 101) 
    if(temp / 100 == 0):
        tag.append(generateTag())
        print('added ', len(tag) - 1)
    temp = random.randint(0, 101) 
    if(temp / 100 == 0):
        index = random.randint(0, len(tag) - 1)
        print('removed ', index)
        tag.pop(index)    
       
    time.sleep(1)
    timer = timer + 1

#requests.post('http://192.168.50.165/postBle', tag)
