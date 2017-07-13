/*
 * File:   ini.h
 * Author: tsxw24@gmail.com
 *
 * Created on 2013年3月18日, 下午2:51
 */

#ifndef INI_H
#define	INI_H


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <string>
using namespace std;
class Ini{
public:
	Ini();
    void setFileName(string);
    string get(string path);
    short int errCode();
private:
    short int err_code;
    boost::property_tree::ptree m_pt;
    string filename;
};
#endif	/* INI_H */
