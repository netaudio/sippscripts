


#sipp -i 192.168.101.22  -sf branchs.xml -inf uas.csv 192.168.101.8:5060 -l 10 -trace_msg -trace_screen -trace_err -p 12346 -m 10 -aa
#sipp -i 192.168.101.22  -sf uas_stand.xml -inf uas.csv 192.168.101.8:5060 -l 10 -trace_msg -trace_screen -trace_err -p 12346 -m 10 -aa

#sipp -i 192.168.27.129  -sf uas.xml -inf uas.csv 192.168.27.149:5060 -l 10 -trace_msg -trace_screen -trace_err -p 12346 -m 10 -aa

#pause

sipp -i 192.168.27.149  -sf uas.xml -inf uas.csv 192.168.27.149:5060  -m 100 -aa