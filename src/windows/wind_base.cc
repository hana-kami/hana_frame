/*
 * @Author: hanakami
 * @Date: 2025-05-08 17:08:00
 * @email: hanakami@163.com
 * @LastEditTime: 2025-05-26 14:03:45
 * @FilePath: /hana_frame/src/windows/wind_base.cc
 * @Description: 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 */


#include <core/app.h>
#include "manage.h"
#include "wind_base.h"
#include "comm_func.h"
#include "global_data.h"

#include "btn_mgr.h"
#include "config_mgr.h"
#include "defualt_config.h"

constexpr int POP_TICK_INTERVAL = 1000;  // 弹窗刷新间隔
constexpr int PAGE_TICK_INTERVAL = 200;  // 页面刷新间隔

constexpr int POPTEXT_TRANSLATIONY = 76;    // 弹幕的Y轴偏移量
constexpr int POPTEXT_ANIMATETIME = 600;    // 弹幕动画时间

/// @brief 点击时系统自动调用
/// @param sound 音量大小(一般不用)
static void playSound(int sound) {
    LOGV("bi~~~~~~~~~~~~~~~~~~~~~~~~");
}

/// @brief 构造以及基础内容检查
/// @return 返回BaseWindow对象
BaseWindow::BaseWindow() :Window(0, 0, -1, -1) {
    mLastAction = SystemClock::uptimeMillis();
    mLastTick = 0;
    mPopTickTime = 0;
    mPageTickTime = 0;

    mToastRunning = false;
    mToastLevel = -1;

    App::getInstance().addEventHandler(this);
}

BaseWindow::~BaseWindow() {
    removeCallbacks(mToastRun);
    mToast->animate().cancel();
    __delete(mPop);
}

int BaseWindow::checkEvents() {
    int64_t tick = SystemClock::uptimeMillis();
    if (tick >= mLastTick) {
        mLastTick = tick + 100;
        return 1;
    }
    return 0;
}

int BaseWindow::handleEvents() {
    int64_t tick = SystemClock::uptimeMillis();

    if (tick >= mPageTickTime) {
        mPageTickTime = tick + PAGE_TICK_INTERVAL;
        if (mPage)mPage->callTick();
    }

    if (tick >= mPopTickTime) {
        mPopTickTime = tick + POP_TICK_INTERVAL;
        if (mPop)mPop->callTick();

        checkToScreenSave();
    }

    return 1;
}

void BaseWindow::init() {
    mContext = getContext();
    mRootView = (ViewGroup*)(
        LayoutInflater::from(mContext)->inflate("@layout/wind_base", this)
        );
    mPageBox = __getgv(mRootView, ViewGroup, hana_frame::R::id::mainBox);
    mPopBox = __getgv(mRootView, ViewGroup, hana_frame::R::id::popBox);
    mLogo = __getg(mRootView, hana_frame::R::id::logo);
    mToast = __getgv(mRootView, TextView, hana_frame::R::id::toast);
    mBlackView = __getgv(mRootView, View, hana_frame::R::id::cover);

    if (!(mPageBox && mPopBox && mToast))
        throw std::runtime_error("BaseWindow Error");

    mPage = nullptr;
    mPop = nullptr;
    mIsShowLogo = false;
    mCloseLogo = [this] { mLogo->setVisibility(GONE);mIsShowLogo = false; };
    mPopBox->setOnTouchListener([](View& view, MotionEvent& evt) { return true; });

    mIsBlackView = false;
    mBlackView->setOnClickListener([this](View& view) { hideBlack(); });

    // 增加点击反馈
    mAttachInfo->mPlaySoundEffect = playSound;

    mToastRun = [this] {
        mToastLevel = -1;
        mToastRunning = false;
        mToast->animate().translationY(POPTEXT_TRANSLATIONY).setDuration(POPTEXT_ANIMATETIME).start();
    };

    showLogo();
}

/// @brief 显示 LOGO
/// @param time 
void BaseWindow::showLogo(uint32_t time) {
    setBrightness(g_config->getBrightness());
    removeCallbacks(mCloseLogo);
    mIsShowLogo = true;
    mLogo->setVisibility(VISIBLE);
    postDelayed(mCloseLogo, time);
}

/// @brief 键盘抬起事件
/// @param keyCode 
/// @param evt 
/// @return 
bool BaseWindow::onKeyUp(int keyCode, KeyEvent& evt) {
    return onKey(keyCode, VIRT_EVENT_UP) || Window::onKeyUp(keyCode, evt);
}

/// @brief 键盘按下事件
/// @param keyCode 
/// @param evt 
/// @return 
bool BaseWindow::onKeyDown(int keyCode, KeyEvent& evt) {
    return onKey(keyCode, VIRT_EVENT_DOWN) || Window::onKeyDown(keyCode, evt);
}

