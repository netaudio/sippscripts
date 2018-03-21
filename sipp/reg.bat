#cnt = %1
#if $cnt = ""
#	$cnt = 1000
#fi
sipp -sf reg.xml -inf uac.csv  -i 192.168.27.1  -m %1 192.168.27.149
sipp -sf reg.xml -inf uas.csv  -i 192.168.27.1  -m %1 192.168.27.149
