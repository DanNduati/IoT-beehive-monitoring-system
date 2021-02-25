import requests,json

accessUrl ="https://dev-42331gx2.us.auth0.com/oauth/token"
dataUrl ="https://anabi-smartsystems.com/api/data"
accessHeader = {"content-type": 'application/json'}
accessData ='{"client_id":"Goi7MZiF0YCI4YAykw5gbd5Xky8s4JUj","client_secret":"v6sEKlu8WE6OVGpSXamVLEM8W9_IKSwg6eeaQJmY0qdCIrLfhfQP1j-GRDSOEL6K","audience":"sams-dwh-web-api","grant_type":"client_credentials"}'

#get access token
accessResponse = requests.post(accessUrl,data=accessData,headers=accessHeader)
results = accessResponse.json()
#print status code
print(accessResponse)
#print(accessResponse.text)
accessToken = results['access_token']
print("The acces token is: %s\n\n" %accessToken)

##post pseudo data to anabi
postHeader={"content-type":'application/json',"Authorization":'Bearer '+str(accessToken)+''}
print("The post header is: %s \n\n"%postHeader)
postData = '[{"sourceId":"temp-Goi7MZiF0YCI4YAykw5gbd5Xky8s4JUj","values":[{"ts":"2021-02-25T08:40:00Z","value":69}]}]'
publishResponse = requests.post(dataUrl,data=postData,headers=postHeader)
#print status code
print(publishResponse)
print(publishResponse.json())
