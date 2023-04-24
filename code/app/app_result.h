#pragma once

typedef int AppResult;

// AppResult codes

enum AppResultCodes : AppResult {
    APP_CODE_OK = 0,
    APP_CODE_UNSUPPORTED_OS,
    APP_CODE_WND_INIT_FAIURE,
    APP_CODE_VK_INIT_FAIURE,
    APP_CODE_VK_COMMAND_FAIURE,
    APP_CODE_DEV_ENUM_FAILED,
    APP_CODE_UNKNOWN = ~((AppResult)0)
};

#define CHECK_RESULT(res) (res == APP_CODE_OK)
