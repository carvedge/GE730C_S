# GE730C_S

GE730C_S是服务器端通信程序

Original Version 

初始版本

V1.1.0.170718_release_j 

增加截图功能

V1.2.0.170808_release_j

一键巡视唤醒处理

1.增加std::map<const char*,int> map_wake_count，用于存储每套设备的唤醒
次数；

2.设备在线，正常唤醒一次并删除唤醒命令。设备不在线或处于切换中，每两分
钟唤醒一次，最多执行三次，最后删除命令并清空一键巡视取图命令。