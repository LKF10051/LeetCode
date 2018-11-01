#include <time.h>
#include "MultiNodesDbHandle.h"
#include "servant/Application.h"
#include <ctime>
#include "sql-context.h"
#include <glib.h>
#include "sql-construction.h"


int MultiNodesDbHandle::init()
{
    TC_Config tConf;
__TRY__  
    
    tConf.parseFile( ServerConfig::BasePath + ServerConfig::ServerName + ".conf" );

    //string sHost = tConf.get( "/main/etcdb/<dbhost>", "" );
    //string sUser = tConf.get( "/main/etcdb/<dbuser>",  "" );
    //string sPassword = tConf.get( "/main/etcdb/<dbpass>", "" );
    //string sCharset = tConf.get( "/main/etcdb/<charset>", "utf8mb4" );
    //int sPort = TC_Common::strto<int>( tConf.get( "/main/etcdb/<dbport>"), "3306" );
    //
    //_sDBName = tConf.get( "/main/etcdb/<db>", "" );

    //_cntDB = tars::TC_Common::strto<int>( tConf.get( "/main/etcdb/<dbcount>", "1" ) );

    //
    //_mDBConnectionArr["etcdb"] = vector<TC_Mysql>( _cntDB );
    //
    //for( auto &Mysql : _mDBConnectionArr["etcdb"] )
    //{
    //    Mysql.init( sHost, sUser, sPassword, _sDBName, sCharset, sPort, 0 );
    //}


    vector<string> sub_domain;
    sub_domain = tConf.getDomainVector("/main/etcdb_multi_nodes");
    LOG_F <<  sub_domain.size() << endl;
    for(int i = 0; i < (int)sub_domain.size(); i++)
    {
        string sub_domain_name = "/main/etcdb_multi_nodes/" + sub_domain[i];
        LOG_F << sub_domain_name<< endl;

        string sHost =  TC_Common::trim(tConf.get(sub_domain_name + "/<dbhost>", ""));
        string sUser =  TC_Common::trim(tConf.get(sub_domain_name + "/<dbuser>", ""));
        string sPassword =  TC_Common::trim(tConf.get(sub_domain_name + "/<dbpass>", ""));
        string sCharset =  TC_Common::trim(tConf.get(sub_domain_name + "/<charset>", "utf8mb4"));
        int iPort = TC_Common::strto<int>( tConf.get(sub_domain_name + "/<dbport>", "3306" ));

        string sDBName = TC_Common::trim(tConf.get(sub_domain_name + "/<db>", ""));

        int cntDB = tars::TC_Common::strto<int>( tConf.get(sub_domain_name + "/<dbcount>", "1" ) );
        LOG_F << cntDB<< endl;
        //_mDBConnectionArr_MultiNodes[sub_domain[i]] = vector<TC_Mysql>( cntDB );
        _mDBConnectionArr_MultiNodes[sub_domain[i]] = vector<TC_Mysql>(cntDB);

        for( auto &Mysql : _mDBConnectionArr_MultiNodes[sub_domain[i]] )
        {
            Mysql.init( sHost, sUser, sPassword, sDBName, sCharset, iPort, 0 );
        }

    }
    

    return 0;
    
__CATCH__  


    exit( 0 );
    return -1;
}


