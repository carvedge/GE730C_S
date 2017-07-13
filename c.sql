DROP TABLE IF EXISTS cts;
CREATE TABLE cts (
  id bigint NOT NULL AUTO_INCREMENT,
  bid varchar(16) not null default '',
  pm1 smallint NOT NULL DEFAULT 0,
  pm2 int NOT NULL DEFAULT 0,
  PRIMARY KEY (id)
)ENGINE=MyISAM DEFAULT CHARSET utf8;

DROP TABLE IF EXISTS cap;
CREATE TABLE cap (
  id int NOT NULL AUTO_INCREMENT,
  dte bigint NOT NULL unique,
  PRIMARY KEY (id)
)ENGINE=MyISAM DEFAULT CHARSET utf8;

DROP TABLE IF EXISTS gpm;
CREATE TABLE gpm (
  id int NOT NULL AUTO_INCREMENT,
  dte bigint NOT NULL unique,
  PRIMARY KEY (id)
)ENGINE=MyISAM DEFAULT CHARSET utf8;

DROP TABLE IF EXISTS alm;
CREATE TABLE alm (
  id int NOT NULL AUTO_INCREMENT,
  dte bigint NOT NULL unique,
  flg tinyint NOT NULL DEFAULT 0,
  cmt varchar(255) not null default '',
  PRIMARY KEY (id)
)ENGINE=MyISAM DEFAULT CHARSET utf8;

DROP TABLE IF EXISTS pwr;
CREATE TABLE pwr (
  id int NOT NULL AUTO_INCREMENT,
  dte bigint NOT NULL unique,
  dat varchar(255) not null default '',
  PRIMARY KEY (id)
)ENGINE=MyISAM DEFAULT CHARSET utf8;

DROP TABLE IF EXISTS rec;
CREATE TABLE rec (
  id int NOT NULL AUTO_INCREMENT,
  dte bigint NOT NULL unique,
  tme bigint NOT NULL unique,
  flg tinyint NOT NULL DEFAULT 0,
  PRIMARY KEY (id)
)ENGINE=MyISAM DEFAULT CHARSET utf8;

//ge730c

DROP TABLE IF EXISTS xts;
CREATE TABLE xts (
  id bigint NOT NULL AUTO_INCREMENT,
  bid varchar(17) not null default '',
  pm1 smallint NOT NULL DEFAULT 0,             //1:打开打开视频  5:截图 6:c版本唤醒 7:c版本配置
  pm2 int NOT NULL DEFAULT 0,                       
  d1 varchar(16) not null default '',
  d2 varchar(16) not null default '',
  d3 varchar(16) not null default '',
  d4 varchar(16) not null default '',
  d5 varchar(16) not null default '',
  d6 varchar(16) not null default '',
  d7 varchar(16) not null default '',
  d8 varchar(16) not null default '',
  d9 varchar(16) not null default '',
  PRIMARY KEY (id)
)ENGINE=MyISAM DEFAULT CHARSET utf8;

DROP TABLE IF EXISTS xwr;
CREATE TABLE xwr (
  id int NOT NULL AUTO_INCREMENT,
  dte bigint NOT NULL unique,
  pwr varchar(64) not null default '',
  tem varchar(8) not null default '',
  gps varchar(32) not null default '',
  PRIMARY KEY (id)
)ENGINE=MyISAM DEFAULT CHARSET utf8;


DROP TABLE IF EXISTS xox;
CREATE TABLE xox (
  id int NOT NULL AUTO_INCREMENT,
  bid varchar(16) not null unique,
  s1 tinyint not null default 0,              //电池
  s2 tinyint not null default 0,              //机芯
  s3 tinyint not null default 0,              //gps
  s4 tinyint not null default 0,              //太阳能
  s5 tinyint not null default 0,              //温度设备
  s6 tinyint not null default 0,              //发热状态
  PRIMARY KEY (id)
)ENGINE=MyISAM DEFAULT CHARSET utf8;