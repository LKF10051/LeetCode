


tag：python微服务， 多线程， 不丢数据退出， deamon运行， 写mysql


usage:
python logserver.py deamon

kill gracefully: (Do not use kill -9 unless necessary)
ps -ef | grep python
kill sub_thread_pid1 sub_thread_pid2 ...

usage after kill:
rm ./pid.pid
python logserver.py deamon
