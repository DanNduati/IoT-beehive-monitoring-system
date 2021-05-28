import urequests as requests
import ujson
import connectWifi

#connect to the wifi
connectWifi.connect()

url = "https://dev-42331gx2.us.auth0.com/oauth/token"
headers = {'content-type':'application/json'}
data ='{"client_id":"EptrWV0x17H0AlOoTvz06akdZ5imV4U5","client_secret":"M5c70lGDh8g9Zp2cItFbQvIJnpGtJqX7tTwq3MC7wqWkR_9OD4j7iilAW0vVgzo9","audience":"sams-dwh-web-api","grant_type":"client_credentials"}'
response = requests.post(url,data=data,headers=headers)
results = response.json()
accessToken = results['access_token']
bearer = results['Bearer']
print(accessToken)
