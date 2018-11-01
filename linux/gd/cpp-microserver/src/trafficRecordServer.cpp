/*
 * @file	trafficRecordServer.cpp
 * @author	LKF
 * @date	2018-10-8  15:07
 * @brief	
 * @Copyright (C) 2018, by gd
 */
#include <iostream>
#include <string>
#include <fstream>
#include <regex>

#include <hiredis.h>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/log/common.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>  // sigset.async_wait(&SignalHandler);
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

#include <boost/property_tree/ptree.hpp>		// for json parse
#include <boost/property_tree/json_parser.hpp>	// for json parse
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>			// for split

#include "boost/date_time/gregorian/gregorian.hpp" // today_date

#include <boost/locale.hpp>

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace po = boost::program_options;
namespace conv = boost::locale::conv;

using namespace boost::property_tree;

using namespace std;
using namespace boost;

struct G_Config
{
    string      redisIp;
    int         redisPort;
    string      redisPwd;
    int         redisTimeoutSecond;
    int         redisTimeoutMicrosecond;
    string      redisQueueName;
    string      logPath;
    string      writeTo;
    int         sleepMilliseconds;
};

/*extern*/ G_Config g_config_;
bool isRunning = true;

void ReadConfig(char* configPath)
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("redisIp",                 po::value<string>(&g_config_.redisIp),              "redisIp")
        ("redisPort",               po::value<int>(&g_config_.redisPort),               "redisPort")
        ("redisPwd",                po::value<string>(&g_config_.redisPwd),             "redisPwd")
        ("redisTimeoutSecond",      po::value<int>(&g_config_.redisTimeoutSecond),      "redisTimeoutSecond")
        ("redisTimeoutMicrosecond", po::value<int>(&g_config_.redisTimeoutMicrosecond), "redisTimeoutMicrosecond")
        ("redisQueueName",          po::value<string>(&g_config_.redisQueueName),       "redisQueueName")
        ("logPath",                 po::value<string>(&g_config_.logPath),              "logPath")
        ("writeTo",                 po::value<string>(&g_config_.writeTo),              "writeTo")
        ("sleepMilliseconds",       po::value<int>(&g_config_.sleepMilliseconds),       "sleepMilliseconds")
        ;

    std::ifstream settings_file(configPath);
    po::variables_map vm;
    po::store(po::parse_config_file(settings_file, desc), vm);
    po::notify(vm);
}

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(test_lg, src::logger_mt)

class TrafficRecordHandler
{
public:

    static void handleYueTong(string& fullfilename)
    {
        string filename;
        vector<string> strs2;
        boost::split(strs2, fullfilename, boost::is_any_of("/"));
        filename = strs2[strs2.size() - 1];

        ifstream file(fullfilename.c_str());
        string processed = "[";
        string aline;

        boost::gregorian::date current_date(boost::gregorian::day_clock::local_day());
        string today_date = to_iso_extended_string(current_date);

        while (std::getline(file, aline))
        {
            try
            {
                vector<string> strs;
                boost::split(strs, aline, boost::is_any_of("|"));
                if (strs.size() < 8) // this value should equal 8
                {
                    BOOST_LOG(test_lg::get()) << "warn YueTong strs.size() < 8 ";
                    continue;
                }

                string a_little_part;
                string total_fee_high = strs[6].size() >= 3 ? strs[6].substr(0, strs[6].size() - 2) : "0";
                string total_fee_low = strs[6].size() >= 2 ? strs[6].substr(strs[6].size() - 2, 2) :  strs[6];

                a_little_part = str( boost::format("{\"exchange_id\":\"%s\",\"card_no\":\"4401%s\",\"entry_time\":\"%s\",\
\"entry_station\":\"%s\",\"exit_time\":\"%s\",\"exit_station\":\"%s\",\"total_fee\":%s.%s,\"clear_time\":\"%s\"},") 
                % strs[0] /*exchange_id*/
                % strs[1] /*card_no*/
                % strs[2] /*entry_time*/
                % strs[3] /*entry_station*/
                % strs[4] /*exit_time*/
                % strs[5] /*exit_station*/
                % total_fee_high /*total_fee*/
                % total_fee_low /*total_fee*/
                % today_date /*clear_time*/
                    );

                processed.append(a_little_part);
            }
            catch (std::exception& e)
            {
                BOOST_LOG(test_lg::get()) << "std::exception& e    " << e.what();
            }
            catch (...)
            {
                BOOST_LOG(test_lg::get()) << "exception ... ";
            }
        }

        processed.replace(processed.size() - 1, 1, "]"); // replace ',' to ']'

        string writeTo = g_config_.writeTo;
        BOOST_LOG(test_lg::get()) << writeTo;
        writeTo.append("/").append(filename.replace(filename.size() - 4, 4/*size of '.dat' */, "_Exchanged.dat"));
        std::ofstream wfile(writeTo.c_str());
        wfile << processed;
        wfile.close();
        BOOST_LOG(test_lg::get()) << "done YueTong "<<writeTo;
    }

