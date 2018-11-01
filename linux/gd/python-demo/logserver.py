import redis
import signal
import datetime
import os
import logging
import config
import multiprocessing
import json
import time
import sys
import MySQLdb
from daemon import Daemon

kill_now = False

class GracefulKiller:
    #kill_now = False
    def __init__(self):
        signal.signal(signal.SIGINT, self.exit_gracefully)
        signal.signal(signal.SIGTERM, self.exit_gracefully)
    
    def exit_gracefully(self,signum, frame):
        global kill_now
        kill_now = True

def setLogger():  
    logger = logging.getLogger('mylogger')  
    logger.setLevel(logging.DEBUG)  
  
    fh = logging.FileHandler(os.path.join(os.getcwd(), 'log.txt'))  
    fh.setLevel(logging.DEBUG)  
                
    ch = logging.StreamHandler()  
    ch.setLevel(logging.DEBUG)  
          
    formatter = logging.Formatter('%(asctime)s - %(module)s.%(funcName)s.%(lineno)d - %(levelname)s - %(message)s')  
    fh.setFormatter(formatter)  
    ch.setFormatter(formatter)  
            
    logger.addHandler(fh)  
    logger.addHandler(ch)  
          
    logger.info('hello world, i\'m log helper in python, may i help you')  
    return logger  

def workThread():
    #r = redis.Redis(host=config.redis['host'], port=config.redis['port'], decode_responses=True)
    r = redis.Redis(host=config.redis['host'], port=config.redis['port'], decode_responses=True, password=config.redis['password'])
    db = MySQLdb.connect(host=config.mysql['host'],port=config.mysql['port'], user=config.mysql['usr'], passwd=config.mysql['pwd'], db=config.mysql['db'], charset='utf8' )
    cursor = db.cursor()
    logger = setLogger()
    try:
        global kill_now
        while not kill_now:
        #starttime = datetime.datetime.now()
        #for num in range(0,10000):
            amessage = None
            try:
                amessage = r.rpop("queue")
            except:
                logger.exception("Exception Logged")
                continue

            #arrayList = r.lrange("queue",0,1)
            #amessage = arrayList[0]
            if amessage is None:
                #print "rpop is none"
                time.sleep(config.conf['sleep_sc'])
                continue

            akv = json.loads(amessage)

            post_args_kv = akv["post_args"]
            post_args_kv_sn = "" if post_args_kv.get("sn", 0) == 0 else post_args_kv["sn"]
            post_args_kv_uid = "" if post_args_kv.get("user_id", 0) == 0 else post_args_kv["user_id"]
            post_args_kv_type = "" if post_args_kv.get("type_str", 0) == 0 else post_args_kv["type_str"]
            post_args_kv_param = "" if post_args_kv.get("param", 0) == 0 else post_args_kv["param"]
            post_args_kv_respon = "" if post_args_kv.get("respon", 0) == 0 else post_args_kv["respon"]
            post_args_kv_iqa = "" if post_args_kv.get("in_queue_at", 0) == 0 else post_args_kv["in_queue_at"]
            post_args_kv_headers = json.dumps(akv["headers"])

            write_table = config.mysql['table']
            respon_len = len(post_args_kv_respon)
            if respon_len > 65535:
                logger.exception("respon len: " + str(respon_len))
                write_table = config.mysql['table_big']
            else:
                write_table = config.mysql['table']
            
            if post_args_kv_param.find("'") > -1:
                post_args_kv_param = post_args_kv_param.replace("'", "\\'")
            if post_args_kv_headers.find("'") > -1:
                post_args_kv_headers = post_args_kv_headers.replace("'", "\\'")

            sql = "INSERT INTO `%s` (`sn`, `user_id`, `type_str`, `header`, `param`,\
                    `respon`, `in_queue_at`, `created_at`, `updated_at`)  VALUES ('%s', '%s', '%s',\
                    '%s', '%s', '%s', '%s', '%s', '%s')" % (write_table, post_args_kv_sn,\
                    post_args_kv_uid, post_args_kv_type, post_args_kv_headers, post_args_kv_param,\
                    post_args_kv_respon, post_args_kv_iqa, post_args_kv_iqa, post_args_kv_iqa)
                    
            #print sql

            try:
                cursor.execute(sql)
                db.commit()

                #print "lkf commit"
            except:
                db = MySQLdb.connect(host=config.mysql['host'],port=config.mysql['port'], user=config.mysql['usr'], passwd=config.mysql['pwd'], db=config.mysql['db'], charset='utf8' )
                cursor = db.cursor()
                try:
                    cursor.execute(sql)
                    db.commit()
                except:
                    logger.exception("retry execute sql fail: "+sql)

                logger.exception("reconnect success")
                #print "lkf rollback" 
                    
        #endtime = datetime.datetime.now()
        #logger.exception((endtime - starttime).seconds)

        print "lkf close"
        db.close()


    except:
        logger.exception("Exception Logged")


class pantalaimon(Daemon):
    def run(self):

        killer = GracefulKiller()
        threads = []
        for i in xrange(config.conf['thread_num']):
            p = multiprocessing.Process(target=workThread, args=())
            p.start()
            threads.append(p)

        for t in threads:
            t.join()
        print "all thread done"



if __name__ == '__main__':
    if sys.argv[1] == "normal":
        pineMarten = pantalaimon(Daemon)
        pineMarten.run()
    elif sys.argv[1] == "deamon":
        pineMarten = pantalaimon(config.conf['pid_path'])
        pineMarten.start()