int MultiNodesDbHandle::getSelectInfo(const string &sDbName,const string &sSQL, vector<map<string,string> > &vSelectInfo, string &sErrorInfo, struct sql_context_t* sql_context  )
{
    int iRet = -1;
    LOG_F << "sSQL = "<< sSQL<< endl;
    if (sSQL.find("where") != std::string::npos && 
        sSQL.find("group by") == string::npos && 
        sSQL.find("order by") == string::npos && 
        sSQL.find("sum") == string::npos && 
        sSQL.find("avg") == string::npos)
    {
        HandleSqlWhere(sSQL, sDbName, vSelectInfo, sErrorInfo);
        iRet = 0;
    }

    // see cobarSolution.ppt p52-p60
    if (sSQL.find("order by") != string::npos)
    {
        HandleSqlOrderby(sSQL, sDbName, vSelectInfo, sErrorInfo, sql_context);
        iRet = 0;
    }

    if (sSQL.find("group by") != string::npos)
    {
        HandleSqlGroupby(sSQL, sDbName, vSelectInfo, sErrorInfo, sql_context);
        iRet = 0;
    }

    if (sSQL.find("sum") != string::npos && sSQL.find("group by") == string::npos)
    {
        HandleSqlSum(sSQL, sDbName, vSelectInfo, sErrorInfo, sql_context);
        iRet = 0;
    }

    if (sSQL.find("avg") != string::npos)
    {
        HandleSqlAvg(sSQL, sDbName, vSelectInfo, sErrorInfo, sql_context);
        iRet = 0;
    }

    return iRet;
}

void MultiNodesDbHandle::HandleSqlWhere( const string & sSQL, const string & sDbName, vector<map<string,string> > &vSelectInfo, string &sErrorInfo )
{
    // 
    // sql where 处理流程： 简单合并各个nodes的结果
    // 

    LOG_F << _mDBConnectionArr_MultiNodes.size() << endl;
    for (auto db_node = _mDBConnectionArr_MultiNodes.begin(); db_node != _mDBConnectionArr_MultiNodes.end(); db_node++)
    {
        vector<tars::TC_Mysql>& anode = db_node->second;
        try
        {
            string modify_SQL = sSQL;
            string table_name = db_node->first;

            int position1 =  modify_SQL.find("from ");
            int position2 =  modify_SQL.find(" where");

            modify_SQL.replace(position1 + 5/*size of "from "*/, position2 - position1 - 5/*size of "from "*/, table_name);
            //LOG_F << modify_SQL<< endl;

            vector<map<string,string> >  avSelectInfo= getDb( sDbName, anode ).queryRecord( modify_SQL ).data();
            //LOG_F << "avSelectInfo.size()=" << avSelectInfo.size() << endl;
            vSelectInfo.insert(vSelectInfo.end(), avSelectInfo.begin(), avSelectInfo.end());
        }
        catch (std::exception const& e)
        {
            sErrorInfo=e.what();
            LOG->error() <<__FILE__<<"::"<< __FUNCTION__ << string(" catch std exception: ") + e.what() << endl;
        }
        catch (...)
        {
            sErrorInfo="unknown exception";
            LOG->error() <<__FILE__<<"::"<< __FUNCTION__<< " catch unknown exception" << endl;
        }

        LOG_F <<"|"<<__FILE__LINE__FUNCTION__<<"|"<<sDbName <<"|"<<sSQL <<endl;
    }
}