    static void handleJSuTong(string& fullfilename)
    {
        string filename;
        vector<string> strs2;
        boost::split(strs2, fullfilename, boost::is_any_of("/"));
        filename = strs2[strs2.size() - 1];

        ifstream file(fullfilename.c_str());
        string processed = "[";
        string aline;

        while (std::getline(file, aline))
        {
            try
            {
                //string JSuTong =  "32011801231600001366|!03CA|!谷得龙|!吉J3Q315|!0|!500|!2018/9/25 7:49:07|!G2安亭-安亭入;G2江桥-嘉松公路出|!20180925085810|!10000|!上海|!32011801231600001366110100320180925074907|!";

                vector<string> strs_odd_even;
                boost::split(strs_odd_even, aline, boost::is_any_of("!|"));
                if (strs_odd_even.size() < 25) // this value should equal 25
                {
                    BOOST_LOG(test_lg::get()) << "warn JSuTong strs_odd_even.size() < 25 ";
                    continue;
                }
                vector<string> strs;
                for (size_t i = 0; i < strs_odd_even.size(); i += 2)
                {
                    strs.push_back(strs_odd_even[i]);
                }

                string a_little_part;
                string total_fee_high = strs[5].size() >= 3 ? strs[5].substr(0, strs[5].size() - 2) : "0";
                string total_fee_low = strs[5].size() >= 2 ? strs[5].substr(strs[5].size() - 2, 2) :  strs[5];

                string surplus_money_high = strs[9].size() >= 3 ? strs[9].substr(0, strs[9].size() - 2) : "0";
                string surplus_money_low = strs[9].size() >= 2 ? strs[9].substr(strs[9].size() - 2, 2) :  strs[9];

                // vector<string> strs_toll_station;
                // boost::split(strs_toll_station,  strs[7], boost::is_any_of(";至"));
                // if (strs_toll_station.size() == 3)
                // {
                //     strs_toll_station[1] = strs_toll_station[2]; // if use chinese split, strs3[1] will be nullstring
                // }

                vector<string> strs_toll_station;
                regex regex(";|至");
                sregex_token_iterator iterator(strs[7].begin(), strs[7].end(), regex, -1);
                sregex_token_iterator end;
                for ( ; iterator != end; ++iterator)
                {
                    strs_toll_station.push_back(*iterator);
                }

                if (strs_toll_station.size() < 2)
                {
                    BOOST_LOG(test_lg::get()) << "warning: can't parse exit_station";
                    BOOST_LOG(test_lg::get()) << strs[7];
                    strs_toll_station.push_back("");
                }

                a_little_part = str( boost::format("{\"card_no\":\"%s\",\"exchange_id\":\"%s-%s\",\"user_name\":\"%s\",\"plate_no\":\
\"%s\",\"plate_color\":\"%s\",\"total_fee\":%s.%s,\"exit_time\":\"%s\",\"exit_network\":\"\",\"entry_network\":\"\",\"entry_time\":\"\",\"entry_station\":\"%s\",\"exit_station\":\
\"%s\",\"clear_time\":\"%s\",\"surplus_money\":%s.%s,\"province\":\"%s\"},") 
                % strs[0] /*card_no*/
                % strs[1] /*exchange_id*/
                % strs[11] /*exchange_id*/
                % strs[2] /*user_name*/
                % strs[3] /*plate_no*/
                % strs[4] /*plate_color*/
                % total_fee_high  /*total_fee*/
                % total_fee_low /*total_fee*/
                % strs[6] /*exit_time*/
                % strs_toll_station[0] /*entry_station*/
                % strs_toll_station[1] /*exit_station*/
                % strs[8] /*clear_time*/
                % surplus_money_high  /*surplus_money*/
                % surplus_money_low /*surplus_money*/
                % strs[10] /*province*/
                );

                processed.append(a_little_part);
            }
            catch (std::exception& e)
            {
                BOOST_LOG(test_lg::get()) << "std::exception& e    " << e.what();
            }
            catch (...)
            {
                BOOST_LOG(test_lg::get()) << "exception ... ";
            }
        }

        processed.replace(processed.size() - 1, 1, "]"); // replace ',' to ']'

        string writeTo = g_config_.writeTo;
        BOOST_LOG(test_lg::get()) << writeTo;
        writeTo.append("/").append(filename.replace(filename.size() - 4, 4/*size of '.dat' */, "_Exchanged.dat"));
        std::ofstream wfile(writeTo.c_str());
        wfile << processed;
        wfile.close();
        BOOST_LOG(test_lg::get()) << "done JSuTong "<<writeTo;
    }

