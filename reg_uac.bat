#!/bin/bash


sipp -trace_err -sf reg_uac.xml -r 1 -rp 1 -inf reg_uac.csv -p 6090 -i 192.168.101.113  -m 1 -l 10000 192.168.101.168:5060
