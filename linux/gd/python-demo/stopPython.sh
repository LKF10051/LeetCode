PID=$(</root/pythonLogWriter/pid.pid)
ps -ef | grep python | grep  "$PID" | awk '$3!=1{print $2}' | xargs kill
