
#include "RedisConnPool.h"

int main()
{
	// 1. 初始化
	RedisSDK::SharedMemoryName smn;
	smn.LockName = "yourLockName";				// 自定义， 单机内不重复
	smn.SpaceVectorName = "yourVectorName";		// 自定义， SpaceName内不重复
	smn.SpaceName = "yourSpaceName";			// 自定义， 单机内不重复
	RedisSDK::g_RedisConnPool_.setSharedMemoryName(smn);
	RedisSDK::g_RedisConnPool_.initConfigInfo("yourAppId", "yourRedisIp", 
		1234/*yourRedisPort*/, 1/*yourRedisMaxConnectionNumber*/);

	// 2. 使用redis写入数据， 全局函数
	RedisSDK::RedisReplyPtr reply;
	reply = RedisSDK::redisExpire("yourRedisKey","60"/*Expire time(second)*/);
	reply = RedisSDK::redisDel("yourRedisKey");
	reply = RedisSDK::redisSetStr("yourRedisKey","yourHashKey","yourHashKeyValue");
	reply = RedisSDK::redisSetInt("yourRedisKey","yourHashKey", 1/*yourHashKeyValue*/);

	// 3. 使用获取，并验证reply数据
	reply = RedisSDK::redisGet("yourRedisKey", "yourHashKey");
	if (reply == NULL){ return 0; }
	if (reply->type == REDIS_REPLY_NIL){ return 0; }
	if (reply->str == NULL){ return 0; }

	// 4. reply->str 就是获取得到的数据。
	int ivalue = atoi(reply->str);

	// 5. 假如想使用主动释放redis连接 与 共享内存解锁标记
	RedisSDK::g_ConnHolder_.ConnHolderRelease();

	// demo end
	return 0;
}

// 6. reload函数
void reload_thread()
{
	// 假设这个函数被一个线程定时调用
	// reload函数可以检测相应的redis配置是否被改变， 是否需要重新初始化连接池
	RedisSDK::g_RedisConnPool_.reload();
}