    static void handleBSuTong(string& fullfilename)
    {
        string filename;
        vector<string> strs2;
        boost::split(strs2, fullfilename, boost::is_any_of("/"));
        filename = strs2[strs2.size() - 1];

        ifstream file(fullfilename.c_str());
        string processed = "[";
        string aline;

        while (std::getline(file, aline))
        {
            try
            {
                //string BSuTong = "000be4e5bb9a,366735,11012002360375,1201230001085554,20180924,000307,20180924,195924,1425,镇江营进京入|夏村双向出,637026433";

                aline = conv::between(aline, "UTF-8", "GBK");
                vector<string> strs;
                boost::split(strs, aline, boost::is_any_of(",|"));
                if (strs.size() < 12) // this value should equal 12
                {
                    BOOST_LOG(test_lg::get()) << "warn BSuTong strs.size() < 12 ";
                    continue;
                }

                string a_little_part;
                string total_fee_high = strs[8].size() >= 3 ? strs[8].substr(0, strs[8].size() - 2) : "0";
                string total_fee_low = strs[8].size() >= 2 ? strs[8].substr(strs[8].size() - 2, 2) :  strs[8];

                a_little_part = str( boost::format("{\"issuer_account\":\"%s\",\"card_no\":\"1101%s\",\"exit_time\":\"%s-%s-%s %s:%s:%s\",\"clear_time\"\
:\"%s\",\"total_fee\":%s.%s,\"exit_station\":\"%s\",\"entry_station\":\"%s\",\"exchange_id\":\"%s-%s-%s\"},") 
                % strs[2] /*issuer_account*/
                % strs[3] /*card_no*/
                % strs[6].substr(0, 4) /*exit_time*/
                % strs[6].substr(4, 2) /*exit_time*/
                % strs[6].substr(6, 2) /*exit_time*/
                % strs[7].substr(0, 2) /*exit_time*/
                % strs[7].substr(2, 2) /*exit_time*/
                % strs[7].substr(4, 2) /*exit_time*/
                % strs[6] /*clear_time*/
                % total_fee_high /*total_fee*/ 
                % total_fee_low /*total_fee*/
                % strs[10] /*exit_station*/
                % strs[9] /*entry_station*/
                % strs[0] /*exchange_id*/
                % strs[1] /*exchange_id*/
                % strs[11] );/*exchange_id*/

                processed.append(a_little_part);
            }
            catch (std::exception& e)
            {
                BOOST_LOG(test_lg::get()) << "std::exception& e    " << e.what();
            }
            catch (...)
            {
                BOOST_LOG(test_lg::get()) << "exception ... ";
            }
        }

        processed.replace(processed.size() - 1, 1, "]"); // replace ',' to ']'

        string writeTo = g_config_.writeTo;
        BOOST_LOG(test_lg::get()) << writeTo;
        writeTo.append("/").append(filename.replace(filename.size() - 4, 4/*size of '.dat' */, "_Exchanged.dat"));
        std::ofstream wfile(writeTo.c_str());
        wfile << processed;
        wfile.close();
        BOOST_LOG(test_lg::get()) << "done BSuTong "<<writeTo;
    }

