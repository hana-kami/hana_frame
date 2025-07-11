/*
 * @Author: hanakami
 * @Date: 2025-05-08 17:08:00
 * @email: hanakami@163.com
 * @LastEditTime: 2025-06-18 07:58:58
 * @FilePath: /hana_frame/src/windows/base.cc
 * @Description: 页面基类
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 */

#include <core/app.h>

#include "base.h"
#include "manage.h"

#include "this_func.h"
#include "conn_mgr.h"
#include "btn_mgr.h"


 /*
  *************************************** 基类 ***************************************
  */

PBase::PBase() {
    mContext = &App::getInstance();;
    mLooper = Looper::getMainLooper();
    mInflater = LayoutInflater::from(mContext);
    mLastTick = 0;
}

PBase::~PBase() {
    __delete(mRootView);
}

uint8_t PBase::getLang() const {
    return mLang;
}

View* PBase::getRootView() {
    return mRootView;
}

void PBase::callTick() {
    if (mAutoExit && SystemClock::uptimeMillis() - g_window->mLastAction > mAutoExit) {
        LOGI("auto exit");
        g_windMgr->goTo(PAGE_HOME, mAutoExitWithBlack);
    } else {
        onTick();
    }
}

void PBase::callAttach() {
    mIsAttach = true;
    onAttach();
}

void PBase::callDetach() {
    mIsAttach = false;
    onDetach();
}

void PBase::callReload() {
    onReload();
}

void PBase::callMsg(int type, void* data) {
    onMsg(type, data);
}

void PBase::callMcu(uint8_t* data, uint8_t len) {
    onMcu(data, len);
}

bool PBase::callKey(uint16_t keyCode, uint8_t evt) {
    LOGV("callKey -> keyCode:%d evt:%d", keyCode, evt);
    return onKey(keyCode, evt);
}

void PBase::callLangChange(uint8_t lang) {
    mLang = lang;
    onLangChange();
}

void PBase::callCheckLight(uint8_t* left, uint8_t* right) {
    onCheckLight(left, right);
}

View* PBase::findViewById(int id) {
    if (!mRootView)return nullptr;
    return mRootView->findViewById(id);
}

void PBase::onTick() {
}

void PBase::onAttach() {
}

void PBase::onDetach() {
}

void PBase::onReload() {
}

void PBase::onMsg(int type, void* data) {
}

void PBase::onMcu(uint8_t* data, uint8_t len) {
}

bool PBase::onKey(uint16_t keyCode, uint8_t evt) {
    return false;
}

void PBase::onLangChange() {
}

void PBase::onCheckLight(uint8_t* left, uint8_t* right) {
}

void PBase::setAutoBackToStandby(uint32_t time, bool withBlack) {
    mAutoExit = time * 1000;
    mAutoExitWithBlack = withBlack;
}

void PBase::setLangText(TextView* v, const Json::Value& value) {
    if (v == nullptr) LOGE("TextView is nullptr");
    else v->setText(jsonSafeGet(value, "null"));
}

/*
 *************************************** 弹窗 ***************************************
 */

PopBase::PopBase(std::string resource) :PBase() {
    mRootView = (ViewGroup*)mInflater->inflate(resource, nullptr);
}

PopBase::~PopBase() {
}

/*
 *************************************** 页面 ***************************************
 */

 /// @brief 
 /// @param resource 
PageBase::PageBase(std::string resource) :PBase() {
    int64_t startTime = SystemClock::uptimeMillis();
    mRootView = (ViewGroup*)mInflater->inflate(resource, nullptr);
    LOGI("Load UI[%s] cost:%lldms", resource.c_str(), SystemClock::uptimeMillis() - startTime);
}

/// @brief 析构
PageBase::~PageBase() {
}

/// @brief 初始化UI
void PageBase::initUI() {
    mInitUIFinish = false;
    initBase();
    getView();
    setAnim();
    setView();
    loadData();
    mInitUIFinish = true;
}