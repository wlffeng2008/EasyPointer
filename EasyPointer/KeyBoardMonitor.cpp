#include "KeyBoardMonitor.h"
#include "qdebug.h"
#include <Windows.h>
// 判断指定键是否按下
bool IsKeyPressed(int vkCode)
{
    // 取最高位，判断当前物理按键状态
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}

HHOOK g_hKeyboardHook = NULL;

// 底层键盘钩子回调
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT* pKeyData = (KBDLLHOOKSTRUCT*)lParam;

    if (nCode >= 0)
    {
        // WM_KEYDOWN = 按键按下
        if (wParam == WM_KEYDOWN)
        {
            DWORD vkCode = pKeyData->vkCode;
            qDebug() << "全局捕获按键 VK: " << vkCode;

            // 字母A-Z
            if (vkCode >= 'A' && vkCode <= 'Z')
                qDebug() << " 字符: " << (char)vkCode;


        }
    }
    // 传递钩子消息给下一个钩子，不拦截按键
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

// 安装全局键盘钩子
void InstallKeyboardHook()
{
    g_hKeyboardHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        GetModuleHandle(NULL),
        0
        );
    if (!g_hKeyboardHook)
    {
        qDebug() << "钩子安装失败，错误码：" << GetLastError() ;
    }
}

// 卸载钩子
void UninstallKeyboardHook()
{
    if (g_hKeyboardHook)
    {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }
}


KeyBoardMonitor::KeyBoardMonitor(QObject *parent)
    : QThread{parent}
{
    start();
}


void KeyBoardMonitor::run()
{
    qDebug() << "实时监听键盘，按ESC退出程序";

#if 0
    while (true)
    {
        // ESC退出
        //if (IsKeyPressed(VK_ESCAPE))
        //    break;

        // 检测字母A-Z (VK_A ~ VK_Z)
        for (int key = 'A'; key <= 'Z'; key++)
        {
            if (IsKeyPressed(key))
            {
                qDebug() << "按下字母键：" << (char)key ;
            }
        }

        // 常用功能键
        if (IsKeyPressed(VK_SPACE)) qDebug() << "按下 空格";
        if (IsKeyPressed(VK_SHIFT)) qDebug() << "按下 Shift";
        if (IsKeyPressed(VK_LSHIFT)) qDebug() << "按下 左Shift";
        if (IsKeyPressed(VK_RSHIFT)) qDebug() << "按下 右Shift";
        if (IsKeyPressed(VK_CONTROL)) qDebug() << "按下 Ctrl";
        if (IsKeyPressed(VK_MENU)) qDebug() << "按下 Alt";
        if (IsKeyPressed(VK_UP)) qDebug() << "按下 上方向键";

        Sleep(10); // 降低CPU占用，10ms轮询一次
    }
#else
    InstallKeyboardHook();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UninstallKeyboardHook();
#endif
}