    static void handleJiLinTong(string& fullfilename)
    {
        string filename;
        vector<string> strs2;
        boost::split(strs2, fullfilename, boost::is_any_of("/"));
        filename = strs2[strs2.size() - 1];
        vector<string> strs3;
        boost::split(strs3, filename, boost::is_any_of("_"));
        vector<string> strs4;
        boost::split(strs4, strs3[3], boost::is_any_of("."));
        string clear_time = strs4[0];

        string processed = "[";
        string aline;
        ptree pt;
        try
        {
            read_json(fullfilename, pt);
        }
        catch (ptree_error & e)
        {
            BOOST_LOG(test_lg::get()) << "read_json ocur ptree_error :" << e.what();
        }

        BOOST_FOREACH (ptree::value_type &record, pt)
        {
            try
            {
                std::string trans_no = record.second.get<std::string>("trans_no");
                std::string entry_time = record.second.get<std::string>("entry_time");
                int plate_color = record.second.get<int>("plate_color");
                int package_no = record.second.get<int>("package_no");
                std::string agenttype_no = record.second.get<std::string>("agenttype_no");
                int transid = record.second.get<int>("transid");
                std::string agent_no = record.second.get<std::string>("agent_no");
                std::string traded_at = record.second.get<std::string>("trade_at");
                int account_status = record.second.get<int>("account_status");
                long post_time = record.second.get<long>("post_time");
                std::string xh = record.second.get<std::string>("xh");
                std::string exit_time = record.second.get<std::string>("exit_time");
                std::string exchange_id = record.second.get<std::string>("exchange_id");
                long account_time = record.second.get<long>("account_time");
                std::string card_no = record.second.get<std::string>("card_no");
                std::string exit_station = record.second.get<std::string>("exit_station");
                std::string plate_no = record.second.get<std::string>("plate_no");
                std::string entry_station = record.second.get<std::string>("entry_station");
                std::string total_fee = record.second.get<std::string>("total_fee");
                std::string provider_id = record.second.get<std::string>("provider_id");
                std::string rec_create_date = record.second.get<std::string>("rec_create_date");
                int post_status = record.second.get<int>("post_status");
                int rn = record.second.get<int>("RN");

                std::string a_little_part = str( boost::format("{\"trans_no\":\"%s\",\"clear_time\":\"%s\",\"entry_time\":\"%s\",\
\"plate_color\":%d,\"package_no\":%d,\"agenttype_no\":\"%s\",\"transid\":%d,\"agent_no\":\"%s\",\
\"traded_at\":\"%s\",\"account_status\":%d,\"post_time\":%ld,\"xh\":\"%s\",\"exit_time\":\"%s\",\
\"exchange_id\":\"%s\",\"account_time\":%ld,\"card_no\":\"%s\",\"exit_station\":\"%s\",\
\"plate_no\":\"%s\",\"entry_station\":\"%s\",\"total_fee\":\"%s\",\"provider_id\":\"%s\",\
\"rec_create_date\":%ld,\"post_status\":%d,\"RN\":%d},")
                % trans_no
                % clear_time
                % entry_time
                % plate_color
                % package_no
                % agenttype_no
                % transid
                % agent_no
                % traded_at
                % account_status
                % post_time
                % xh
                % exit_time
                % exchange_id
                % account_time
                % card_no
                % exit_station
                % plate_no
                % entry_station
                % total_fee
                % provider_id
                % rec_create_date
                % post_status
                % rn);

                BOOST_LOG(test_lg::get()) << a_little_part;

                processed.append(a_little_part);
            }
            catch (std::exception& e)
            {
                BOOST_LOG(test_lg::get()) << "std::exception& e    " << e.what();
            }
            catch (...)
            {
                BOOST_LOG(test_lg::get()) << "exception ... ";
            }
        }

        processed.replace(processed.size() - 1, 1, "]"); // replace ',' to ']'
        string writeTo = g_config_.writeTo;
        writeTo.append("/").append(filename.replace(filename.size() - 4, 4/*size of '.dat' */, "_Exchanged.dat"));
        std::ofstream wfile(writeTo.c_str());
        wfile << processed;
        wfile.close();
        BOOST_LOG(test_lg::get()) << "done JiLinTong " << writeTo;
    }