void MultiNodesDbHandle::HandleSqlOrderby( const string & sSQL, const string & sDbName, vector<map<string,string> > &vSelectInfo, string &sErrorInfo, struct sql_context_t* sql_context )
{
    // 
    // sql order by 处理流程： 详见 阿里 cobarSolution.ppt 文档p52-p60， https://github.com/alibaba/cobar/blob/master/doc/cobarSolution.ppt
    // 

    string origin_table;
    string orderby;
    int offset = 0;
    int ilimit = 0;

    if (sql_context->stmt_type == STMT_SELECT)
    {
        sql_select_t *p_sql_statement = (sql_select_t *)sql_context->sql_statement;

        sql_column_t *col = (sql_column_t *)g_ptr_array_index(p_sql_statement->orderby_clause, 0);
        sql_expr_t *expr = col->expr;
        orderby = string(expr->token_text) ;

        sql_src_item_t *src = (sql_src_item_t *)g_ptr_array_index(p_sql_statement->from_src, 0);
        origin_table =  string(src->table_name);

        ilimit = p_sql_statement->limit->num_value;
        offset = p_sql_statement->offset->num_value;
    }

    vector<vector<map<string, string> > > vSelectInfos_orderby_only;
    vector<vector<map<string, string> > > vSelectInfos_origin;

    vector<size_t> vSelectInfo_offsets;

    for (auto db_node = _mDBConnectionArr_MultiNodes.begin(); db_node != _mDBConnectionArr_MultiNodes.end(); db_node++)
    {
        vector<tars::TC_Mysql>& anode = db_node->second;
        try
        {
            string modify_SQL = sSQL;
            string table_name = db_node->first;

            int position1 =  modify_SQL.find("from ");
            int position2 =  modify_SQL.find(" order");

            modify_SQL.replace(position1 + 5/*size of "from "*/, position2 - position1 - 5/*size of "from "*/, table_name);
            //LOG_F << modify_SQL<< endl;

            vector<map<string, string> >  avSelectInfo= getDb( sDbName, anode ).queryRecord( modify_SQL ).data();
            vector<map<string, string> >  avSelectInfo_orderby_only;
            for (auto am = avSelectInfo.begin(); am != avSelectInfo.end(); am++)
            {
                map<string,string> am2;
                am2.insert(std::pair<string, string>(orderby, (*am)[orderby]));
                avSelectInfo_orderby_only.push_back(am2);
            }

            vSelectInfos_orderby_only.push_back(avSelectInfo_orderby_only);
            vSelectInfos_origin.push_back(avSelectInfo);
            vSelectInfo_offsets.push_back(0);

            LOG_F <<"|"<<__FILE__LINE__FUNCTION__<<"|"<< sDbName <<"|"<< modify_SQL <<endl;
        }
        catch (std::exception const& e)
        {
            sErrorInfo=e.what();
            LOG->error() <<__FILE__<<"::"<< __FUNCTION__ << string(" catch std exception: ") + e.what() << endl;
        }
        catch (...)
        {
            sErrorInfo="unknown exception";
            LOG->error() <<__FILE__<<"::"<< __FUNCTION__<< " catch unknown exception" << endl;
        }

        //LOG_F <<"|"<<__FILE__LINE__FUNCTION__<<"|"<< sDbName <<"|"<< modify_SQL <<endl;
    }


    try
    {
        // see cobarSolution.ppt p52-p60 again
        while(true)
        {
            if (ilimit == 0)
            {
                break;
            }

            multimap<int/*valule*/, int/*id*/> getvalue;
            for (size_t i = 0; i< vSelectInfos_orderby_only.size(); i++)
            {
                vector<map<string,string> > &avector =  vSelectInfos_orderby_only[i];

                if (avector.size() - 1 < vSelectInfo_offsets[i])
                {
                    continue;
                }

                string svalue = avector[vSelectInfo_offsets[i]].begin()->second;
                int ivalue = std::stoi(svalue); // exception
                getvalue.insert(std::pair<int, int>(ivalue, i)); // auto sort with Ascending order
            }

            auto ag = getvalue.begin();
            if (offset > 0)
            {
                vSelectInfo_offsets[ag->second]++;
                offset--;
                continue;
            }

            vSelectInfo.insert(vSelectInfo.end(), 
                vSelectInfos_origin[ag->second].begin() + vSelectInfo_offsets[ag->second], 
                vSelectInfos_origin[ag->second].begin() + vSelectInfo_offsets[ag->second] +1);
            ilimit--;
            vSelectInfo_offsets[ag->second]++;
        }
    }
    catch (std::exception const& e)
    {
        sErrorInfo=e.what();
        LOG->error() <<__FILE__<<"::"<< __FUNCTION__ << string(" catch std exception: ") + e.what() << endl;
    }
    catch (...)
    {
        sErrorInfo="unknown exception";
        LOG->error() <<__FILE__<<"::"<< __FUNCTION__<< " catch unknown exception" << endl;
    }
}

