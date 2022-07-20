How to access the server from your laptops
1. you should install ssh (ignore if you have)
2. copy summer-CHITACC.pem in your local machine (don't share with anyone)

Run this command:
chmod 600 summer-CHITACC.pem
ssh-add summer-CHITACC.pem
ssh cc@IP_ADDRESS

You will be successfully login into your respective servers.