    static void handleLuTong(string& fullfilename)
    {
        string filename;
        vector<string> strs2;
        boost::split(strs2, fullfilename, boost::is_any_of("/"));
        filename = strs2[strs2.size() - 1];

        vector<string> strs3;
        boost::split(strs3, filename, boost::is_any_of("_"));
        vector<string> strs4;
        boost::split(strs4, strs3[3], boost::is_any_of("."));
        string clear_time = strs4[0];

        ifstream file(fullfilename.c_str());
        string processed = "[";
        string aline;

        while (std::getline(file, aline))
        {
            try
            {
                //string LuTong = "43267526|37011801230201462455|2018-09-22 15:30:46|1.96|蓝口|2018-09-22 15:32:48|柳城|2018-09-22 15:32:48|";

                aline = conv::between(aline, "UTF-8", "GBK");
                vector<string> strs;
                boost::split(strs, aline, boost::is_any_of("|"));
                if (strs.size() < 8) // this value should equal 8
                {
                    BOOST_LOG(test_lg::get()) << "warn BSuTong strs.size() < 8 ";
                    continue;
                }

                string a_little_part = str( boost::format("{\"exchange_id\":\"%s\",\"card_no\":\"%s\",\"entry_time\":\"%s\",\
\"total_fee\":\"%s\",\"entry_station\":\"%s\",\"exit_time\":\"%s\",\"exit_station\":\"%s\",\"clear_time\":\"%s\"},") 
                % strs[0] /*exchange_id*/
                % strs[1] /*card_no*/
                % strs[2] /*entry_time*/
                % strs[3] /*total_fee*/
                % strs[4] /*entry_station*/
                % strs[5] /*exit_time*/
                % strs[6] /*exit_station*/
                % clear_time /*clear_time*/
                );

                processed.append(a_little_part);
            }
            catch (std::exception& e)
            {
                BOOST_LOG(test_lg::get()) << "std::exception& e    " << e.what();
            }
            catch (...)
            {
                BOOST_LOG(test_lg::get()) << "exception ... ";
            }
        }

        processed.replace(processed.size() - 1, 1, "]"); // replace ',' to ']'

        string writeTo = g_config_.writeTo;
        writeTo.append("/").append(filename.replace(filename.size() - 4, 4/*size of '.dat' */, "_Exchanged.dat"));
        std::ofstream wfile(writeTo.c_str());
        wfile << processed;
        wfile.close();
        BOOST_LOG(test_lg::get()) << "done LuTong " << writeTo;
    }
};

void TrafficRecordHandle(const int n)
{
    redisContext* c;
    redisReply* reply;

    struct timeval timeout = { g_config_.redisTimeoutSecond, g_config_.redisTimeoutMicrosecond }; // { 1, 500000 } means 1.5 seconds
    c = redisConnectWithTimeout(g_config_.redisIp.c_str(), g_config_.redisPort, timeout);
    if (c == NULL || c->err) 
    {
        if (c) 
        {
            BOOST_LOG(test_lg::get()) << "Connection error: " << c->errstr;
            redisFree(c);
        } 
        else 
        {
            BOOST_LOG(test_lg::get()) << "Connection error: can't allocate redis context";
        }
        isRunning = false;
    }

    reply= static_cast<redisReply*>(redisCommand(c, "AUTH %s", g_config_.redisPwd.c_str()));
    freeReplyObject(reply);

    BOOST_LOG(test_lg::get()) << "begin while loop ... ";
    while (isRunning)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(g_config_.sleepMilliseconds));

        reply = static_cast<redisReply*>(redisCommand(c, "RPOP %s", g_config_.redisQueueName.c_str()));

        if (reply->str == NULL)
        {
            BOOST_LOG(test_lg::get()) << "null...";
            freeReplyObject(reply);
            continue;
        }
        else
        {
            BOOST_LOG(test_lg::get()) << reply->str;
            // freeReplyObject(reply);
        }

        //string reply->str = "{\"issuerCode\": \"JSuTong\", \"changeType\": 1, \"filename\": \"/var/www/webdav/leijianhui/lkfDev/trafficRecordServer/bin/ISSUER_CONSUME_ACCOUNT_2018-09-25.dat\", \"fileType\": 2, \"date\": \"2018-09-25\", \"version\": 0}";

        std::stringstream ssconfig(reply->str);
        freeReplyObject(reply);
        ptree pt;
        try
        {
            read_json(ssconfig, pt);
        }
        catch (ptree_error & e)
        {
            BOOST_LOG(test_lg::get()) << "read_json ocur ptree_error :" << e.what();
            continue;
        }

        string fullfilename;
        string issuerCode;
        try
        {
            issuerCode = pt.get<string>("issuerCode") ;
            //int changeType = pt.get<int>("changeType") ;
            fullfilename = pt.get<string>("filename") ;
            //BOOST_LOG(test_lg::get()) << issuerCode ;
            //BOOST_LOG(test_lg::get()) << changeType ;
        }
        catch (ptree_error & e)
        {
            BOOST_LOG(test_lg::get()) << "ptree_error :" << e.what();
        }

        if (issuerCode == "JSuTong")
        {
            TrafficRecordHandler::handleJSuTong(fullfilename);
        }
        else if (issuerCode == "BSuTong")
        {
            TrafficRecordHandler::handleBSuTong(fullfilename);
        }
        else if (issuerCode == "yuetong")
        {
            TrafficRecordHandler::handleYueTong(fullfilename);
        }
        else if (issuerCode == "JiLinTong")
        {
            TrafficRecordHandler::handleJiLinTong(fullfilename);
        }
        else if (issuerCode == "LuTong")
        {
            TrafficRecordHandler::handleLuTong(fullfilename);
        }
    }
}