void MultiNodesDbHandle::HandleSqlGroupby( const string & sSQL, const string & sDbName, vector<map<string,string> > &vSelectInfo, string &sErrorInfo, struct sql_context_t* sql_context )
{
    // 
    // sql group by 处理流程： 详见 阿里 cobarSolution.ppt 文档p76-p82， https://github.com/alibaba/cobar/blob/master/doc/cobarSolution.ppt
    // 

    // FIXME： 尚未实现groupby 量级优化

    string origin_table;
    string orderby;
    string groupby;
    string column;
    string column_start;
    string column_end;

    if (sql_context->stmt_type == STMT_SELECT)
    {
        sql_select_t *p_sql_statement = (sql_select_t *)sql_context->sql_statement;

        sql_expr_t *grp_expr_by = (sql_expr_t *)g_ptr_array_index(p_sql_statement->groupby_clause, 0);
        groupby = string(grp_expr_by->token_text) ;
        orderby = groupby;

        sql_src_item_t *src = (sql_src_item_t *)g_ptr_array_index(p_sql_statement->from_src, 0);
        origin_table =  string(src->table_name);

        sql_expr_t *grp_expr_c = (sql_expr_t *)g_ptr_array_index(p_sql_statement->columns, 0);
        column_start = string(grp_expr_c->start);
        column_end = string(grp_expr_c->end);
        column = column_start.substr(0, column_start.size() - column_end.size());
        std::transform(column.begin(), column.end(), column.begin(), ::tolower);

        if (grp_expr_c->alias != NULL)
        {
            sql_expr_t *grp_expr_c = (sql_expr_t *)g_ptr_array_index(p_sql_statement->columns, 0);
            column = string(grp_expr_c->alias);
        }
    }

    vector<vector<map<string, string> > > vSelectInfos_orderby_only;
    vector<vector<map<string, string> > > vSelectInfos_origin;

    vector<size_t> vSelectInfo_offsets;

    for (auto db_node = _mDBConnectionArr_MultiNodes.begin(); db_node != _mDBConnectionArr_MultiNodes.end(); db_node++)
    {
        vector<tars::TC_Mysql>& anode = db_node->second;
        try
        {
            string modify_SQL = sSQL;
            string table_name = db_node->first;

            int position1 =  modify_SQL.find("from ");
            int position2 =  modify_SQL.find(" group");

            modify_SQL.replace(position1 + 5/*size of "from "*/, position2 - position1 - 5/*size of "from "*/, table_name);
            modify_SQL.replace(position1-1,0, string(", ").append(groupby) );
            modify_SQL.replace(modify_SQL.size() - 1, 0, string(" order by ").append(groupby) );
            //LOG_F <<"modify_SQL "<< modify_SQL<< endl;


            vector<map<string, string> >  avSelectInfo = getDb( sDbName, anode ).queryRecord( modify_SQL ).data();
            for (auto av = avSelectInfo.begin(); av != avSelectInfo.end(); av++)
            {
                LOG_F <<"av cont "<<endl;
                for (auto am = (*av).begin(); am != (*av).end(); am++)
                {
                    LOG_F <<"am cont "<< am->first<< " "<< am->second<<endl;
                }
            }

            vector<map<string, string> >  avSelectInfo_orderby_only;
            for (auto am = avSelectInfo.begin(); am != avSelectInfo.end(); am++)
            {
                map<string,string> am2;
                am2.insert(std::pair<string, string>(orderby, (*am)[orderby]));
                // LOG_F <<orderby "<< orderby <<" " << (*am)[orderby]<< endl;
                avSelectInfo_orderby_only.push_back(am2);
            }

            vSelectInfos_orderby_only.push_back(avSelectInfo_orderby_only);
            vSelectInfos_origin.push_back(avSelectInfo);
            vSelectInfo_offsets.push_back(0);

            LOG_F <<"|"<<__FILE__LINE__FUNCTION__<<"|"<< sDbName <<"|"<< modify_SQL <<endl;
        }
        catch (std::exception const& e)
        {
            sErrorInfo=e.what();
            LOG->error() <<__FILE__<<"::"<< __FUNCTION__ << string(" catch std exception: ") + e.what() << endl;
        }
        catch (...)
        {
            sErrorInfo="unknown exception";
            LOG->error() <<__FILE__<<"::"<< __FUNCTION__<< " catch unknown exception" << endl;
        }

    }


    try
    {
        while(true)
        {
            bool isbreak = true;
            for (size_t i = 0; i < vSelectInfo_offsets.size(); i++)
            {
                if (vSelectInfo_offsets[i] < vSelectInfos_orderby_only[i].size())
                {
                    isbreak = false; // unfinish, need to continue
                    break;
                }
            }
            if (isbreak)
            {
                break;
            }
            multimap<string/*valule*/, int/*id*/> getvalue;
            for (size_t i = 0; i< vSelectInfos_orderby_only.size(); i++)
            {
                vector<map<string,string> > &avector =  vSelectInfos_orderby_only[i];

                if (avector.size() - 1 < vSelectInfo_offsets[i])
                {
                    continue;
                }

                string svalue = avector[vSelectInfo_offsets[i]].begin()->second;
                getvalue.insert(std::pair<string, int>(svalue, i)); // auto sort with Ascending order
            }


            string first = getvalue.begin()->first;
            int same_count = 0;
            for (auto an = getvalue.begin(); an != getvalue.end(); an++)
            {
                if (an->first == first)
                {
                    same_count++;
                    vSelectInfo_offsets[an->second]++;
                }
                else
                {
                    break;
                }
            }
            LOG_F <<"same_count " << same_count <<endl;

            int isum = 0;
            string ssum;
            auto an = getvalue.begin();
            for (int i = 0;  i < same_count; i++)
            {
                map<string, string> atm;
                vector<map<string, string>>& atv = vSelectInfos_origin[an->second];
                string svalue = atv[vSelectInfo_offsets[an->second] - 1][column];

                int ivalue = std::stoi(svalue);
                isum += ivalue;
                an++;
            }

            ssum = std::to_string(isum);
            LOG_F <<"ssum " << ssum <<endl;


            auto ag = getvalue.begin();
            LOG_F <<"ssum " << (*vSelectInfos_origin[vSelectInfo_offsets[ag->second] - 1].begin())[column] <<endl;
            //vector<vector<map<string, string> > > vSelectInfos_origin;

            vSelectInfos_origin[ag->second][vSelectInfo_offsets[ag->second] - 1][column] = ssum;

            vSelectInfo.insert(vSelectInfo.end(), 
                vSelectInfos_origin[ag->second].begin() + vSelectInfo_offsets[ag->second] - 1, 
                vSelectInfos_origin[ag->second].begin() + vSelectInfo_offsets[ag->second]);

            // vector<map<string,string> >
            for (auto atm = vSelectInfo.begin(); atm != vSelectInfo.end(); atm++)
            {
                map<string, string>& attm = *atm;
                for (auto fs = attm.begin(); fs != attm.end(); fs++)
                {
                    LOG_F <<"fs " << fs->first <<" " << fs->second<<endl;
                }
            }
        }
    }
    catch (std::exception const& e)
    {
        sErrorInfo=e.what();
        LOG->error() <<__FILE__<<"::"<< __FUNCTION__ << string(" catch std exception: ") + e.what() << endl;
    }
    catch (...)
    {
        sErrorInfo="unknown exception";
        LOG->error() <<__FILE__<<"::"<< __FUNCTION__<< " catch unknown exception" << endl;
    }
}

