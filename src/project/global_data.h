/*
 * @Author: hanakami
 * @Date: 2025-05-08 17:08:00
 * @email: hanakami@163.com
 * @LastEditTime: 2025-05-28 17:22:11
 * @FilePath: /hana_frame/src/project/global_data.h
 * @Description: 全局数据
 * Copyright (c) 2025 by hanakami, All Rights Reserved.
 */

#ifndef _GLOBAL_DATA_H_
#define _GLOBAL_DATA_H_

#include "struct.h"

#include <core/uieventsource.h>
#include <core/preferences.h>

#define g_data globalData::ins()

class globalData :public MessageHandler {
public:
    uint8_t     mNetWork = 0;             // 网络状态
    uint8_t     mNetWorkDetail = 0;       // 网络详细状态

public: // 涂鸦部分
    bool        mTUYAPower = true;        // 电源状态
    int8_t      mTUYATem = 0;             // 涂鸦温度
    int8_t      mTUYATemMin = 0;          // 涂鸦温度最小值
    int8_t      mTUYATemMax = 0;          // 涂鸦温度最大值
    std::string mTUYAWeather = "146";     // 涂鸦天气代码
    uint16_t    mWifiTestRes = 0xFFFF;    // wifi测试结果

public: // 设备信息
    bool        mPower = false;           // 开关机
    bool        mLock = false;            // 童锁
private:
    enum {
        MSG_SAVE,
    };

    Looper* mLooper;
    bool             mHaveChange;
    uint64_t         mNextBakTime;

    Message          mSaveMsg;
    uint64_t         mPowerOnTime;  // 启动时间

private:
    globalData() = default;
    void update();
public:
    ~globalData();
    static globalData* ins() {
        static globalData s_globalData;
        return &s_globalData;
    }
    // 防止拷贝和移动
    globalData(globalData&) = delete; //拷贝构造
    globalData& operator=(globalData&) = delete;   //拷贝赋值
    globalData(globalData&&) = delete;  //移动构造
    globalData& operator=(globalData&&) = delete;  //移动赋值
    void handleMessage(Message& message)override;

    void init();

    uint64_t getPowerOnTime();
private:
    /*本地信息*/
    bool loadFromFile();
    bool saveToFile(bool isBak = false);
public:
};

#endif // _GLOBAL_DATA_H_