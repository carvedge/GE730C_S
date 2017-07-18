#include "bss.h"
#include "bsv.h"
#include "fun.h"
#include "Log.h"

#include <sstream>
extern boost::mutex   m_lock;
bss::bss(boost::asio::io_service & io_service, bsv* parent)
    :io_service_(io_service), socket_(io_service),timer_(io_service),parent_(parent)
{
     timeIndex = 0;
     resloved_mip == 0;
     resloved_sip == 0;
     cfg_cnt = 0;
     Bid = new unsigned char[17];
     re_send_x = 3;
}
bss::~bss()
{
    std::cout<<"release session"<<std::endl;
}
void bss::start()
{
        // map_bss[bid_]=this;

    boost::system::error_code ec;
    boost::asio::socket_base::keep_alive option(true);
    socket_.set_option(option, ec);

    socket_.non_blocking(true);
    socket_.async_read_some(boost::asio::buffer(inbuf_, max_length),
                            boost::bind(&bss::handle_read, shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));

    timer_.expires_from_now(boost::posix_time::seconds(60));
    timer_.async_wait(boost::bind(&bss::handle_timeout, shared_from_this(),boost::asio::placeholders::error));
   


}
void bss::handle_timeout(const boost::system::error_code& error)
{
    if (error)
    {
        std::cout << "time error: " << error.message() << "\n";
    }
    else
    {
        ++timeIndex;
        if (timeIndex % 10 == 0)
        {
            //send(0x05);
        }
        timer_.expires_from_now(boost::posix_time::seconds(10));
        timer_.async_wait(boost::bind(&bss::handle_timeout, shared_from_this(),boost::asio::placeholders::error));
    }
    
}

void bss::handle_read(const boost::system::error_code& error,size_t bytes_transferred)
{
    
    if(!error)
    {
    	
        parse(inbuf_,bytes_transferred);
        socket_.async_read_some(boost::asio::buffer(inbuf_, max_length),
                            boost::bind(&bss::handle_read, shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));


		
    }
    else
    {
    	std::cout << "socket read error---: " << error.message() << "\n";
        LOG("handle_read error,bid is %s\n",(char*)Bid);
        if(socket_.is_open())
        socket_.close( );
    }
}
void bss::parse(unsigned char*data,size_t length){
    printf("parse data ,receive length is %d\n", length);
    if (0xA5 == data[0] && 0x5A == data[1])
    {
          int length = ucha2int(data+2);
          printf("parse length is %d\n",length);
          unsigned short crc = RTU_CRC( data, length-2 );
          int receiveCrc =  ucha2int(data+length-2);
          printf("calculate crc  is %02x%02x\n",crc&0xFF,(crc>>8)& 0xFF);
          printf("receiveCrc  is %02x%02x\n",data[length-2],data[length-1] );
          parseDetail(data+4,length-6);
          
    }

}
/**
 *解析具体数据
 */
