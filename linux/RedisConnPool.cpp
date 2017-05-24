#include "RedisConnPool.h"
#include "log.h"
//#include "common/Singleton.h"
#include <boost/crc.hpp>
#include <cstring>
#include <boost/property_tree/ptree.hpp>		// for json parse
#include <boost/property_tree/json_parser.hpp>	// for json parse
#include <boost/foreach.hpp>					// for json parse
#include <boost/algorithm/string.hpp>			// for split
#include <boost/lexical_cast.hpp>				// for int to char*

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <iostream>
#include <cstdlib> //std::system
#include <boost/thread/thread_time.hpp> 

#include "boost/generator_iterator.hpp"

using namespace boost::interprocess;

using namespace RedisSDK;

RedisConnPool RedisSDK::g_RedisConnPool_;
ConnHolder RedisSDK::g_ConnHolder_;

RedisConnPool::RedisConnPool()
{
}

RedisConnPool::~RedisConnPool()
{
}

void RedisConnPool::iniConnCollection(int maxCont)
{
	std::map<int, IpPort>::iterator it = redis_ipportlist_.begin();
	for (; it != redis_ipportlist_.end(); it++)
	{
		IpPort ipport = it->second;
		int slotvalue = it->first;

		std::vector<RedisContextPtr> avec;
		for (int i = 0; i < maxCont; i++)
		{
			RedisContextPtr conn = newConnectPtr(ipport.ip, ipport.port);
			avec.push_back(conn);
		}
		connCollPool_.insert(pair<int, std::vector<RedisContextPtr> >(slotvalue, avec));
	}
}

RedisContextPtr RedisConnPool::getConn(int slot, int index)
{
	//G_LOG_DEBUG "getConn slot = %d, index = %d",slot,index LOG_END
	RedisContextPtr ret = connCollPool_[slot][index];
	//G_LOG_DEBUG "getConn end" LOG_END
	return ret;
}

bool RedisConnPool::initConfigInfo( const string& appid, 
									const string& ip, 
									int port, 
									int maxCont )
{
	ramdomNum = new boost::uniform_int<>(0, maxCont - 1);
	setInfo(appid, ip, port);
	configInfo_ = readConfigInfo();
	if (configInfo_.empty())
	{
		G_LOG_ERROR "redis config info error !" LOG_END
		return false;
	}

	if (!jsonParse(configInfo_))
	{
		G_LOG_ERROR "redis config json parse error !" LOG_END
		return false;
	}

	iniSharedMemoryMap(maxCont);
	iniConnCollection(maxCont);
	maxCont_ = maxCont;
	return true;
}

RedisContextPtr RedisConnPool::newConnectPtr( string ip, int port )
{
	struct timeval tv = {3,0};
	redisContext* cont = redisConnectWithTimeout(ip.c_str(), port, tv);
	RedisContextPtr contPtr(cont, RedisContextCloser());
	if (NULL == cont)
	{
		G_LOG_ERROR "redis connect failure null! " LOG_END
		return contPtr;
	}
	else if (cont->err)
	{
		G_LOG_ERROR "redis connect err:[%s]! ",cont->errstr LOG_END
		return contPtr;
	}
	if(redisSetTimeout(cont,tv) != REDIS_OK)
	{
		G_LOG_ERROR "redis set timeout err:[%s]! ",cont->errstr LOG_END
		return contPtr;
	}
	RedisReplyPtr replyPtr(static_cast<redisReply*>(redisCommand(cont,"INFO Replication")), 
						   RedisReplyCloser());   //使用shared_ptr智能指针的定制删除器，节省代码、易读
	if (NULL == replyPtr)
	{
		G_LOG_ERROR "Execut command1 failure " LOG_END
	}
	if (replyPtr->type == REDIS_REPLY_ERROR)
	{
		G_LOG_ERROR "Failed to execute command[%s] ",replyPtr->str LOG_END
	}
	return contPtr;
}

