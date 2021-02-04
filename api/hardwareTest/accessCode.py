import urequests as requests
import ujson
import connectWifi

#connect to the wifi
connectWifi.connect()

url = "https://dev-42331gx2.us.auth0.com/oauth/token"
headers = {'content-type':'application/json'}
data = '{"client_id":"Goi7MZiF0YCI4YAykw5gbd5Xky8s4JUj","client_secret":"v6sEKlu8WE6OVGpSXamVLEM8W9_IKSwg6eeaQJmY0qdCIrLfhfQP1j-GRDSOEL6K","audience":"sams-dwh-web-api","grant_type":"client_credentials"}'
response = requests.post(url,data=data,headers=headers)
results = response.json()
print(results)
