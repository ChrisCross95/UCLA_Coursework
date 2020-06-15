#!
if [[ "$OSTYPE" == "linux-gnu" ]]; then
        serv_ipaddr=$(hostname -I)
elif [[ "$OSTYPE" == "darwin"* ]]; then
        serv_ipaddr=$(ipconfig getifaddr en0)
fi 
echo '#define SERV_ADDR "'$serv_ipaddr'"' >> netdefs.h
echo "Server Address: $serv_ipaddr"
