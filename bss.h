
#ifndef BSS
#define BSS
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include<boost/function.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include<boost/make_shared.hpp>
#include<boost/enable_shared_from_this.hpp>
#include"bsv.h"
class bsv;
using namespace boost;
using namespace boost::asio;
using boost::asio::ip::tcp;
using namespace std;
class bss:public enable_shared_from_this<bss>
{
  public:
    bss(boost::asio::io_service& io_service,bsv* parent);
    ~bss();
    tcp::socket& socket()
    {
        return socket_;
    }
    void start();
    void handle_read(const boost::system::error_code& error,size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);
  private:
    boost::asio::io_service& io_service_;
    tcp::socket socket_;
    enum { max_length = 1024 };
    enum { dat_max_length = 2048 };
    unsigned char inbuf_[dat_max_length];
   
    bsv * parent_;
    
  private:
   void parse(unsigned char*data,size_t length);
   void parseDetail(unsigned char*data,size_t length);
   void reply(unsigned char Frame_Type);
   void send(unsigned char Frame_Type);
   
  public:
   unsigned char* Bid;
   void send_require_video_on(bool on);//查看视频
   void send_require_wakeup();//唤醒
   void send_require_capture();//截图
   void send_require_ptz(int pm2);//控制云台
   void send_require_set_pre(int pm2);//设pre
   void send_require_to_pre(int pm2);//置pre
   void send_require_config(char*d1,char*d2,char*d3,char*d4,char*d5,char*d6,char*d7,char*d8,char*d9);
  private:
     void handle_timeout(const boost::system::error_code& error);
     deadline_timer timer_;
     int timeIndex;
  private:
    void execXBeat(string bid,int xType);
    void httpXPower(char*powerData,char*tempData,char*gpsData);
    tcp::resolver::iterator endpoint_iterator_mip;
    int resloved_mip;//
    tcp::resolver::iterator endpoint_iterator_sip;
    int resloved_sip;//
    //int xType;//请求类型，0:单片机心跳 1:是否处于加热状态 2:单片机配置之后回复心跳 3:单片机收到唤醒之后回复
    int xs1;
    int xs2;
    int xs3;
    int xs4;
    int xs5;
    int xs6;
    void X2Str(unsigned char* s_src, int n_len_src, char* s_dst);
    void Str2X(char* s_src, int n_len_dst, unsigned char* s_dst);
  private:
    //心跳后保存配置信息
    unsigned char Day_Start[1]; 
    unsigned char Day_End[1]; 
    unsigned char Span_Heart[1]; 
    unsigned char Span_Work[1]; 
    unsigned char Hot_Temp[1]; 
    unsigned char Hot_Time[1]; 
    unsigned char Close_Volt[1]; 
    unsigned char Alarm_Switch[1]; 
    unsigned char Alarm_Id[4]; 
    void compare_config(string reply);
    void exec_cfg_c(string bid);
    int cfg_cnt ;
    int re_send_x;//心跳发送失败后，重新发送心跳

};

#endif // TCP_SESSION
