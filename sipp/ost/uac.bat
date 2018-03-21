
rem sipp -aa -i 192.168.27.1 -sf uac_t1.xml -inf uac.csv 192.168.27.149:5060 -l 1 -m 1 -p 12346   -trace_msg -trace_screen -trace_err

sipp.exe -i 192.168.101.106  192.168.101.8:5060 -inf uac.csv  -sf uac_t2.xml  -m 1 -l 1 -trace_err -trace_screen -trace_msg
