import urequests as requests
import ujson
from machine import Pin, I2C
from time import sleep
import BME280
import connectWifi

i2c = I2C(scl=Pin(22), sda=Pin(21), freq=10000)
bme = BME280.BME280(i2c=i2c)
temp = bme.temperature

connectWifi.connect()

#urls and headers
accessUrl = "https://dev-42331gx2.us.auth0.com/oauth/token"
dataUrl = "https://anabi-smartsystems.com/api/data"
accessHeaders = {"content-type":'application/json'}
accessData ='{"client_id":"EptrWV0x17H0AlOoTvz06akdZ5imV4U5","client_secret":"M5c70lGDh8g9Zp2cItFbQvIJnpGtJqX7tTwq3MC7wqWkR_9OD4j7iilAW0vVgzo9","audience":"sams-dwh-web-api","grant_type":"client_credentials"}'

#get accesstoken and bearer
response = requests.post(accessUrl,data=accessData,headers=accessHeaders)
results = response.json()
accessToken = results['access_token']
print("The access token is: %s\n\n" %accessToken)
#post data to DW
publishHeaders = {"content-type":'application/json',"Authorization":' Bearer '+str(accessToken)+''}
print("The post data header is: %s \n\n" %publishHeaders)
publishData = '[{"sourceId":"Temp","values":[{"ts":"2021-02-12T22:37:00Z","value":69}]}]'
#publishData = {"sourceId":"Temp","values":69}
#publishData = ujson.dumps(publishData)
publishData = ujson.dumps(publishData)
print(publishData)
#print(type(publishData))
publishResponse = requests.post(dataUrl,headers=publishHeaders,data=publishData)
print(publishResponse.json())