void bss::parseDetail(unsigned char*data,size_t length){
   
   unsigned char Frame_Type[1];  
   memcpy(Bid, data, 17); 
   printf("parse bid is %s\n",Bid );

    {
        boost::mutex::scoped_lock lock(parent_->m_lock);
        bool exist = false;
        map<const char*,bss*>::iterator _it=parent_->map_.begin();
        map<const char*,bss*>::iterator _next_it=_it;
        while(_it!=parent_->map_.end())
        {
            if(ucharCmp((unsigned char*)_it->first,Bid,17)==0){
                bss * b =  _it->second;
                if(b !=  this)
                {
                    printf("remove map bid,bid is %s\n",(char*)Bid );
                    _next_it = _it;
                    ++_next_it;
                    parent_->map_.erase(_it);
                    _it = _next_it;
                    continue;
                }
                else{
                    exist = true; 
                }
            }
            ++_it;
        }
        if (!exist)
        {
            printf("insert map bid,bid is %s\n",(char*)Bid );
            parent_->map_.insert(std::pair<const char*, bss*>((const char*)Bid, this));  
        }
    }
    
    


   memcpy(Frame_Type, data+17, 1);
   printf("Frame_Type is %02x\n",Frame_Type[0]);
   if (Frame_Type[0] == 0x01)
   {
    LOG("handle_read hbt,bid is %s\n",(char*)Bid);
    //单片机发送心跳
     unsigned char Battery_Status[1]; 
     unsigned char Cam_Status[1];
     unsigned char GPS_Status[1]; 
     unsigned char Power_Status[1];
     unsigned char Temp_Status[1];  
    
     
     memcpy(Battery_Status, data+18, 1);
     memcpy(Cam_Status, data+19, 1);
     memcpy(GPS_Status, data+20, 1);
     memcpy(Power_Status, data+21, 1);
     memcpy(Temp_Status, data+22, 1);
     memcpy(Day_Start, data+23, 1);
     memcpy(Day_End, data+24, 1);
     memcpy(Span_Heart, data+25, 1);
     memcpy(Span_Work, data+26, 1);
     memcpy(Hot_Temp, data+27, 1);
     memcpy(Hot_Time, data+28, 1);
     memcpy(Close_Volt, data+29, 1);
     memcpy(Alarm_Switch, data+30, 1);
     memcpy(Alarm_Id, data+31, 4);
     xs1 = Battery_Status[0]==0xFF ? 1:0;
     xs2 = Cam_Status[0]==0xFF ? 1:0;
     xs3 = GPS_Status[0]==0xFF ? 1:0;
     xs4 = Power_Status[0]==0xFF ? 1:0;
     xs5 = Temp_Status[0]==0xFF ? 1:0;
     //保存数据库 update
     if (++cfg_cnt % 5 == 0)
     {
        // boost::function0<void>f1=boost::bind(&bss::exec_cfg_c,this);
        // boost::thread thrd1(f1);

        char bid_[18];
        memset(bid_,'\0',18);
        memcpy(bid_,Bid,17);
        std::string str_bid(bid_);
        exec_cfg_c(str_bid);


     }
     else{
        char bid_[18];
        memset(bid_,'\0',18);
        memcpy(bid_,Bid,17);
        std::string str_bid(bid_);
        execXBeat(str_bid,0);
        // boost::function0<void>f=boost::bind(&bss::execXBeat,this);
        // boost::thread thrd(f); 
     }
     
   }
   else if (Frame_Type[0] == 0x03){
    //单片机定时发送电源数据和温度数据
    unsigned char Power_Data[19]; 
    unsigned char Temp_Data[2];
    unsigned char GPS_Data[12];
    
    
     
    memcpy(Power_Data, data+18, 19);
    memcpy(Temp_Data, data+37, 2);
    memcpy(GPS_Data, data+39, 12);

    char * powerData = (char*)malloc(50);
    char * tempData  = (char*)malloc(50);
    char * gpsData   = (char*)malloc(50);

    X2Str(Power_Data, 19, powerData);
    X2Str(Temp_Data, 2, tempData);
    X2Str(GPS_Data, 12, gpsData);
    httpXPower(powerData,tempData,gpsData);
    free(powerData);
    free(tempData);
    free(gpsData);

    
   }
   else if (Frame_Type[0] == 0x06){
    //单片机回复唤醒命令
      // boost::function0<void>f=boost::bind(&bss::execXBeat,this);
      // boost::thread thrd(f); 

        char bid_[18];
        memset(bid_,'\0',18);
        memcpy(bid_,Bid,17);
      std::string str_bid(bid_);
      execXBeat(str_bid,3);

   }
   else if (Frame_Type[0] == 0x08){
    //单片机回复服务器的配置命令
      // xType = 2;
      // boost::function0<void>f=boost::bind(&bss::execXBeat,this);
      // boost::thread thrd(f); 

        char bid_[18];
        memset(bid_,'\0',18);
        memcpy(bid_,Bid,17);
       std::string str_bid(bid_);
       execXBeat(str_bid,2);
    
   }
   else if (Frame_Type[0] == 0x09){
    //单片机发送处于加热状态的指令
    unsigned char Hot_State[1];
    memcpy(Hot_State, data+18, 1);

    //xType = 1;
    xs6 = Hot_State[0]==0xFF ? 1:0;
    // boost::function0<void>f=boost::bind(&bss::execXBeat,this);
    // boost::thread thrd(f); 

    char bid_[18];
        memset(bid_,'\0',18);
        memcpy(bid_,Bid,17);
     std::string str_bid(bid_);
     execXBeat(str_bid,1);

   }
   else if (Frame_Type[0] == 0x11){
    //机芯发送给服务器的心跳
     
   }
   else if (Frame_Type[0] == 0x14){
    //机芯回复视频请求命令
    
   }
   else if (Frame_Type[0] == 0x15){
    //机芯发送数据给服务器
    unsigned char Power_Data[19]; 
    unsigned char Temp_Data[2];
    unsigned char GPS_Data[12];
    
    
     
    memcpy(Power_Data, data+18, 19);
    memcpy(Temp_Data, data+37, 2);
    memcpy(GPS_Data, data+39, 12);

    // char * powerData = new char[50];
    // char * tempData = new char[50];
    // char * gpsData = new char[50];

    // X2Str(Power_Data, 19, powerData);
    // X2Str(Temp_Data, 2, tempData);
    // X2Str(GPS_Data, 2, gpsData);

    // char * sql = new char[100];

    // time_t t;
    // int dte = time(&t);

    // sprintf(sql,"insert into xwr(dte,pwr,tem,gps) values(%d,%s,%s)",dte,powerData,tempData,gpsData);
    // parent_->execDBMonitor(sql);
    // free(sql);
    // free(powerData);
    // free(tempData);
   }
   reply(Frame_Type[0]);
}
void bss::send(unsigned char Frame_Type){
  reply(Frame_Type);
}
void bss::send_require_video_on(bool on){

    printf("------------------------0\n");
    unsigned char ret_dat[500];
    unsigned char Head[2];
    unsigned char packet_length[2];
    ret_dat[0] = 0xA5;
    ret_dat[1] = 0x5A;

   
    int length = 2+2+17+1+1+2;
    ret_dat[2] = length& 0xFF;
    ret_dat[3] = (length>>8)& 0xFF;
   
    memcpy(ret_dat+4, Bid, 17);
    ret_dat[21] = 0x13;
    if(on){
        ret_dat[22] = 0xFF;
    }
    else{
        ret_dat[22] = 0x00;
    }
    unsigned short crc = RTU_CRC( ret_dat, length-2 );     
    ret_dat[24] = crc>>8 & 0xFF;
    ret_dat[23] = crc & 0xFF;
    socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                             boost::bind(&bss::handle_write, shared_from_this(),
                                         boost::asio::placeholders::error()));


}
void bss::send_require_ptz(int pm2){
    printf("bss require %d\n",pm2);
    unsigned char ret_dat[1500];
    unsigned char Head[2];
    unsigned char packet_length[2];
    ret_dat[0] = 0xA5;
    ret_dat[1] = 0x5A;

   
    int length = 2+2+17+1+1+2;
    ret_dat[2] = length& 0xFF;
    ret_dat[3] = (length>>8)& 0xFF;
   
    memcpy(ret_dat+4, Bid, 17);
    ret_dat[21] = 0x17;
    ret_dat[22] = pm2& 0xFF;
    
    unsigned short crc = RTU_CRC( ret_dat, length-2 );     
    ret_dat[24] = crc>>8 & 0xFF;
    ret_dat[23] = crc & 0xFF;
    socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                             boost::bind(&bss::handle_write, shared_from_this(),
                                         boost::asio::placeholders::error()));
}
void bss::send_require_set_pre(int pm2){
    printf("bss require %d\n",pm2);
    unsigned char ret_dat[1500];
    unsigned char Head[2];
    unsigned char packet_length[2];
    ret_dat[0] = 0xA5;
    ret_dat[1] = 0x5A;

   
    int length = 2+2+17+1+1+2;
    ret_dat[2] = length& 0xFF;
    ret_dat[3] = (length>>8)& 0xFF;
   
    memcpy(ret_dat+4, Bid, 17);
    ret_dat[21] = 0x19;
    ret_dat[22] = pm2& 0xFF;
    
    unsigned short crc = RTU_CRC( ret_dat, length-2 );     
    ret_dat[24] = crc>>8 & 0xFF;
    ret_dat[23] = crc & 0xFF;
    socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                             boost::bind(&bss::handle_write, shared_from_this(),
                                         boost::asio::placeholders::error()));
}
void bss::send_require_to_pre(int pm2){
    printf("bss require %d\n",pm2);
    unsigned char ret_dat[1500];
    unsigned char Head[2];
    unsigned char packet_length[2];
    ret_dat[0] = 0xA5;
    ret_dat[1] = 0x5A;

   
    int length = 2+2+17+1+1+2;
    ret_dat[2] = length& 0xFF;
    ret_dat[3] = (length>>8)& 0xFF;
   
    memcpy(ret_dat+4, Bid, 17);
    ret_dat[21] = 0x21;
    ret_dat[22] = pm2& 0xFF;
    
    unsigned short crc = RTU_CRC( ret_dat, length-2 );     
    ret_dat[24] = crc>>8 & 0xFF;
    ret_dat[23] = crc & 0xFF;
    socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                             boost::bind(&bss::handle_write, shared_from_this(),
                                         boost::asio::placeholders::error()));
}
void bss::send_require_wakeup(){
    send(0x05);
}
void bss::send_require_capture(){
    printf("bss require capture\n");
    unsigned char ret_dat[1500];
    unsigned char Head[2];
    unsigned char packet_length[2];
    ret_dat[0] = 0xA5;
    ret_dat[1] = 0x5A;

   
    int length = 2+2+17+1+1+2;
    ret_dat[2] = length& 0xFF;
    ret_dat[3] = (length>>8)& 0xFF;
   
    memcpy(ret_dat+4, Bid, 17);
    ret_dat[21] = 0x23;
    ret_dat[22] = 0x00;
    
    unsigned short crc = RTU_CRC( ret_dat, length-2 );     
    ret_dat[24] = crc>>8 & 0xFF;
    ret_dat[23] = crc & 0xFF;
    socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                             boost::bind(&bss::handle_write, shared_from_this(),
                                         boost::asio::placeholders::error()));
}
void bss::send_require_config(char*d1,char*d2,char*d3,char*d4,char*d5,char*d6,char*d7,char*d8,char*d9){
    int length = 2+2+17+1*9+4+2;
    unsigned char ret_dat[500];
    ret_dat[0] = 0xA5;
    ret_dat[1] = 0x5A;
    ret_dat[2] = length& 0xFF;
    ret_dat[3] = (length>>8)& 0xFF;
    memcpy(ret_dat+4, Bid, 17);
    ret_dat[21] = 0x07;

    ret_dat[22] = atoi(d1);
    ret_dat[23] = atoi(d2);
    ret_dat[24] = atoi(d3);
    ret_dat[25] = atoi(d4);
    ret_dat[26] = atoi(d5);
    ret_dat[27] = atoi(d6);
    ret_dat[28] = atoi(d7);
    ret_dat[29] = atoi(d8);
    Str2X(d9, 4, ret_dat+30);
    unsigned short crc = RTU_CRC( ret_dat, length-2 );     
    ret_dat[length-1] = crc>>8 & 0xFF;
    ret_dat[length-2] = crc & 0xFF;
    socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                             boost::bind(&bss::handle_write, shared_from_this(),
                                         boost::asio::placeholders::error()));
}
void bss::reply(unsigned char Frame_Type){
    unsigned char ret_dat[500];
    unsigned char Head[2];
    unsigned char packet_length[2];
    ret_dat[0] = 0xA5;
    ret_dat[1] = 0x5A;

    if (Frame_Type == 0x01)
    {
        int length = 2+2+17+1+6+2;
        ret_dat[2] = length& 0xFF;
        ret_dat[3] = (length>>8)& 0xFF;
        memcpy(ret_dat+4, Bid, 17);
        
        ret_dat[21] = 0x02;
        time_t tmpcal_ptr = {0};
        struct tm *tmp_ptr = NULL;
        tmpcal_ptr = time(NULL);
        tmp_ptr = localtime(&tmpcal_ptr);
        printf("after localtime, date is :%d:%d:%d, time is:%d:%d:%d\n", tmp_ptr->tm_year%100,tmp_ptr->tm_mon+1,tmp_ptr->tm_mday,tmp_ptr->tm_hour, tmp_ptr->tm_min, tmp_ptr->tm_sec);

 //服务器当前时间，年、月、日、时、分、秒各占一个字节，年20默认，后面16占一个字节，如0x10表示2016年。
        ret_dat[22] = tmp_ptr->tm_year%100;
        ret_dat[23] = tmp_ptr->tm_mon+1;
        ret_dat[24] = tmp_ptr->tm_mday;
        ret_dat[25] = tmp_ptr->tm_hour;
        ret_dat[26] = tmp_ptr->tm_min;
        ret_dat[27] = tmp_ptr->tm_sec;
        unsigned short crc = RTU_CRC( ret_dat, length-2 );     
        ret_dat[29] = crc>>8 & 0xFF;
        ret_dat[28] = crc & 0xFF;
        socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                                 boost::bind(&bss::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error()));

    }
    else if(Frame_Type == 0x03){
       int length = 2+2+17+1+2;
        ret_dat[2] = length& 0xFF;
        ret_dat[3] = (length>>8)& 0xFF;
        memcpy(ret_dat+4, Bid, 17);
        ret_dat[21] = 0x04;
        unsigned short crc = RTU_CRC( ret_dat, length-2 );     
        ret_dat[23] = crc>>8 & 0xFF;
        ret_dat[22] = crc & 0xFF;
        socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                                 boost::bind(&bss::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error()));

    }
    else if(Frame_Type == 0x05){
       int length = 2+2+17+1+2;
        ret_dat[2] = length& 0xFF;
        ret_dat[3] = (length>>8)& 0xFF;
        memcpy(ret_dat+4, Bid, 17);
        ret_dat[21] = 0x05;
        unsigned short crc = RTU_CRC( ret_dat, length-2 );     
        ret_dat[23] = crc>>8 & 0xFF;
        ret_dat[22] = crc & 0xFF;
        socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                                 boost::bind(&bss::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error()));

    }
    else if(Frame_Type == 0x09){
       int length = 2+2+17+1+2;
        ret_dat[2] = length& 0xFF;
        ret_dat[3] = (length>>8)& 0xFF;
        memcpy(ret_dat+4, Bid, 17);
        ret_dat[21] = 0x10;
        unsigned short crc = RTU_CRC( ret_dat, length-2 );     
        ret_dat[23] = crc>>8 & 0xFF;
        ret_dat[22] = crc & 0xFF;
        socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                                 boost::bind(&bss::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error()));

    }
    else if(Frame_Type == 0x11){
       int length = 2+2+17+1+2;
        ret_dat[2] = length& 0xFF;
        ret_dat[3] = (length>>8)& 0xFF;
        memcpy(ret_dat+4, Bid, 17);
        ret_dat[21] = 0x12;
        unsigned short crc = RTU_CRC( ret_dat, length-2 );     
        ret_dat[23] = crc>>8 & 0xFF;
        ret_dat[22] = crc & 0xFF;
        socket_.async_write_some(boost::asio::buffer(ret_dat, length),
                                 boost::bind(&bss::handle_write, shared_from_this(),
                                             boost::asio::placeholders::error()));

    }

}
void bss::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {

    }
    else
    {
      LOG("handle_write error,bid is %s\n",(char*)Bid);
    }
}
//转发单片机心跳
void bss::execXBeat(string str_bid,int xType)
{
    printf("execXBeat,bid is %s\n", str_bid.c_str());
    tcp::resolver resolver(io_service_);
    tcp::resolver::query query("ivs2.carvedge.com","80");
    LOG("resolve before ,bid is %s\n",str_bid.c_str());

    if (resloved_mip == 0)
    {
      try { 
          endpoint_iterator_mip = resolver.resolve(query);
      }
      catch (boost::system::system_error& e) {
           std::cout <<e.what()<<"\n"; 
           return;
      }
    }
    
   
   //LOG("resolve after ,bid is %s\n",bid_);
   resloved_mip = 1;


    tcp::socket socket(io_service_);

   
    boost::system::error_code ec = boost::asio::error::host_not_found;
    tcp::resolver::iterator iterator = endpoint_iterator_mip;
    tcp::resolver::iterator end;
    while(ec && iterator != end) {
        printf("connect iterator ,ec is %s\n", ec.message().c_str());
        socket.close();
        socket.connect(*iterator++, ec);
    }
    
    
    if( ec ){
        //have not network connect,then per hour save preset images
        resloved_mip =0;
        LOG("execXBeat connect failed,bid is %s,ec is %s\n",str_bid.c_str(),ec.message().c_str());
        
         if (--re_send_x > 0)
         {
            execXBeat(str_bid,xType);
         }
         return;
    }
    re_send_x = 3;
    std::ostringstream stringStream;
    if (0 == xType)
    {
        stringStream <<"/svr/box.php?act=x&bid="<<str_bid<<"&s1="<<xs1<<"&s2="<<xs2<<"&s3="<<xs3<<"&s4="<<xs4<<"&s5="<<xs5;
    }
    else if(1 == xType){
        stringStream <<"/svr/box.php?act=x&bid="<<str_bid<<"&s6="<<xs6;
    }
    else if(2 == xType){
        stringStream <<"/svr/box.php?act=l&bid="<<str_bid<<"&tpe=2";//配置单片机
    }
    else if(3 == xType){
        stringStream <<"/svr/box.php?act=l&bid="<<str_bid<<"&tpe=1";//唤醒单片机
    }
   
    string path_ = stringStream.str();
    LOG("execXBeat ,request url is  %s\n",path_.c_str());
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path_ << " HTTP/1.0\r\n";
    request_stream << "Host: " << "ivs2.carvedge.com" << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    boost::asio::write(socket, request,ec);
    if( ec ){
        //resloved = 0;
        cout<<222<<endl;
        LOG("-----------0，request url is  %s\n",path_.c_str());
        return;
    }

    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n",ec);
     if( ec ){
         //resloved = 0;
         LOG("-----------1，request url is  %s\n",path_.c_str());
         return;
    }

    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
        std::cout << "Invalid response\n";
      //  LOG("-----------2，request url is  %s\n",path_.c_str());
        //resloved = 0;
        return ;
    }
    if (status_code != 200)
    {
        std::cout << "Response returned with status code " << status_code << "\n";
      //  LOG("-----------3，request url is  %s\n",path_.c_str());
        return ;
    }
    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n",ec);
     if( ec ){
         //resloved = 0;
         return;
    }
    //Process the response headers.
    std::string header;
    while (std::getline(response_stream, header) && header != "\r");
    //the heart-beat reply.
    std::string reply_;
    std::getline(response_stream ,reply_);

    while(boost::asio::read(socket, response, boost::asio::transfer_at_least(1), ec)){
      ;
    }
    cout<<"reply: "<<reply_ <<endl;
   // LOG("reply  bid is %s,reply is %s\n",(char*)Bid,reply_.c_str());
    socket.close();

}