bool RedisConnPool::jsonParse(const string& config)
{
	using namespace boost::property_tree;
	using namespace boost;

	std::stringstream ssconfig(config);
	ptree pt;
	try
	{
		read_json(ssconfig, pt);
	}
	catch (ptree_error & e)
	{
		G_LOG_ERROR "read_json ocur ptree_error : %s ", e.what() LOG_END
		return false;
	}

	try
	{
		ptree sentinel_array = pt.get_child("sentinels");
		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, sentinel_array)  
		{
			string sentinelInfo = v.second.data(); // format is like: 10.10.2.53:26379
			G_LOG_DEBUG "sentinelInfo = %s ", sentinelInfo.c_str() LOG_END
			std::vector<std::string> strs;
			char* sentinelInfo_c_str = const_cast<char*>(sentinelInfo.c_str());
			boost::split(strs, sentinelInfo_c_str, boost::is_any_of(":"));
			sentinel_ipportlist_.push_back(IpPort(strs[0], atoi(strs[1].c_str())));
		}
		ptree master_array = pt.get_child("masters");
		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, master_array)  
		{
			string masterInfo = v.second.data(); // like master01-6369:0=32767
			G_LOG_DEBUG "masterInfo = %s ", masterInfo.c_str() LOG_END
			std::vector<std::string> strs;
			char* masterInfo_c_str = const_cast<char*>(masterInfo.c_str());
			boost::split(strs, masterInfo_c_str, boost::is_any_of(":="));
			master_infolist_.push_back(MasterInfo(strs[0], 
												  atoi(strs[1].c_str()),
												  atoi(strs[2].c_str())
												  ));
		}
		getMastersIpPort(sentinel_ipportlist_, master_infolist_);
	}
	catch (ptree_error & e)
	{
		G_LOG_ERROR "ptree_error : %s ", e.what() LOG_END
		return false;
	}

	return true;
}

void RedisConnPool::getMastersIpPort( std::vector<IpPort>& sipl, std::vector<MasterInfo>& mil )
{
	bool masterInfoReaded = false;
	for (std::vector<IpPort>::iterator aSentinel = sipl.begin();
		 aSentinel != sipl.end();
		 aSentinel++)
	{
		RedisContextPtr conPtr = newConnectPtr(aSentinel->ip, aSentinel->port);
		if (conPtr == NULL)
		{
			continue;
		}
		for (std::vector<MasterInfo>::iterator aMaster = mil.begin();
			aMaster != mil.end();
			aMaster++)
		{
			RedisReplyPtr replyPtr(redisCmd(conPtr.get(), 
											"SENTINEL get-master-addr-by-name %s", 
											aMaster->name.c_str() ), 
								   RedisReplyCloser());
			if (replyPtr == NULL)
			{
				break;
			}
			
			if (replyPtr->type == REDIS_REPLY_NIL)
			{
				break;
			}
			G_LOG_DEBUG "master = %s,  ip = %s,  port = %s ",
				aMaster->name.c_str(), 
				replyPtr->element[0]->str,
				replyPtr->element[1]->str LOG_END
			redis_ipportlist_.insert(pair<int, IpPort>(aMaster->slot, 
													   IpPort(replyPtr->element[0]->str,
															  atoi(replyPtr->element[1]->str))
														));
			masterInfoReaded = true;
		}

		if(masterInfoReaded)
		{
			break;
		}
	}
	if (!masterInfoReaded)
	{
		G_LOG_ERROR "read master info fail ! " LOG_END
	}
}

