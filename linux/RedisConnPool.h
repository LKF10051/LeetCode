/*
 * @file	RedisConnPool.h
 * @author	LKF
 * @date	2015-7-24  11:40
 * @brief	redis连接池。更多细节补充， 见：基础模块设计/RedisConnPool/
 *			RedisConnPool设计细节补充.docx
 * @Copyright (C) 2015, by LKF
 */
#ifndef __RedisConnPool_H__
#define __RedisConnPool_H__

#include <deque>
#include <map>
#include <string>
#include <vector>
#include "hiredis/hiredis.h"
#include "log.h"
#include "boost/shared_ptr.hpp"
#include "boost/random.hpp"

const int REDIS_SLOT_MOD = 65536;	// 分为65536个槽值

using namespace std;

class redisContext;
namespace RedisSDK
{

#define D_CHECK_REPLY(reply) 		if (reply == NULL)\
	{\
		G_LOG_ERROR "reply =  NULL !" LOG_END\
		return false;\
	}\
	if (reply->type == REDIS_REPLY_NIL)\
	{\
		G_LOG_ERROR "reply->type =  REDIS_REPLY_NIL !" LOG_END\
		return false;\
	}\
	if (reply->str == NULL)\
	{\
		G_LOG_ERROR "reply->str == null !" LOG_END\
		return false;\
	}

// ip和端口 数据结构体
struct IpPort
{
	IpPort(string cip, int cport)
		: ip(cip),
		  port(cport)
	{}
	IpPort()
	{}
	string	ip;
	int		port;
};

struct SharedMemoryName
{
	string SpaceName;		// 共享内存空间名
	string SpaceVectorName;	// 共享内存Vector名字
	string LockName;		// 共享内存锁名
};

// redis Master信息集
struct MasterInfo
{
	MasterInfo(string cname, int cdb, int cslot)
		: name(cname),
		  db(cdb),
		  slot(cslot)
	{}
	string	name;
	int		db;
	int		slot;
};

// 用于关闭 redisContext* 的仿函数
class RedisContextCloser
{
public:
	void operator()(redisContext* rcontext)
	{
		//G_LOG_DEBUG " important release" LOG_END
		redisFree(rcontext);
	}
};
typedef boost::shared_ptr<redisContext> RedisContextPtr;

// redis 连接数据集
struct Pack
{
	int index;
	RedisContextPtr aptr;
};

// 以索引号释放redis连接 
void releaseConn(int index);

// redis连接持有者
class ConnHolder
{
public:
	ConnHolder(){}
	~ConnHolder()
	{ ConnHolderRelease(); }

	// 按slot值 取出redis连接
	RedisContextPtr getConn(int slotvalue)
	{
		std::map<int, Pack>::iterator pos;
		pos = amap_.find(slotvalue);
		if (pos == amap_.end())
		{
			RedisContextPtr nullret;
			return nullret;
		}

		return amap_[slotvalue].aptr;
	}

	// 添加redis连接， 当连接在共享内存被标记为此进程使用时， 调用此接口添加进入ConnHolder
	// 当ConnHolder释放时候， 在共享内存标记此连接可用， 以便其他进程复用
	void addPtr(int slotvalue, int index,RedisContextPtr aptr)
	{
		Pack apack;
		apack.index = index;
		apack.aptr = aptr;
		amap_.insert(std::pair<int, Pack>(slotvalue, apack));
	}

	// 释放全部持有的连接
	void ConnHolderRelease()
	{
		if (amap_.size() == 0)
		{
			return;
		}
		std::map<int, Pack>::iterator pos = amap_.begin();
		releaseConn(pos->second.index);
		amap_.clear();
	}
private:
	std::map<int/*slotvalue*/, Pack> amap_;
};

extern ConnHolder g_ConnHolder_;			// 全局对象

// 用于关闭 redisReply* 的仿函数
class RedisReplyCloser
{
public:
	void operator()(redisReply* reply)
	{
		freeReplyObject(reply);
	}
};
typedef boost::shared_ptr<redisReply> RedisReplyPtr;

// redis 连接池
// 共享内存布局（与redis 连接池持有的连接集合布局相同）：
// 
// slot value    shared memroy layout
// 			----------------------
//  65536/n		| 0 | 1 | ...|maxCont|
// 			----------------------
//  65536*2/n		|                    |
// 			----------------------
//  ...			|                    |
// 			----------------------
//  65536		|                    |
// 			----------------------
// 其中n = redis工作进程数量
class RedisConnPool 
{
public:
	typedef map<int, redisContext*> RedisConnMap;
	RedisConnPool();
	~RedisConnPool();

