#!/usr/bin/python3

import requests, random
import time, datetime

HOSTNAME = "things.interstitial.coop"

class Device():

    def __init__(self, tok, lat, lon):
        self.token = tok
        self.lat0 = lat
        self.lon0 = lon
        self.voltage = random.uniform(45,55)

    def updateVoltage(self):
        # Random Walk
        self.voltage = self.voltage + random.uniform(-0.5,0.5)
        if self.voltage <= 45:
            self.voltage = 46.
        if self.voltage >= 55:
            self.voltage = 54.

def doTheThing(devices):
    for device in devices:
        device.updateVoltage()
        data = {}
        data['latitude'] = device.lat0 + random.uniform(0,0.01)
        data['longitude'] = device.lon0 + random.uniform(0,0.01)
        data['voltage'] = device.voltage
        url = "https://things.interstitial.coop/api/v1/%s/telemetry" % device.token
        response = requests.post(url, json=data)
        print(response)

if __name__ == '__main__':

    myDevices = []
    #myDevices.append(Device("mySecretToken", 47.6062, -122.3321))  # Vampire4 in Seattle

    while True:
        print("Posting at %s" % datetime.datetime.now().strftime("%d/%m/%Y %H:%M:%S"))
        doTheThing(myDevices)
        time.sleep(10)
