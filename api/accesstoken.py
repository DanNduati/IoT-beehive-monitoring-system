import requests,json

accessUrl ="https://anabi-smartsystems.com/api/token"
accessHeader = {"content-type": 'application/json'}
accessData='{"client_id":"CsntdHilPaKkZ9L1TXHAYsvaKiCTQ6FQ","client_secret":"TCp9Vgpv9UaBLfjTdhQmuEiQZOJ51BNnlphqIGuWH7ao5825PixX5fk1rcArqrXx","audience":"sams-dwh-web-api","grant_type":"client_credentials"}'

#get access token
accessResponse = requests.post(accessUrl,data=accessData,headers=accessHeader)
results = accessResponse.json()
#print status code
print(accessResponse)
print(accessResponse.text)

accessToken = results['access_token']
print("The acces token is: %s\n\n" %accessToken)


