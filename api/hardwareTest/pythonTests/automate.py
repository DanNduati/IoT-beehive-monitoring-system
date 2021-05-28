import requests,json,datetime

#client creds
clientId = "1Jfkgsido75tN5WIPt70dREFdWOxSFhu"
clientSecret = "eOtaVX1elukj1TPrCfgIYeImxdAdeCa4Bn1PlSoGMYOiAQPuIQEIFgkAmVcWgnDl"

accessUrl ="https://anabi-smartsystems.com/api/token"
dataUrl ="https://anabi-smartsystems.com/api/data"
accessHeader = {"content-type": 'application/json'}
accessData ='{"client_id":"1Jfkgsido75tN5WIPt70dREFdWOxSFhu","client_secret":"eOtaVX1elukj1TPrCfgIYeImxdAdeCa4Bn1PlSoGMYOiAQPuIQEIFgkAmVcWgnDl","audience":"sams-dwh-web-api","grant_type":"client_credentials"}'

#get access token
accessResponse = requests.post(accessUrl,data=accessData,headers=accessHeader)
results = accessResponse.json()
#print status code
print(accessResponse)
print(accessResponse.text)

accessToken = results['access_token']
print("The acces token is: %s\n\n" %accessToken)

timestamp = datetime.datetime.now().isoformat()
print("The timestamp is: ",timestamp)
##post pseudo temperature and audio data to anabi
postHeader={"content-type":'application/json',"Authorization":'Bearer '+str(accessToken)+''}
#print("The post header is: %s\n\n"%postHeaderUpostData='[{"sourceId":"temp","values":[{"ts":"2021-05-10T22:45:03Z","value":40}]},{"sourceId":"audio","values":[{"ts":"2021-05-10T22:45:03Z","value":-253015}]}]'
postData='[{"sourceId":"temp","values":[{"ts":"'+str(timestamp)+'","value":69}]},{"sourceId":"audio","values":[{"ts":"'+str(timestamp)+'","value":35015}]}]'
print(postData)
publishResponse = requests.post(dataUrl,data=postData,headers=postHeader)
#print status code
print(publishResponse)
print(publishResponse.json())
