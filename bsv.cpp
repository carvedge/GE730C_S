#include "bsv.h"
#include "fun.h"
#include "Log.h"


bsv::bsv( boost::asio::io_service& _io_service,  int port)
    : io_service_(_io_service),
      timer_(_io_service),
      acceptor_(_io_service, tcp::endpoint(tcp::v4(), port))
{
    bss_ptr new_session(new bss(io_service_,this));
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&bsv::handle_accept, this, new_session, _1));

}
void bsv::handle_accept(bss_ptr new_session,const boost::system::error_code& error)
{

    if (!error)
    {
        std::cout<<"handle accept"<<std::endl;
        new_session->start();

        new_session.reset(new bss(io_service_,this));
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&bsv::handle_accept, this, new_session,
                                           _1));
    }
    else
    {
        std::cout<<"delete accept"<<std::endl;
        //delete new_session;
    }
}
bool bsv::initValues(){
    string ini_file = "./db.ini";
    Ini *ini = new Ini();
    ini->setFileName(ini_file);
    //暂时认为  dbHost 也是 rtmp 服务器ip
    dbHost = ini->get( "cfg.host" );
    dbPort = boost::lexical_cast<int>(ini->get( "cfg.port" ));
    dbNme = ini->get( "cfg.nme" );
    dbUsr = ini->get( "cfg.usr" );
    dbPwd = ini->get( "cfg.pwd" );
    return true;
}
bool bsv::conToDb(){
    if (!isDbConnected)
    {
        con = (MYSQL*)malloc(sizeof(MYSQL));
        mysql_init(con);
        if (!mysql_real_connect(con,dbHost.c_str(),dbUsr.c_str(),dbPwd.c_str(),dbNme.c_str(),dbPort,NULL ,0))
        {
            printf("Error connecting to database:%s\n",mysql_error(con));
            return false;
        }
        else{
            isDbConnected = true;
            printf("connect successed\n");
            char value = 1;
            mysql_options(con,MYSQL_OPT_RECONNECT,&value);
            sql =( char* ) malloc(100);
            return true;
        }
    }
    else{
        return true;
    }
}

void bsv::execDBMonitor(char * sql){
    mysql_query(con,sql);
}
void bsv::execDBMonitor()
{

        sprintf(sql,"select * from xts");
        mysql_query(con,sql);
        res = mysql_store_result(con);
        if (res)
        {
           //printf("count is %d\n",mysql_num_rows(res));
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)))
            {
                int id = atoi(row[0]);
                const char* bid = row[1];
                {
                   // printf("table bid is %s\n",bid);
                    boost::mutex::scoped_lock lock(m_lock);
                    map<const char*,bss*>::iterator _it=map_.begin();
                    while(_it!=map_.end())
                    {

                        if(ucharCmp((unsigned char*)_it->first,(unsigned char*)bid,17)==0)
                        {
                            printf("map bid is %s,table bid is %s\n",_it->first,bid);
                            int pm1 = atoi(row[2]);//1:打开打开视频  5:截图 6:c版本唤醒 7:c版本配置
                            printf("require new cmd ,bid is %s,cmd is %d\n",bid,pm1);
                            if (1 == pm1)
                            {
                                _it->second->send_require_video_on(true);
                            }
                            else if(2 == pm1){
                                int pm2 = atoi(row[3]);
                                // > 1000 控制云台
                                //100-119 设巡视点
                                //200-219 置巡视点
                                printf("pm2 is %d\n",pm2);
                                if (pm2 > 1000 )
                                {
                                    _it->second->send_require_ptz(pm2/1000);
                                }
                                else if(pm2 >= 200){
                                    _it->second->send_require_to_pre(pm2%200);
                                }
                                else if(pm2 >= 100){
                                    _it->second->send_require_set_pre(pm2%100);
                                } 
                            }
                            else if(5 == pm1){
                                _it->second->send_require_capture();
                            }
                            else if(6 == pm1)
                            {
                                    int pm2 = atoi(row[3]);
                                    gettimeofday(&time_val,&tz);
                                    int current_time = time_val.tv_sec;
                                    map<const char*,int>::iterator _it_wake = map_wake_count.begin();
                                    while(_it_wake != map_wake_count.end())
                                    {
                                        if(ucharCmp((unsigned char*)_it_wake->first,(unsigned char*)bid,17)==0)
                                        {
                                             break;
                                        }
                                        ++_it_wake;
                                    }
                                    if(_it_wake->second == 0)
                                    {
                                        _it_wake->second  = 1;
                                    }
                                    else if(_it_wake->second  == 1)
                                    {
                                        _it->second->send_require_wakeup();
                                        _it_wake->second  = 2;
                                        break;//break跳转,数据库中的本条命令不删除
                                    }
                                    else if(((current_time - pm2) > 120)&&(_it_wake->second == 2))
                                    {
                                        _it->second->send_require_wakeup();
                                        _it_wake->second = 3;
                                        std::string str_bid(bid);
                                        _it->second->execXBeat(str_bid, 5);
                                        break;//break跳转,数据库中的本条命令不删除
                                    }
                                    else if(((current_time - pm2) > 240)&&(_it_wake->second == 3))
                                    {
                                        _it->second->send_require_wakeup();
                                        _it_wake->second = 4;
                                        std::string str_bid(bid);
                                        _it->second->execXBeat(str_bid, 5);
                                        break;//break跳转,数据库中的本条命令不删除
                                    }
                                    else if(((current_time - pm2) > 360)&&(_it_wake->second == 4))
                                    {
                                        std::string str_bid(bid);
                                        _it->second->execXBeat(str_bid, 4);//清除一键巡视命令
                                        _it_wake->second = 1;
                                        //没有break删除数据库中的本条命令
                                    }
                                    else
                                        break;
                            }
                            else if(7 == pm1)
                            {
                                _it->second->send_require_config(row[4],row[5],row[6],row[7],row[8],row[9],row[10],row[11],row[12]);  
                            }
                            char* deleteSql = (char*)malloc(100);
                            sprintf(deleteSql,"delete from xts  where id = %d",id);
                            printf("%s\n", deleteSql);
                            mysql_query(con,deleteSql);
                            free(deleteSql);
                            break;  
                        }
                        ++_it;
                        
                    }
                }
            }
            mysql_free_result(res);//释放结果集使用的内存
        }
    
}

void bsv::handle_timeout(const boost::system::error_code& error)
{
    if (error)
    {
        std::cout << "time error: " << error.message() << "\n";
    }
    else
    {
        execDBMonitor();
        timer_.expires_from_now(boost::posix_time::microseconds(100));
        timer_.async_wait(boost::bind(&bsv::handle_timeout, this,boost::asio::placeholders::error));
    }

}
void bsv::startMoniter()
{
    timer_.expires_from_now(boost::posix_time::microseconds(100));
    timer_.async_wait(boost::bind(&bsv::handle_timeout, this,boost::asio::placeholders::error));
}

