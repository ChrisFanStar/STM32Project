#include "task_main.h"
#include "oled.h"
#include "kk_rtc.h"
#include "kk_knob.h"

/* 常量定义 */
#define CURSOR_FLASH_INTERVAL 500  // 光标闪烁间隔(ms)
#define REFRESH_INTERVAL      200  // 主刷新间隔(ms)

/* 类型定义 */
typedef enum {
    STATE_NORMAL,   // 正常显示模式
    STATE_SETTING   // 时间设置模式
} DisplayState;

typedef enum {
    SET_YEAR = 0,
    SET_MONTH,
    SET_DAY,
    SET_HOUR,
    SET_MINUTE,
    SET_SECOND,
    SET_END
} SettingState;

typedef struct {
    uint8_t x;      // 列坐标 (0-15)
    uint8_t y;      // 行坐标 (0-7)
    uint8_t width;  // 字符宽度
} CursorPos;

/* 全局变量 */
static DisplayState display_state = STATE_NORMAL;
static SettingState setting_state = SET_YEAR;
static struct tm setting_time;
static uint8_t need_refresh = 1;  // 刷新标志位

/* 光标位置配置 */
static const CursorPos cursor_pos[] = {
        {3, 1, 4},  // Year (2025)
        {8, 1, 2},  // Month
        {11,1, 2},  // Day
        {2, 3, 2},  // Hour
        {5, 3, 2},  // Minute
        {8, 3, 2}   // Second
};

