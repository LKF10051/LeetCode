conf = dict(
    batch_size = 50,
    thread_num = 8,
    pid_path = "/root/pythonLogWriter/pid.pid",
    sleep_sc = 0.1,
)
redis = dict(
    host =      "10.0.0.0",
    port =      1111,
    password =  "password@123",
)
mysql = dict(
    host =  "1.1.1.1",
    port =  3306,
    usr =   "usr",
    pwd =   "pwd",
    db =    "log_server",
    table = "tb_piao_gen_log",
    table_big = "tb_piao_gen_big_log",
)
