#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include<boost/function.hpp>
#include <boost/thread/thread.hpp>
#include "bsv.h"
using boost::asio::ip::tcp;
boost::asio::io_service io_service_;
void worker_thread()
{
    io_service_.run(); 
}

int main(int argc, char* argv[])
{
  
    int bp = 1940;
    bsv *b = new bsv(io_service_,bp);
    if (b->initValues ())
    {
    	if (b->conToDb())
    	{
    		b->startMoniter();
    	}
    }
    io_service_.run();
}
