/*
 * @Author: hanakami
 * @Date: 2025-05-08 17:08:00
 * @email: hanakami@163.com
 * @LastEditTime: 2025-05-09 09:37:07
 * @FilePath: /hana_frame/main.cc
 * @Description:
 * Copyright (c) 2025 by hanakami, All Rights Reserved.
 */

#include <cdlog.h>
#include <core/app.h>

#include "this_func.h"
#include "global_data.h"
#include "config_mgr.h"
#include "manage.h"
#include "conn_mgr.h"
#include "btn_mgr.h"
#include "tuya_mgr.h"

int main(int argc, const char* argv[]) {
    printProjectInfo(argv[0]);
    // printKeyMap();

#ifdef CDROID_X64
    setenv("DEV_MODE", "3", 1);   // 开发模式
    setenv("DEVICE_ID", "62741", 1);   // 设备ID
    setenv("SCREEN_SIZE", "1280*480", 1);   // 设置屏幕尺寸
#ifdef CDROID_SIGMA
    LOGE("IM HANA");
    setenv("FONTCONFIG_PATH", "/home/wzt/cdroid/apps/hisense_dy73/fonts", 1);
#endif
#endif

    App app(argc, argv);
    cdroid::Context* ctx = &app;
    g_data->init();
    g_config->init();
    g_windMgr->init();
    // g_connMgr->init();
    // g_btnMgr->init();
    // g_tuyaMgr->init();
    return app.exec();
}