/* 星期显示 */
static const char* const weeks[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

/* 显示当前时间 */
void showTime(struct tm *time) {
    char buf[20];

    /* 日期显示 (2025-04-30) */
    sprintf(buf, "%04d-%02d-%02d", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday);
    OLED_ShowString(2, 1, buf, 16, 0);

    /* 时间显示 (12:28:00) */
    sprintf(buf, "%02d:%02d:%02d", time->tm_hour, time->tm_min, time->tm_sec);
    OLED_ShowString(2, 3, buf, 16, 0);

    /* 星期显示 (Wed) */
    uint8_t x = (128 - strlen(weeks[time->tm_wday]) * 8) / 2;
    OLED_ShowString(x/8, 5, weeks[time->tm_wday], 16, 0);
}

///* 闪烁光标 */
//void showCursor() {
//    static uint32_t last_tick = 0;
//    static uint8_t visible = 1;
//
//    if (HAL_GetTick() - last_tick >= CURSOR_FLASH_INTERVAL) {
//        last_tick = HAL_GetTick();
//        visible = !visible;
//    }
//
//    const CursorPos *pos = &cursor_pos[setting_state];
//    char str[5];
//
//    switch (setting_state) {
//        case SET_YEAR:  sprintf(str, "%04d", setting_time.tm_year + 1900); break;
//        case SET_MONTH: sprintf(str, "%02d", setting_time.tm_mon + 1); break;
//        case SET_DAY:   sprintf(str, "%02d", setting_time.tm_mday); break;
//        case SET_HOUR:  sprintf(str, "%02d", setting_time.tm_hour); break;
//        case SET_MINUTE:sprintf(str, "%02d", setting_time.tm_min); break;
//        case SET_SECOND:sprintf(str, "%02d", setting_time.tm_sec); break;
//        default: return;
//    }
//
//    OLED_ShowString(pos->x, pos->y, str, 16, visible ? 0 : 1);  // 反相显示光标
//    need_refresh = 1;
//}
// 删除未使用的变量

void showCursor() {
    static uint32_t last_tick = 0;
    static uint8_t visible = 1;

    // 每500ms切换一次光标可见性
    if (HAL_GetTick() - last_tick >= CURSOR_FLASH_INTERVAL) {
        last_tick = HAL_GetTick();
        visible = !visible;
        need_refresh = 1;
    }

    const CursorPos *pos = &cursor_pos[setting_state];
    char str[5];

    switch (setting_state) {
        case SET_YEAR:  sprintf(str, "%04d", setting_time.tm_year + 1900); break;
        case SET_MONTH: sprintf(str, "%02d", setting_time.tm_mon + 1); break;
        case SET_DAY:   sprintf(str, "%02d", setting_time.tm_mday); break;
        case SET_HOUR:  sprintf(str, "%02d", setting_time.tm_hour); break;
        case SET_MINUTE:sprintf(str, "%02d", setting_time.tm_min); break;
        case SET_SECOND:sprintf(str, "%02d", setting_time.tm_sec); break;
        default: return;
    }

    OLED_ShowString(pos->x, pos->y, str, 16, visible ? 0 : 1);  // 反相显示光标
}

/* 编码器回调函数 */
void onKnobForward() {
    if(display_state == STATE_SETTING) {
        need_refresh = 1;
        switch(setting_state) {
            case SET_YEAR:   setting_time.tm_year++;  break;
            case SET_MONTH:  setting_time.tm_mon  = (setting_time.tm_mon  + 1) % 12; break;
            case SET_DAY:    setting_time.tm_mday  = (setting_time.tm_mday  % 31) + 1; break;
            case SET_HOUR:   setting_time.tm_hour  = (setting_time.tm_hour  + 1) % 24; break;
            case SET_MINUTE: setting_time.tm_min  = (setting_time.tm_min  + 1) % 60; break;
            case SET_SECOND: setting_time.tm_sec  = (setting_time.tm_sec  + 1) % 60; break;
        }
    }
}

void onKnobBackward() {
    if(display_state == STATE_SETTING) {
        need_refresh = 1;
        switch(setting_state) {
            case SET_YEAR:   setting_time.tm_year  = (setting_time.tm_year  < 70) ? 70 : setting_time.tm_year  - 1; break;
            case SET_MONTH:  setting_time.tm_mon  = (setting_time.tm_mon  - 1 + 12) % 12; break;
            case SET_DAY:    setting_time.tm_mday  = (setting_time.tm_mday  - 2 + 31) % 31 + 1; break;
            case SET_HOUR:   setting_time.tm_hour  = (setting_time.tm_hour  - 1 + 24) % 24; break;
            case SET_MINUTE: setting_time.tm_min  = (setting_time.tm_min  - 1 + 60) % 60; break;
            case SET_SECOND: setting_time.tm_sec  = (setting_time.tm_sec  - 1 + 60) % 60; break;
        }
    }
}

void onKnobPressed() {
    if (display_state == STATE_NORMAL) {
        memcpy(&setting_time, KK_RTC_GetTime(), sizeof(struct tm));
        setting_state = SET_YEAR;
        display_state = STATE_SETTING;
    } else if (display_state == STATE_SETTING) {
        if (++setting_state >= SET_END) {
            KK_RTC_SetTime(&setting_time);
            display_state = STATE_NORMAL;
        }
    }
}


/* 主任务初始化 */
void MainTaskInit() {
    OLED_Init();
    KK_RTC_Init();
    Knob_Init();
    Knob_SetForwardCallback(onKnobForward);
    Knob_SetBackwardCallback(onKnobBackward);
    Knob_SetPressedCallback(onKnobPressed);

    // 初始化默认时间（2025-04-30 12:28:00 Wed）
    setting_time.tm_year  = 125;
    setting_time.tm_mon  = 3;
    setting_time.tm_mday  = 30;
    setting_time.tm_hour  = 12;
    setting_time.tm_min  = 28;
    setting_time.tm_sec  = 0;
    setting_time.tm_wday  = 3;
}

/* 主任务循环 */
void MainTask() {
    static uint32_t last_refresh = 0;
    uint32_t current_tick = HAL_GetTick();

    Knob_Loop();

    // 控制刷新率，每REFRESH_INTERVAL毫秒刷新一次
    if (current_tick - last_refresh >= REFRESH_INTERVAL || need_refresh) {
        last_refresh = current_tick;
        need_refresh = 0;

        OLED_NewFrame();

        if (display_state == STATE_NORMAL){
            struct tm* now = KK_RTC_GetTime();
            showTime(now);
        }else{
            showTime(&setting_time);
            showCursor();
        }

        OLED_ShowFrame();
    }
}