void RedisConnPool::reload()
{
	string reloadstr = readConfigInfo();
	if (configInfo_ == reloadstr)
	{
		bool isConnAvl = true;
		std::map<int, IpPort> amap = g_RedisConnPool_.getSlotMap();
		std::map<int, IpPort>::iterator it = amap.begin();
		for (; it != amap.end(); it++)
		{
			int slotvalue = it->first;
// 			int index = 0;
// 			RedisContextPtr cont = getConn(slotvalue, index); // from shared memory;
			RedisContextPtr cont = newConnectPtr(it->second.ip, it->second.port);

			RedisReplyPtr replyPtr(static_cast<redisReply*>(redisCommand(cont.get(),"INFO Replication")), 
				RedisReplyCloser());

			if (NULL == replyPtr)
			{
				G_LOG_ERROR "redis reload check is null ! " LOG_END
				isConnAvl = false;
				break;
			}
			if (replyPtr->type == REDIS_REPLY_ERROR)
			{
				G_LOG_ERROR "redis reload check is error ! " LOG_END
				isConnAvl = false;
				break;
			}
		}
		g_ConnHolder_.ConnHolderRelease();
		if (isConnAvl)
		{
			return; // not been changed
		}
		G_LOG_INFO "redis conn reload ! " LOG_END
	}

	connCollPool_.clear();
	redis_ipportlist_.clear();
	sentinel_ipportlist_.clear();
	master_infolist_.clear();

	if (!jsonParse(configInfo_))
	{
		return;
	}

	iniSharedMemoryMap(maxCont_);
	iniConnCollection(maxCont_);

	delete ramdomNum;
	ramdomNum = new boost::uniform_int<>(0, maxCont_ - 1);
}

void RedisConnPool::setInfo( const string& appid, const string& ip, int port )
{
	appid_	= appid;
	ip_		= ip;
	port_	= port;
}

string RedisConnPool::readConfigInfo()
{
	RedisContextPtr aRedisConn = newConnectPtr(ip_, port_);
	if (aRedisConn.get() == NULL)
	{
		G_LOG_ERROR "Redis Conn is null !" LOG_END
		return "";
	}

	RedisReplyPtr reply(redisCmd(aRedisConn.get(),"GET %s", appid_.c_str()), 
		RedisReplyCloser());

	if(reply == NULL)
	{
		G_LOG_ERROR "redis reply is null !" LOG_END
		return "";
	}

	if (reply->type == REDIS_REPLY_NIL)
	{
		G_LOG_ERROR "cant find appid : %s !", appid_.c_str() LOG_END
		return "";
	}

	string ret = reply->str;
	return ret;
}

// ----------------------  Global function   -------------------

redisReply* RedisSDK::redisCmd(redisContext* cont, const char *format, ...)
{
	va_list ap;
	redisReply* reply = NULL;
	if (NULL == cont)
	{
		G_LOG_ERROR "Redis context is null" LOG_END
			return NULL;
	}
	va_start(ap,format);  //处理可变参数
	reply = (redisReply *)redisvCommand(cont,format,ap);
	va_end(ap);
	if (NULL == reply || reply->type == REDIS_REPLY_ERROR)
	{
		G_LOG_ERROR "RedisCmd reply->type = REDIS_REPLY_ERROR" LOG_END
	}

	return reply;
}

void RedisSDK::getConnFromKey(const char* key, 
								RedisContextPtr& conn,
								string& keyWithSlot)
{
	boost::crc_32_type crc32;
	crc32.process_bytes(key,strlen(key));
	int crc_value = crc32.checksum() % REDIS_SLOT_MOD;

	std::map<int, IpPort> amap = g_RedisConnPool_.getSlotMap();
	std::map<int, IpPort>::iterator lower_it = amap.lower_bound(crc_value);
	int slotvalue = lower_it->first;
	//G_LOG_WARN "slot ===  %d", slotvalue LOG_END;

	keyWithSlot.append(boost::lexical_cast<std::string>(crc_value));
	keyWithSlot.append(":");
	keyWithSlot.append(key);

	int index;
	conn = getConn(slotvalue, index); // from shared memory; 
	if (NULL == conn.get())
	{
		G_LOG_ERROR "Redis context is null, key= %s",key LOG_END;
	}
}

RedisReplyPtr RedisSDK::redisGet( const char* key, 
								  const char* field )
{
	string keyWithSlot;
	RedisContextPtr conn;
	getConnFromKey(key, conn, keyWithSlot);
	G_LOG_DEBUG "redisGet key = %s, field = %s", keyWithSlot.c_str(), field LOG_END;
	RedisReplyPtr ret =  RedisReplyPtr(RedisSDK::redisCmd(conn.get(), "HGET %s %s", keyWithSlot.c_str(), field), 
											RedisReplyCloser());
	return ret;
}

