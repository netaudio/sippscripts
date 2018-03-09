#!/bin/bash


sipp -trace_err -sf reg_uac.xml -r 1 -rp 1 -inf reg_uac.csv -p 6067 -i 192.168.101.89  -m 1000 -l 10000 192.168.101.172:5060