	// 重新加载连接池数据
	void reload();
	// 初始化配置数据
	bool initConfigInfo(const string& appid,	// redis应用编号
						const string& ip, 
						int port, 
						int maxCont);			// 连接池最大连接数

	// 获取连接， slot是槽值， 0 <= index <= maxCont
	RedisContextPtr getConn(int slot, int index);
	// 新建立一个redis连接
	RedisContextPtr newConnectPtr(string ip, int port);

	// 按槽值获取具体的redis进程ip端口数据
	IpPort getIpPort(int slot)
	{
		IpPort ret =  redis_ipportlist_[slot];
		return ret;
	}
	// 获取槽值映射表
	map<int, IpPort> getSlotMap()
	{ return redis_ipportlist_; }
	// 获取最大连接数
	int getMaxCont()
	{ return maxCont_; }
	// 获取一个随机数
	int getRamdomNum()
	{
		rng.seed(time(NULL) + getpid());
		int index = (*ramdomNum)(rng);
		return index;
	}
	// 设置共享内存名字
	void setSharedMemoryName( const SharedMemoryName& smn )
	{ shared_memory_name_ = smn; }
	// 获取共享内存名字
	SharedMemoryName getSharedMemoryName()
	{ return shared_memory_name_ ; }

private:
	// 以最大连接数为边界， 初始化连接池所有连接
	void iniConnCollection(int maxCont);
	// 从redis配置服务器读取配置信息
	string readConfigInfo();
	void setInfo(const string& appid, const string& ip, int port);
	// 解析json格式数据
	bool jsonParse(const string& config);

	// 从redis配置进程和redis工作进程名字，获取具体ip端口
	void getMastersIpPort(vector<IpPort>& sipl, vector<MasterInfo>& mil);

private:
	map<int/*max slot*/, vector<RedisContextPtr> >	connCollPool_;	// 持有的连接集合
	map<int/*max slot*/, IpPort>		redis_ipportlist_;			// 槽值与ip端口映射表

	string				appid_;					// 应用名
	string				ip_;
	int					port_;
	string				configInfo_;
	vector<IpPort>		sentinel_ipportlist_;	// redis配置（哨兵）进程ip端口数据集
	vector<MasterInfo>	master_infolist_;		// redis工作进程数据集

	int maxCont_;								// 每个槽值的最大连接数
	SharedMemoryName	shared_memory_name_;

private:
	typedef boost::mt19937 RNGType;
	RNGType rng;
	boost::uniform_int<> *ramdomNum;
};

extern RedisConnPool g_RedisConnPool_;			// 连接池为全局对象

// ----------------------  Global function   -------------------


// redis命令通用接口
redisReply* redisCmd(redisContext* cont, const char *format, ...);

// 以key值，获取对应的redis key值连接值和槽值。
void getConnFromKey(const char* key, 
					RedisContextPtr& conn,
					string& keyWithSlot);

// redis Get命令封装
RedisReplyPtr redisGet(const char* key, 
					   const char* field);

// redis Set Int命令封装
RedisReplyPtr redisSetInt(const char* key, 
						  const char* field, 
						  int ivalue);

// redis Set Str命令封装
RedisReplyPtr redisSetStr(const char* key, 
						  const char* field, 
						  const char* svalue);

// redis 删除命令封装
RedisReplyPtr redisDel(const char* key);

// redis 超时命令封装
RedisReplyPtr redisExpire(const char* key, 
						  const char* time);

// 以索引号和槽值获取具体的redis连接
RedisContextPtr getConn(int slotvalue, int& index);
// 以索引号释放redis连接
void releaseConn(int index);
// 初始化共享内存数据， 此处的maxCont与redis连接池的maxCont是对应关系
void iniSharedMemoryMap(int maxCont);


} // namespace RedisSDK

#endif // RedisConnPool_H__