RedisReplyPtr RedisSDK::redisSetInt( const char* key, 
									 const char* field, 
									 int ivalue )
{
	string keyWithSlot;
	RedisContextPtr conn;
	getConnFromKey(key, conn, keyWithSlot);
	G_LOG_DEBUG "redisSetInt key = %s, field = %s, ivalue = %d", keyWithSlot.c_str(), field, ivalue LOG_END;
	RedisReplyPtr ret =  RedisReplyPtr(RedisSDK::redisCmd(conn.get(), "HSET %s %s %d",keyWithSlot.c_str(), field, ivalue), 
											RedisReplyCloser());

	return ret;
}

RedisReplyPtr RedisSDK::redisSetStr( const char* key, 
									 const char* field, 
									 const char* svalue )
{
	string keyWithSlot;
	RedisContextPtr conn;
	getConnFromKey(key, conn, keyWithSlot);
	G_LOG_DEBUG "redisSetStr key = %s, field = %s, svalue = %s", keyWithSlot.c_str(), field, svalue LOG_END;
	RedisReplyPtr ret =  RedisReplyPtr(RedisSDK::redisCmd(conn.get(), "HSET %s %s %s",keyWithSlot.c_str(), field, svalue), 
											RedisReplyCloser());

	return ret;
}

RedisReplyPtr RedisSDK::redisDel( const char* key)
{
	string keyWithSlot;
	RedisContextPtr conn;
	getConnFromKey(key, conn, keyWithSlot);
	G_LOG_DEBUG "redisDel key = %s", keyWithSlot.c_str() LOG_END;
	RedisReplyPtr ret =  RedisReplyPtr(RedisSDK::redisCmd(conn.get(), "DEL %s",keyWithSlot.c_str()), 
											RedisReplyCloser());
	return ret;
}

RedisReplyPtr RedisSDK::redisExpire( const char* key, 
									 const char* time )
{
	string keyWithSlot;
	RedisContextPtr conn;
	getConnFromKey(key, conn, keyWithSlot);
	G_LOG_DEBUG "redisExpire key = %s, time = %s", keyWithSlot.c_str(), time LOG_END;
	RedisReplyPtr ret =  RedisReplyPtr(RedisSDK::redisCmd(conn.get(), "EXPIRE %s %s",keyWithSlot.c_str(), time), 
						 RedisReplyCloser());
	return ret;
}

