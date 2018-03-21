#cnt = %1
#if $cnt = ""
#	$cnt = 1000
#fi
sipp -sf reg.xml -inf uac.csv  -i 192.168.101.104  -m %1 192.168.101.8
sipp -sf reg.xml -inf uas.csv  -i 192.168.101.104  -m %1 192.168.101.8
