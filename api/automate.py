import requests,json

accessUrl ="https://dev-42331gx2.us.auth0.com/oauth/token"
dataUrl ="https://anabi-smartsystems.com/api/data"
accessHeader = {"content-type": 'application/json'}
accessData ='{"client_id":"EptrWV0x17H0AlOoTvz06akdZ5imV4U5","client_secret":"M5c70lGDh8g9Zp2cItFbQvIJnpGtJqX7tTwq3MC7wqWkR_9OD4j7iilAW0vVgzo9","audience":"sams-dwh-web-api","grant_type":"client_credentials"}'

#get access token
accessResponse = requests.post(accessUrl,data=accessData,headers=accessHeader)
results = accessResponse.json()
#print status code
print(accessResponse)
print(accessResponse.text)

accessToken = results['access_token']
print("The acces token is: %s\n\n" %accessToken)


##post pseudo data to anabi
postHeader={"content-type":'application/json',"Authorization":'Bearer '+str(accessToken)+''}
print("The post header is: %s \n\n"%postHeader)
postData='[{"sourceId":"ds18b20-0-EptrWV0x17H0AlOoTvz06akdZ5imV4U5","values":[{"ts":"2021-03-30T16:26:03Z","value":33}]}]'
print("The post data is %s \n\n" %(postData))
publishResponse = requests.post(dataUrl,data=postData,headers=postHeader)
#print status code
print(publishResponse)
print(publishResponse.json())
