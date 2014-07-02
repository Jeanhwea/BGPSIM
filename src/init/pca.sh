# sysctl net.ipv4.ip_forward=1
ifconfig eth0 192.168.2.2 netmask 255.255.255.0
ifconfig eth1 192.168.3.1 netmask 255.255.255.0
ifconfig eth0 promisc 
ifconfig eth1 promisc

