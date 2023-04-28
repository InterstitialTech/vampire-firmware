#!/usr/bin/python3

import requests, random, sys

ACCESS_TOKEN = "AJgYBgE6k67FPz43dM1u"

data = {}
data['V'] = random.randrange(4096)  # random voltage
data['I'] = random.randrange(4096)  # random current

if len(sys.argv) >= 2:
    hostname = sys.argv[1]
else:
    hostname = 'localhost'

url = "https://things.interstitial.coop/api/v1/%s/telemetry" % ACCESS_TOKEN

# for debugging..
#req = requests.Request('POST', url, headers={'X-Custom':'Test'}, json=data)
#req_prepared = req.prepare()

response = requests.post(url, json=data)
print(response)