RedisContextPtr RedisSDK::getConn(int slotvalue, int& index)
{
	int maxcont = g_RedisConnPool_.getMaxCont();
	RedisContextPtr aPtr = g_ConnHolder_.getConn(slotvalue);
	if (aPtr.get() != NULL)
	{
		return aPtr;
	}
	index = -1;

	SharedMemoryName smn = g_RedisConnPool_.getSharedMemoryName();
	//string lockName = "mtx";

	named_mutex named_mtx(open_or_create, smn.LockName.c_str());
	bool getLock = named_mtx.timed_lock(boost::get_system_time() + boost::posix_time::milliseconds(3));

	if (!getLock)
	{
		G_LOG_ERROR "shared memory get lock fail!  = %s", smn.LockName.c_str() LOG_END;
		IpPort ipport = g_RedisConnPool_.getIpPort(slotvalue);
		RedisContextPtr ret = g_RedisConnPool_.newConnectPtr(ipport.ip, ipport.port);
		return ret;
	}

	// check shared memroy mark and get connection
	typedef boost::interprocess::allocator<int, managed_shared_memory::segment_manager> vecAllocator;
	typedef boost::interprocess::vector<int, vecAllocator> vec;
	managed_shared_memory segment(open_or_create, smn.SpaceName.c_str(), 65536);
	vecAllocator vectorallocator(segment.get_segment_manager());

	vec *myvector = segment.find<vec>(smn.SpaceVectorName.c_str()).first;
	index = g_RedisConnPool_.getRamdomNum();
	//G_LOG_DEBUG "get random =  %d", index LOG_END;
	if ((*myvector)[index] == 0)
	{
		(*myvector)[index] = 1;
	}
	else
	{
		int newindex  = index + 1;

		for (; ; newindex++)
		{
			if (newindex >= maxcont)
			{
				newindex = 0;
			}
			if (newindex == index)
			{
				newindex = -1;
				break;  // all be used,  create one
			}

			if ((*myvector)[newindex] == 0)   // dont repeat yourself!
			{
				(*myvector)[newindex] = 1;
				break;
			}
		}
		index = newindex;
	}
	
	unlock:
	named_mtx.unlock();

	//Singleton<GlobalData>::instance().imax = index;
	//Singleton<GlobalData>::instance().slotvalue = slotvalue;
	
	RedisContextPtr ret;
	if (index < 0)
	{
 		G_LOG_WARN "no enough connection, new one ... " LOG_END
		std::map<int, IpPort> amap = g_RedisConnPool_.getSlotMap();
		std::map<int, IpPort>::iterator it = amap.begin();
		for (; it != amap.end(); it++)
		{
			IpPort ipport = g_RedisConnPool_.getIpPort(it->first);
			RedisContextPtr acon = g_RedisConnPool_.newConnectPtr(ipport.ip, ipport.port);
			g_ConnHolder_.addPtr(it->first, index, acon);
		}
	}
	else
	{
		//ret = g_RedisConnPool_.getConn(slotvalue, index);
		std::map<int, IpPort> amap = g_RedisConnPool_.getSlotMap();
		std::map<int, IpPort>::iterator it = amap.begin();
		for (; it != amap.end(); it++)
		{
			RedisContextPtr acon = g_RedisConnPool_.getConn(it->first, index);
			g_ConnHolder_.addPtr(it->first, index, acon);
		}
	}
	G_LOG_DEBUG "get index  slotvalue = %d, index = %d", slotvalue, index LOG_END

	//aholder.addPtr(slotvalue, index, ret);
	ret = g_ConnHolder_.getConn(slotvalue);
	return ret;
}

void RedisSDK::releaseConn(int index)
{
	if (index < 0)
	{
		return;
	}
	SharedMemoryName smn = g_RedisConnPool_.getSharedMemoryName();

	//string lockName = "mtx";
	named_mutex named_mtx(open_or_create, smn.LockName.c_str());
	named_mtx.lock();

	// 在共享内存标记可用
	typedef boost::interprocess::allocator<int, managed_shared_memory::segment_manager> vecAllocator;
	typedef boost::interprocess::vector<int, vecAllocator> vec;
	managed_shared_memory segment(open_or_create, smn.SpaceName.c_str(), 65536);
	vec *myvector = segment.find<vec>(smn.SpaceVectorName.c_str()).first;

	if ((*myvector)[index] == 1)
	{
		(*myvector)[index] = 0;
	}

unlock:
	named_mtx.unlock();
}

void RedisSDK::iniSharedMemoryMap(int maxCont)
{
	SharedMemoryName smn = g_RedisConnPool_.getSharedMemoryName();
	boost::interprocess::permissions permUnrestricted;
	permUnrestricted.set_unrestricted();

	shared_memory_object::remove(smn.SpaceName.c_str());
	typedef boost::interprocess::allocator<int, managed_shared_memory::segment_manager> vecAllocator;
	typedef boost::interprocess::vector<int, vecAllocator> vec;
	managed_shared_memory segment(open_or_create, smn.SpaceName.c_str(), 65536, 0, permUnrestricted);
	vecAllocator vectorallocator(segment.get_segment_manager());

	vec *myvec = segment.construct<vec>(smn.SpaceVectorName.c_str())(vectorallocator);
	for (int i = 0; i < maxCont; i++)
	{
		myvec->push_back(0);
	}
	//string lockName = "mtx";
	named_mutex::remove(smn.LockName.c_str());
	named_mutex named_mtx(open_or_create, smn.LockName.c_str(), permUnrestricted);
}