void bss::exec_cfg_c(string str_bid)
{
    tcp::resolver resolver(io_service_);
    tcp::resolver::query query("ivs2.carvedge.com","80");
   
    if (resloved_mip == 0)
    {
      try { 
          endpoint_iterator_mip = resolver.resolve(query);
      }
      catch (boost::system::system_error& e) {
           std::cout <<e.what()<<"\n"; 
           return;
      }
    }
   resloved_mip = 1;

    tcp::socket socket(io_service_);

   
    boost::system::error_code ec = boost::asio::error::host_not_found;
    tcp::resolver::iterator iterator = endpoint_iterator_mip;
    tcp::resolver::iterator end;
    while(ec && iterator != end) {
        printf("connect iterator ,ec is %s\n", ec.message().c_str());
        socket.close();
        socket.connect(*iterator++, ec);
    }
    if( ec ){
        //have not network connect,then per hour save preset images
        resloved_mip =0;
        return;
    }
    std::ostringstream stringStream;
    //http://ivs2.carvedge.com/svr/app.php?act=cfg-c&bid=01230021702130001
    stringStream <<"/svr/app.php?act=cfg-c&bid="<<str_bid;
    string path_ = stringStream.str();
    LOG("exec_cfg_c ,request url is  %s\n",path_.c_str());
   // LOG("request url is  %s\n",path_.c_str());
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path_ << " HTTP/1.0\r\n";
    request_stream << "Host: " << "ivs2.carvedge.com" << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    boost::asio::write(socket, request,ec);
    if( ec ){
        //resloved = 0;
        cout<<222<<endl;
       // LOG("-----------0，request url is  %s\n",path_.c_str());
        return;
    }

    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n",ec);
     if( ec ){
         //resloved = 0;
       //  LOG("-----------1，request url is  %s\n",path_.c_str());
         return;
    }

    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
        std::cout << "Invalid response\n";
       // LOG("-----------2，request url is  %s\n",path_.c_str());
        //resloved = 0;
        return ;
    }
    if (status_code != 200)
    {
        std::cout << "Response returned with status code " << status_code << "\n";
       // LOG("-----------3，request url is  %s\n",path_.c_str());
        return ;
    }
    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n",ec);
     if( ec ){
         //resloved = 0;
         return;
    }
    //Process the response headers.
    std::string header;
    while (std::getline(response_stream, header) && header != "\r");
    //the heart-beat reply.
    std::string reply_;
    std::getline(response_stream ,reply_);

    while(boost::asio::read(socket, response, boost::asio::transfer_at_least(1), ec)){
      ;
    }
    cout<<"reply: "<<reply_ <<endl;
    compare_config(reply_);

    socket.close();

}
void bss::compare_config(string reply){
  vector<string> vStr;
    boost::split( vStr, reply, boost::is_any_of( "," ), boost::token_compress_on);
    //sip,视频开关，截图，电源数据，版本，时间戳
    if (vStr.size()<9)
    {
        return;
    }


    unsigned char day_Start[1]; 
    unsigned char day_End[1]; 
    unsigned char span_Heart[1]; 
    unsigned char span_Work[1]; 
    unsigned char hot_Temp[1]; 
    unsigned char hot_Time[1]; 
    unsigned char close_Volt[1]; 
    unsigned char alarm_Switch[1]; 
    unsigned char alarm_Id[4]; 

    day_Start[0] = atoi(vStr[0].c_str());
    day_End[0] = atoi(vStr[1].c_str());
    span_Heart[0] = atoi(vStr[2].c_str());
    span_Work[0] = atoi(vStr[3].c_str());
    hot_Temp[0] = atoi(vStr[4].c_str());;
    hot_Time[0] = atoi(vStr[5].c_str());
    int  volt = (int) (atof(vStr[6].c_str()) * 10);
    close_Volt[0] = volt;
    alarm_Switch[0] = atoi(vStr[7].c_str());
    if (vStr[8] == "0")
    {
      Str2X("FFFFFFFF", 4, alarm_Id);
    }
    printf("cam cfg is :%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",Day_Start[0],Day_End[0],Span_Heart[0],Span_Work[0],Hot_Temp[0],
      Hot_Time[0],Close_Volt[0],Alarm_Switch[0]);

    printf("svr cfg is :%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n",day_Start[0],day_End[0],span_Heart[0],span_Work[0],hot_Temp[0],
      hot_Time[0],close_Volt[0],alarm_Switch[0]);


    if(day_Start[0] != Day_Start[0] || day_End[0] != Day_End[0] || span_Heart[0] != Span_Heart[0]|| span_Work[0] != Span_Work[0]
      || hot_Temp[0] != Hot_Temp[0]|| hot_Time[0] != Hot_Time[0]|| close_Volt[0] != Close_Volt[0]|| alarm_Switch[0] != Alarm_Switch[0])
    {
      printf("update cfg \n");
      char * c_volt = (char*)malloc(50);
      char * c_alarm_id = (char*)malloc(50);
      sprintf(c_volt,"%d",volt);
      if (vStr[8] == "0")
      {
        sprintf(c_alarm_id,"FFFFFFFF");
      }
      else{
        sprintf(c_alarm_id,"%s",vStr[8].c_str());
      }

      send_require_config((char*)vStr[0].c_str(),(char*)vStr[1].c_str(),(char*)vStr[2].c_str(),(char*)vStr[3].c_str(),(char*)vStr[4].c_str(),
        (char*)vStr[5].c_str(),c_volt,(char*)vStr[7].c_str(),c_alarm_id);

    }


}

