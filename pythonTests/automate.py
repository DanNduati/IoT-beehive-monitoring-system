import requests,json,datetime

#client creds
clientId = "YF9GDOGOXdcVM23UI56mm0wtXv9TsCMj"
clientSecret = "5FHyIyVQW6O536waXzUGoTI69dE2gdgKBcem4CnOUyepSJth8TRH56HsNzQH3JSt"

accessUrl ="https://anabi-smartsystems.com/api/token"
dataUrl ="https://anabi-smartsystems.com/api/data"
accessHeader = {"content-type": 'application/json'}
accessData ='{"client_id":"'+clientId+'","client_secret":"'+clientSecret+'","audience":"sams-dwh-web-api","grant_type":"client_credentials"}'

#get access token
accessResponse = requests.post(accessUrl,data=accessData,headers=accessHeader)
results = accessResponse.json()
#print status code
print(accessResponse)
print(accessResponse.text)

accessToken = results['access_token']
print("The access token is: %s\n\n" %accessToken)

timestamp = datetime.datetime.now().replace(microsecond=00).isoformat()
##post pseudo temperature and audio data to anabi
postHeader={"content-type":'application/json',"Authorization":'Bearer '+str(accessToken)+''}
#print("The post header is: %s\n\n"%postHeaderUpostData='[{"sourceId":"temp","values":[{"ts":"2021-05-10T22:45:03Z","value":40}]},{"sourceId":"audio","values":[{"ts":"2021-05-10T22:45:03Z","value":-253015}]}]'
#main board data
postData='[{"sourceId":"mainTemp","values":[{"ts":"'+str(timestamp)+'Z","value":69}]},{"sourceId":"mainAudio","values":[{"ts":"'+str(timestamp)+'Z","values":[0.6,0.7,0.8,0.6]}]}]'
print(postData)
publishResponse = requests.post(dataUrl,data=postData,headers=postHeader)
#print status code
print(publishResponse)
print(publishResponse.json())
#aux board data
postData='[{"sourceId":"auxHum","values":[{"ts":"'+str(timestamp)+'Z","value":69}]},{"sourceId":"auxTemp","values":[{"ts":"'+str(timestamp)+'Z","value":69}]},{"sourceId":"auxWeight","values":[{"ts":"'+str(timestamp)+'Z","value":69.69}]}]'
print(postData)
publishResponse = requests.post(dataUrl,data=postData,headers=postHeader)
print(publishResponse)
print(publishResponse.json())
