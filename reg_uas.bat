#!/bin/bash


sipp -trace_err -sf reg_uas.xml -r 1 -rp 1 -inf reg_uas.csv -p 6080 -i 192.168.101.89  -m 1 -l 10000 192.168.101.168:5060
