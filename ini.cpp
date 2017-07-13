#include "ini.h"


Ini::Ini(){
}
void Ini::setFileName(string fileName){
   filename = fileName;
    if (access(filename.c_str(), 0) == 0) {
        this->err_code = 0;
        boost::property_tree::ini_parser::read_ini(filename, this->m_pt);
    } else {
        this->err_code = 1;
    }
}
short Ini::errCode(){
    return this->err_code;
}

string Ini::get(string path){
    if (access(filename.c_str(), 0) == 0) {
        this->err_code = 0;
        boost::property_tree::ini_parser::read_ini(filename, this->m_pt);
    } else {
        this->err_code = 1;
    }
    
    if (this->err_code == 0) {
        return this->m_pt.get<string>(path);
    } else {
        return "";
    }
}
