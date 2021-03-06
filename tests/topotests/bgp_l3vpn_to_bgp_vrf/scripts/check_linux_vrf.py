from lutil import luCommand

rtrs = ['r1', 'r3', 'r4', 'ce1', 'ce2', 'ce3', 'ce4']
for rtr in rtrs:
    luCommand(rtr,'sysctl net.ipv4.tcp_l3mdev_accept',' = \d*','none','')
    found = luLast()
    luCommand(rtr,'ss -aep',':bgp','pass','IPv4:bgp, l3mdev%s' % found.group(0))
    luCommand(rtr,'ss -aep',':.:bgp','pass','IPv6:bgp')

rtrs = ['r1', 'r3', 'r4']
for rtr in rtrs:
    luCommand(rtr, 'ip link show type vrf {}-cust1'.format(rtr),'cust1: .*UP,LOWER_UP','pass','VRF cust1 up')
    luCommand(rtr, 'ip add show vrf {}-cust1'.format(rtr),'r..eth4: .*UP,LOWER_UP.* 192.168','pass','VRF cust1 IP config')
    luCommand(rtr, 'ip route show vrf {}-cust1'.format(rtr),'192.168...0/24 dev r.-eth','pass','VRF cust1 interface route')
luCommand('r4', 'ip link show type vrf r4-cust2','cust2: .*UP,LOWER_UP','pass','VRF cust2 up')
luCommand('r4', 'ip add show vrf r4-cust2','r..eth5.*UP,LOWER_UP.* 192.168','pass','VRF cust1 IP config')
luCommand(rtr, 'ip route show vrf r4-cust2'.format(rtr),'192.168...0/24 dev r.-eth','pass','VRF cust2 interface route')
rtrs = ['ce1', 'ce2', 'ce3']
for rtr in rtrs:
    luCommand(rtr, 'ip route show','192.168...0/24 dev ce.-eth0','pass','CE interface route')
    luCommand(rtr,'ping 192.168.1.1 -c 1',' 0. packet loss','wait','CE->PE ping')
luCommand('ce4', 'ip link show type vrf ce4-cust2','cust2: .*UP,LOWER_UP','pass','VRF cust2 up')
luCommand('ce4', 'ip route show vrf ce4-cust2','192.168...0/24 dev ce.-eth0','pass','CE interface route')
luCommand('ce4','ping 192.168.2.1 -c 1 -I ce4-cust2',' 0. packet loss','wait','CE4->PE4 ping')