void MultiNodesDbHandle::HandleSqlSum( const string & sSQL, const string & sDbName, vector<map<string,string> > &vSelectInfo, string &sErrorInfo, struct sql_context_t* sql_context )
{
    // 
    // sql sum 处理流程： 查询各个nodes后， 求和所有结果的value值
    // 

    string origin_table;
    string column;
    string column_start;
    string column_end;

    //LOG_F << "sum" <<endl;

    if (sql_context->stmt_type == STMT_SELECT)
    {
        sql_select_t *p_sql_statement = (sql_select_t *)sql_context->sql_statement;

        sql_src_item_t *src = (sql_src_item_t *)g_ptr_array_index(p_sql_statement->from_src, 0);
        origin_table =  string(src->table_name);

        sql_expr_t *grp_expr_c = (sql_expr_t *)g_ptr_array_index(p_sql_statement->columns, 0);
        column_start = string(grp_expr_c->start);
        column_end = string(grp_expr_c->end);
        column = column_start.substr(0, column_start.size() - column_end.size());
        std::transform(column.begin(), column.end(), column.begin(), ::tolower);

        if (grp_expr_c->alias != NULL)
        {
            sql_expr_t *grp_expr_c = (sql_expr_t *)g_ptr_array_index(p_sql_statement->columns, 0);
            column = string(grp_expr_c->alias);
        }
    }


    vector<vector<map<string, string> > > vSelectInfos;


    for (auto db_node = _mDBConnectionArr_MultiNodes.begin(); db_node != _mDBConnectionArr_MultiNodes.end(); db_node++)
    {
        vector<tars::TC_Mysql>& anode = db_node->second;
        try
        {
            string modify_SQL = sSQL;
            string table_name = db_node->first;

            int position1 =  modify_SQL.find("from ");
            int position2 =  modify_SQL.find(" where");

            modify_SQL.replace(position1 + 5/*size of "from "*/, position2 - position1 - 5/*size of "from "*/, table_name);

            vector<map<string, string> >  avSelectInfo = getDb( sDbName, anode ).queryRecord( modify_SQL ).data();
            vSelectInfos.push_back(avSelectInfo);

            LOG_F <<"|"<<__FILE__LINE__FUNCTION__<<"|"<< sDbName <<"|"<< modify_SQL <<endl;
        }
        catch (std::exception const& e)
        {
            sErrorInfo=e.what();
            LOG->error() <<__FILE__<<"::"<< __FUNCTION__ << string(" catch std exception: ") + e.what() << endl;
        }
        catch (...)
        {
            sErrorInfo="unknown exception";
            LOG->error() <<__FILE__<<"::"<< __FUNCTION__<< " catch unknown exception" << endl;
        }
    }

    int isum = 0;
    string ssum;
    for (size_t i = 0;  i < vSelectInfos.size(); i++)
    {
        map<string, string> atm;
        vector<map<string, string>>& atv = vSelectInfos[i];
        string svalue = atv[0][column];

        int ivalue = std::stoi(svalue);
        isum += ivalue;
    }

    ssum = std::to_string(isum);
    vSelectInfos[0][0][column] = ssum;
    vSelectInfo.insert(vSelectInfo.end(), vSelectInfos[0].begin(), vSelectInfos[0].begin() + 1);

}