/// @brief 按键处理
/// @param keyCode 
/// @param status 
/// @return 是否需要响铃
bool BaseWindow::onKey(uint16_t keyCode, uint8_t status) {
    mLastAction = SystemClock::uptimeMillis();
    if (keyCode == KEY_WINDOW)return false;  // 刷新mLastAction用
    if (mIsShowLogo) return false;
    if (mIsBlackView) {
        if (status != VIRT_EVENT_DOWN)return false;
        hideBlack();
        return true;
    }
    if (selfKey(keyCode, status)) return true;
    if (mPop) return mPop->callKey(keyCode, status);
    if (mPage) return mPage->callKey(keyCode, status);
    return false;
}

/// @brief 重载触摸事件入口，方便计时最后一次触摸时间
/// @param evt 
/// @return 
bool BaseWindow::dispatchTouchEvent(MotionEvent& evt) {
    mLastAction = SystemClock::uptimeMillis();
    return Window::dispatchTouchEvent(evt);
}

/// @brief 获取当前页面指针
/// @return 
PageBase* BaseWindow::getPage() {
    return mPage;
}

/// @brief 获取当前弹窗指针
/// @return 
PopBase* BaseWindow::getPop() {
    return mPop;
}

/// @brief 获取当前页面类型
/// @return 
int8_t BaseWindow::getPageType() {
    return mPage ? mPage->getType() : PAGE_NULL;
}

/// @brief 获取当前弹窗类型
/// @return 
int8_t BaseWindow::getPopType() {
    return mPop ? mPop->getType() : POP_NULL;
}

/// @brief 显示黑屏
bool BaseWindow::showBlack(bool upload) {
    if (mIsBlackView)return true;
    setBrightness(CONFIG_BRIGHTNESS_MIN);
    mBlackView->setVisibility(VISIBLE);
    mIsBlackView = true;
    return true;
}

/// @brief 
void BaseWindow::hideBlack() {
    if (!mIsBlackView)return;
    g_windMgr->goTo(PAGE_HOME);
    mPage->callKey(KEY_WINDOW, VIRT_EVENT_UP);
    mBlackView->setVisibility(GONE);
    mIsBlackView = false;
    setBrightness(g_config->getBrightness());
}

/// @brief 显示页面
/// @param page 
void BaseWindow::showPage(PageBase* page) {
    removePage();
    mPage = page;
    if (mPage) {
        mPageBox->addView(mPage->getRootView());
        mPage->callAttach();
        mPage->callReload();
        mPage->callTick(); // 为了避免有些页面变化是通过tick更新的，导致页面刚载入时闪烁
    }
}

/// @brief 显示弹窗
/// @param pop 
void BaseWindow::showPop(PopBase* pop) {
    removePop();
    mPop = pop;
    if (mPop) {
        mPopBox->setVisibility(VISIBLE);
        mPopBox->addView(mPop->getRootView());
        mPop->callAttach();
        mPop->callReload();
        mPop->callTick(); // 为了避免有些页面变化是通过tick更新的，导致页面刚载入时闪烁
    }
}

/// @brief 显示弹幕
/// @param text 
/// @param time 
void BaseWindow::showToast(std::string text, int8_t level, bool animate, bool lock) {
    if (mToastRunning && level <= mToastLevel)return;

    removeCallbacks(mToastRun);
    mToastRunning = true;
    mToastLevel = level;
    mToast->setText(text);
    mToast->animate().cancel();
    if (animate) {
        mToast->setTranslationY(POPTEXT_TRANSLATIONY);
        mToast->animate().translationY(0).setDuration(POPTEXT_ANIMATETIME / 3 * 2).start();
    } else {
        mToast->setTranslationY(0);
    }
    if (!lock)
        postDelayed(mToastRun, 3000);
}

/// @brief 移除页面
void BaseWindow::removePage() {
    if (mPage) {
        mPage->callDetach();
        mPageBox->removeAllViews();
        mPage = nullptr;
    }
}

/// @brief 移除弹窗
void BaseWindow::removePop() {
    if (mPop) {
        mPopBox->setVisibility(GONE);
        mPop->callDetach();
        mPopBox->removeAllViews();
        __delete(mPop);
        mPop = nullptr;
    }
}

/// @brief 移除弹幕
void BaseWindow::hideToast() {
    mToastLevel = -1;
    mToastRunning = false;
    mToast->animate().cancel();
    mToast->setTranslationY(POPTEXT_TRANSLATIONY);
    removeCallbacks(mToastRun);
}

/// @brief 隐藏全部元素
void BaseWindow::hideAll() {
    mPopBox->setVisibility(GONE);
    mToast->setVisibility(GONE);
    mBlackView->setVisibility(GONE);
}

bool BaseWindow::selfKey(uint16_t keyCode, uint8_t status) {
    static uint64_t s_keyPowerDownTime = 0;
    uint64_t nowTime = SystemClock::uptimeMillis();
    switch (keyCode) {
    default:
        break;
    }
    return false;
}

void BaseWindow::checkToScreenSave() {
    uint64_t now = SystemClock::uptimeMillis();    // 当前时间
    uint8_t  nowPage = getPageType();              // 当前页面类型

}
/// @brief wind_base中控件更新
void BaseWindow::tabBarTick() {
    
}
