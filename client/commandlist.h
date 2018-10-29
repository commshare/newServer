#ifndef __MONITERCONN_H__
#define __MONITERCONN_H__

#include "imconn.h"
#include "ServInfo.h"
#include "public_define.h"

void init_cmd_map(void);
void send_cmd(uint32_t CmdID,void* paras = NULL);


#endif //__MONITERCONN_H__