void InitLog()
{
    // Open a rotating text file
    boost::shared_ptr< std::ostream > strm(new std::ofstream(g_config_.logPath.append("/trafficRecordServer.log").c_str()));
    if (!strm->good())
        throw std::runtime_error("Failed to open a text log file");

    // Create a text file sink
    boost::shared_ptr< sinks::synchronous_sink< sinks::text_ostream_backend > > sink(
        new sinks::synchronous_sink< sinks::text_ostream_backend >);

    sink->locked_backend()->auto_flush(true); // print log immediately
    sink->locked_backend()->add_stream(strm);
    sink->set_formatter
        (
        expr::format("%1% %2% - %3%")
        //% expr::attr< unsigned int >("RecordID")
        % expr::attr< boost::posix_time::ptime >("TimeStamp")
        % expr::attr< attrs::current_thread_id::value_type >("ThreadID")
        % expr::smessage
        );

    // Add it to the core
    logging::core::get()->add_sink(sink);

    // Add some attributes too
    logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
    logging::core::get()->add_global_attribute("RecordID", attrs::counter< unsigned int >());
    logging::core::get()->add_global_attribute("ThreadID", attrs::current_thread_id());
}

void SignalHandler(const boost::system::error_code& err, int signal)
{
    switch (signal) {
    case SIGINT:
        break;
    case SIGTERM:
        isRunning = false;
        break;
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "usage: trafficRecordServer /path/to/trafficRecordServer.conf" << endl;
        return 0;
    }

    ReadConfig(argv[1]);

    try
    {
        // fork daemon
        // https://www.boost.org/doc/libs/1_47_0/doc/html/boost_asio/example/fork/daemon.cpp

        boost::asio::io_service io_service;

        asio::signal_set sigset(io_service, SIGTERM);
        sigset.async_wait(&SignalHandler);
        io_service.notify_fork(boost::asio::io_service::fork_prepare);
        if (pid_t pid = fork())
        {
            if (pid > 0)
            {
                exit(0);
            }
            else
            {
                cout << "First fork failed" << endl;
                return 1;
            }
        }

        setsid();
        chdir("/");
        umask(0);

        // A second fork ensures the process cannot acquire a controlling terminal.
        if (pid_t pid = fork())
        {
            if (pid > 0)
            {
                exit(0);
            }
            else
            {
                cout << "Second fork failed" << endl;
                return 1;
            }
        }

        close(0);
        close(1);
        close(2);

        io_service.notify_fork(boost::asio::io_service::fork_child);

        InitLog();
        logging::add_common_attributes();
        using namespace logging::trivial;

        BOOST_LOG(test_lg::get()) << "A trace severity message";

        thread_group group;

        for(int num=0; num<1; num++)
        {
            group.create_thread(bind(&TrafficRecordHandle,num));
        }
        BOOST_LOG(test_lg::get()) << "Daemon started";
        io_service.run();
        group.join_all();
        BOOST_LOG(test_lg::get()) << "Daemon stopped";
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
