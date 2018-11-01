ps -ef | grep trafficRecordServer | awk '$3==1{print $2}' | xargs kill
