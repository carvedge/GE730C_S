//
//  DebugLog.h
//  HikRtmp
//
//  Created by 彭闽 on 15/10/21.
//  Copyright © 2015年 Carvedge. All rights reserved.
//

#ifndef __LOG__
#define __LOG__

#include <stdio.h>

static FILE * pFile = fopen("log0.txt","a");
#define LOG(format, ...) \
   if (pFile) {\
    fprintf(pFile, "[%s@%s,%d,%ld] " format "\n", \
             __func__, __FILE__, __LINE__, time(NULL), ##__VA_ARGS__ ); \
    fflush(pFile);}
#endif