void MultiNodesDbHandle::HandleSqlAvg( const string & sSQL, const string & sDbName, vector<map<string,string> > &vSelectInfo, string &sErrorInfo, struct sql_context_t* sql_context )
{
    // 
    // sql avg 处理流程： 查询各个nodes的sum 和 count 后， 求和sum值和count值， 然后求平均
    // 

    string origin_table;
    string column;
    string column_start;
    string column_end;
    string column_sum;
    string column_count;

    //LOG_F << "avg" <<endl;

    if (sql_context->stmt_type == STMT_SELECT)
    {
        sql_select_t *p_sql_statement = (sql_select_t *)sql_context->sql_statement;

        sql_src_item_t *src = (sql_src_item_t *)g_ptr_array_index(p_sql_statement->from_src, 0);
        origin_table =  string(src->table_name);

        sql_expr_t *grp_expr_c = (sql_expr_t *)g_ptr_array_index(p_sql_statement->columns, 0);
        column_start = string(grp_expr_c->start);
        column_end = string(grp_expr_c->end);
        column = column_start.substr(0, column_start.size() - column_end.size());
        column_sum = column;
        column_count = column;
        column_sum.replace(0, 3, "sum");
        column_count.replace(0, 3, "count");
        std::transform(column.begin(), column.end(), column.begin(), ::tolower);

        if (grp_expr_c->alias != NULL)
        {
            sql_expr_t *grp_expr_c = (sql_expr_t *)g_ptr_array_index(p_sql_statement->columns, 0);
            column = string(grp_expr_c->alias);
            column_sum = column;
            column_count = column;
        }
    }
    LOG_F << column <<endl;

    vector<vector<map<string, string> > > vSelectInfos_sum;
    vector<vector<map<string, string> > > vSelectInfos_count;


    for (auto db_node = _mDBConnectionArr_MultiNodes.begin(); db_node != _mDBConnectionArr_MultiNodes.end(); db_node++)
    {
        vector<tars::TC_Mysql>& anode = db_node->second;
        try
        {
            string modify_SQL = sSQL;

            string table_name = db_node->first;

            int position1 =  modify_SQL.find("from ");
            int position2 =  modify_SQL.find(" where");

            modify_SQL.replace(position1 + 5/*size of "from "*/, position2 - position1 - 5/*size of "from "*/, table_name);

            string modify_SQL_sum = modify_SQL;
            string modify_SQL_count = modify_SQL;
            int position_avg =  modify_SQL.find("avg");

            modify_SQL_sum.replace(position_avg, 3, "sum");
            modify_SQL_count.replace(position_avg, 3, "count");

            vector<map<string, string> >  avSelectInfo_sum = getDb( sDbName, anode ).queryRecord( modify_SQL_sum ).data();
            vSelectInfos_sum.push_back(avSelectInfo_sum);
            vector<map<string, string> >  avSelectInfo_count = getDb( sDbName, anode ).queryRecord( modify_SQL_count ).data();
            vSelectInfos_count.push_back(avSelectInfo_count);

            //LOG_F <<"|"<<__FILE__LINE__FUNCTION__<<"|"<< sDbName <<"|"<< modify_SQL <<endl;
            LOG_F <<"|"<<__FILE__LINE__FUNCTION__<<"|"<< sDbName <<"|"<< modify_SQL_sum <<endl;
            LOG_F <<"|"<<__FILE__LINE__FUNCTION__<<"|"<< sDbName <<"|"<< modify_SQL_count <<endl;
        }
        catch (std::exception const& e)
        {
            sErrorInfo=e.what();
            LOG->error() <<__FILE__<<"::"<< __FUNCTION__ << string(" catch std exception: ") + e.what() << endl;
        }
        catch (...)
        {
            sErrorInfo="unknown exception";
            LOG->error() <<__FILE__<<"::"<< __FUNCTION__<< " catch unknown exception" << endl;
        }
    }

    int isum_sum = 0;
    int isum_count = 0;
    string savg;
    for (size_t i = 0;  i < vSelectInfos_sum.size(); i++)
    {
        //LOG_F << vSelectInfos_sum[i][0].begin()->first << endl;
        string svalue_sum = vSelectInfos_sum[i][0][column_sum];
        string svalue_count = vSelectInfos_count[i][0][column_count];

        int ivalue_sum = std::stoi(svalue_sum);
        int ivalue_count = std::stoi(svalue_count);
        isum_sum += ivalue_sum;
        isum_count += ivalue_count;
    }

    savg = std::to_string((float)isum_sum / (float)isum_count);

    vSelectInfos_sum[0][0].clear();
    vSelectInfos_sum[0][0][column] = savg;
    vSelectInfo.insert(vSelectInfo.end(), vSelectInfos_sum[0].begin(), vSelectInfos_sum[0].begin() + 1);

    //LOG_F << isum_sum << " " << isum_count <<endl;
}