void bss::httpXPower(char*powerData,char*tempData,char*gpsData){
    if(resloved_sip == 0){
        try {
            tcp::resolver resolver(io_service_);
            tcp::resolver::query query("127.0.0.1", "1936");
            endpoint_iterator_sip = resolver.resolve(query);
        }
        catch (boost::system::system_error& e) {
            std::cout <<e.what()<<"\n"; 
             
            return;
        }
    }
    printf("httpXPower,bid is %s\n", (char*)Bid);
    resloved_sip = 1;
    char bid_[18];
    memset(bid_,'\0',18);
    memcpy(bid_,Bid,17);
    tcp::socket socket(io_service_);
    boost::system::error_code ec = boost::asio::error::host_not_found;;
    tcp::resolver::iterator end;
    tcp::resolver::iterator iterator = endpoint_iterator_sip;
    while(ec && iterator != end) {
        socket.close();
        socket.connect(*iterator++, ec);
    }
    static int sip_count = 3;
    if( ec ){
        // LOG("httpXPower connect failed,bid is %s,ec is %s\n",bid_,ec.message().c_str());
         resloved_sip = 0;
         
         if (--sip_count > 0)
         {
           httpXPower(powerData,tempData,gpsData);
         }
        return;
    }
    sip_count = 3;
    printf("-------------------------------hbt 1\n");
    std::ostringstream stringStream;

    std::string str_bid(bid_);
    
    stringStream <<"/svr/box.php?act=x&bid="<<str_bid<<"&pwr="<<powerData<<"&tem="<<tempData<<"&gps="<<gpsData;

    
    string  path_ = stringStream.str();
   
    cout<<path_<<endl;
    
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << path_ << " HTTP/1.0\r\n";
    request_stream << "Host: " << "127.0.0.1" << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    boost::asio::write(socket, request,ec);
    if( ec ){
        cout<<222<<endl;
       // resloved_sip =0;
        return;
    }

    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n",ec);
     if( ec ){
         //resloved_sip =0;
         return;
    }
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
        std::cout << "Invalid response\n";
       // resloved_sip =0;
        return ;
    }
    if (status_code != 200)
    {
        std::cout << "Response returned with status code " << status_code << "\n";
      //  resloved_sip =0;
        return ;
    }
    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n",ec);
     if( ec ){
     // resloved_sip =0;
         return;
    }
    //Process the response headers.
    std::string header;
    while (std::getline(response_stream, header) && header != "\r");
    //the heart-beat reply.
    std::string reply_;
    std::getline(response_stream ,reply_);

    while(boost::asio::read(socket, response, boost::asio::transfer_at_least(1), ec));

/* **************************************************************
 * heart-beat parser
 * ************************************************************** */
    printf("==reply is %s \n",reply_.c_str());
    socket.close();
    
}
void bss::X2Str(unsigned char* s_src, int n_len_src, char* s_dst){
  for(int i=0; i<n_len_src; i++){
    sprintf(s_dst+(i*2), "%02x", s_src[i]);
  }
}
void bss::Str2X(char* s_src, int n_len_dst, unsigned char* s_dst){
  char szTmp[3] = {0};
  for(int i=0; i<n_len_dst; i++){
    szTmp[0] = s_src[i*2];
    szTmp[1] = s_src[i*2+1];
    s_dst[i] = strtol(szTmp, NULL, 16);
  }
}


