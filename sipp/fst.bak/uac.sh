#!/bin/bash
# Author:fairylly
#执行脚本时，未输入参数，提示：please input call number!，并退出
if [[ $# -eq 0 ]]
then
        echo "please input call number!"
        exit 1
fi
#m变量使用命令行传递的第一个位置参数
m=$1
 
./sipp -i 192.168.101.22 -sf uac.xml -inf uac_my.csv 192.168.101.8:5060 -l $m -trace_screen -trace_err -p 12345 -m $m -aa
