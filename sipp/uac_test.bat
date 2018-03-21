
sipp -i 192.168.146.17 -sf uac.xml -inf uac.csv 192.168.146.11:5060 -l 1 -trace_msg -trace_screen -trace_err -p 12345 -m 1 -aa


#     参数解释：
#     -i:Set the local IP address for 'Contact:','Via:',and 'From:'headers.
#     Default is primary host IP address.
#     为'Contact:','Via:',和'From:'头部信息设置本地IP ，在脚本中用[local_ip]引入
#     -sf:Loads an alternate xml scenario file.
#     To learn more about XML scenario syntax, use the -sd option to dump embedded scenarios. They contain all the necessary help.
#     引入脚本文件，根据需要模拟的呼叫流程编写
#     -inf:Inject values from an external CSV file during calls into the scenarios.
#     First line of this file say whether the data is to be read in sequence (SEQUENTIAL),random (RANDOM),or user(USER)order.
#     Each line corresponds to one call and has one or more ';' delimited data fields. Those fields can be referred as [field0],[field1],... in the xml scenario file.
#     Several CSV files can be used simultaneously (syntax:-inff1.csv -inf f2.csv ...)
#     在通话场景中使用外部csv 文件引入数据；
#     文件的第一行说明了后面数据的读入方式，常用的有：顺序(SEQUENTIAL),随机(RANDOM),或用户(USER)顺序；
#     第一行对应一个通话，它们由一个或多个’;’分隔数据字段，这些字段可以在xml 场景文件中使用[field0],[field1],... 来调用；
#     多个csv 文件，可以同时使用（语法：-inf f1.csv -inf f2.csv ... ）
#     192.168.146.11:5060
#     Freeswitch 服务端IP 及freeswitch 使用的端口；
#     -l:Set the maximum number of simultaneous calls.
#     Once this limit is reached, traffic is decreased until the number of open calls goes down. Default:(3*call_duration(s)*rate).
#     设置同时呼叫的最大数目；
#     一旦达到此值，流量将被限制直到打开的通话数下降；
#     默认值：3*call_duration(s)*rate
#     -p:Set the local port number. Default is a random free port chosen by the system.
#     设置本地端口号，默认是由系统随机选择空闲的端口号；
#     -m:Stop the test and exit when 'calls' calls are processed
#     当设置的通话数完成时，停止测试并退出；
#     -aa:Enable automatic 200OK answer for INFO, UPDATE and NOTIFY messages.
#     针对INFO, UPDATE 和NOTIFY 消息，进行200OK 自动回复应答；
#     -trace_msg:
#     Displays sent and received SIP messages in <scenariofile name>_<pid>_messages.log
#     在<场景文件名>_<pid>_messages.log中显示发送和接收的SIP 消息；调试时可增加，正试性能测试时，可取消，以免日志量太大影响本地性能；
#     -trace_screen:
#     Dump statistic screens in the<scenario_name>_<pid>_screens.logfile when quitting SIPp. Useful to get a final status report in background mode (-bgoption).
#     在退出SIPp 时，把屏蔽上的统计信息写入<场景名>_<pid>_screens.log文件中；在后台模式（-bq 选项）时，这对于得到最终状态报告很有用；
#     -trace_err:
#     Trace all unexpected messages in <scenariofilename>_<pid>_errors.log.
#     跟踪所有非期望的消息到<场景文件名>_<pid>_errors.log；
#     测试时，有可能要指定发送呼叫频率，可以指定参数：
#     -r 1-rp 3000：每三秒钟发一个呼，如果不指定，默认为1秒加载10用户；
#     性能测试过程中，一直要修改测试用户数，可考虑参照如下修改uac.sh 脚本，-l 及-m 参数使用变更代替：
#     
#     