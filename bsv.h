#ifndef BSV
#define BSV
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include<boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include<boost/make_shared.hpp>
#include<boost/enable_shared_from_this.hpp>
#include<map>
#include "mysql.h"
#include "ini.h"
#include "bss.h"

using namespace boost;
using boost::asio::ip::tcp;
using namespace boost::asio;
class bss;
typedef  shared_ptr<bss> bss_ptr;

class bsv:public enable_shared_from_this<bsv>
{
public:
    
    bsv( boost::asio::io_service & io_service, int port);
    void handle_accept(bss_ptr new_session, const boost::system::error_code& error);
private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;

    void handle_timeout(const boost::system::error_code& error);
    deadline_timer timer_;
public:
    bool conToDb();
    void startMoniter();
    void execDBMonitor(char*sql);
    void execDBMonitor();
public:
    bool initValues ();
private:
    bool isDbConnected;
    std::string dbHost;
    std::string dbNme;
    std::string dbUsr;
    std::string dbPwd;
    int dbPort;
    struct timeval      time_val; 
    struct timezone      tz;                    /*>时区信息结构体<*/
    
    MYSQL  *con;
    MYSQL_RES *res;
    char *sql;
    MYSQL_RES *resptz;
    MYSQL_RES *resste;
public:
    std::map<const char*,bss*> map_;
    std::map<const char*,int> map_wake_count;
    boost::mutex   m_lock;
};
#endif // BSV

