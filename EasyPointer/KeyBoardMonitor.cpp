#include "KeyBoardMonitor.h"
#include "qdebug.h"
#include <Windows.h>
// 判断指定键是否按下
bool IsKeyPressed(int vkCode)
{
    return ((GetAsyncKeyState(vkCode) & 0x8000) != 0);
}

static KeyBoardMonitor *g_keyMon = nullptr;

static HHOOK g_hKbHook = NULL;
static HHOOK g_hMsHook = NULL;

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT* pKeyData = (KBDLLHOOKSTRUCT*)lParam;

    if (nCode >= 0)
    {
        emit g_keyMon->onInputData(nCode,wParam,lParam,false);
        int vkCode = pKeyData->vkCode;
        //qDebug() << "全局捕获按键 VK: " << vkCode;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            //if (vkCode >= 'A' && vkCode <= 'Z')
            //    qDebug() << "按下: " << (char)vkCode << vkCode;
            emit g_keyMon->onKeypress(vkCode,true);
        }
        if (wParam == WM_KEYUP|| wParam == WM_SYSKEYUP)
        {
            //if (vkCode >= 'A' && vkCode <= 'Z')
            //qDebug() << "松开: " << (char)vkCode << vkCode;
            emit g_keyMon->onKeypress(vkCode,false);
        }
    }

    return CallNextHookEx(g_hKbHook, nCode, wParam, lParam);
}

static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    MSLLHOOKSTRUCT* pMsData = (MSLLHOOKSTRUCT*)lParam;

    if (nCode >= 0)
    {
        quint32 message = wParam;

        static quint32 timestamp = 0;
        quint32 nMkey = 0;
        bool  bDbClk = false;
        bool  pressed = true;
        if(wParam == WM_LBUTTONUP)
        {
            nMkey = 0;
            pressed = false;
            if(pMsData->time - timestamp < 300)
            {
                bDbClk = true;
                message = WM_LBUTTONDBLCLK;
            }
            timestamp = pMsData->time;
        }

        if(wParam == WM_RBUTTONUP)
        {
            nMkey = 1;
            pressed = false;
            if(pMsData->time - timestamp < 300)
            {
                bDbClk = true;
                message = WM_RBUTTONDBLCLK;
            }
            timestamp = pMsData->time;
        }

        if(wParam == WM_MBUTTONUP)
        {
            nMkey = 2;
            pressed = false;
            if(pMsData->time - timestamp < 300)
            {
                bDbClk = true;
                message = WM_MBUTTONDBLCLK;
            }
            timestamp = pMsData->time;
        }

        emit g_keyMon->onMousepress(message,nMkey,pressed,bDbClk);
        emit g_keyMon->onInputData(nCode,message,lParam,true);
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || WM_MBUTTONDOWN)
        {
            // qDebug() << "鼠标按下: " << (char)vkCode << vkCode;
            // emit g_keyMon->onMousepress(vkCode,true);
        }
        if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || WM_MBUTTONUP)
        {
            // qDebug() << "鼠标松开: " << (char)vkCode << vkCode;
            // emit g_keyMon->onMousepress(vkCode,false);
        }
    }

    return CallNextHookEx(g_hMsHook, nCode, wParam, lParam);
}

// 安装全局键盘钩子
void InstallKeyboardHook()
{    
    g_hMsHook = SetWindowsHookEx(
        WH_MOUSE_LL,
        LowLevelMouseProc,
        GetModuleHandle(NULL),
        0
        );

    g_hKbHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        GetModuleHandle(NULL),
        0
        );

    return;

    // if (!g_hKbHook)
    // {
    //     qDebug() << "钩子安装失败，错误码：" << GetLastError() ;
    // }

    // RAWINPUTDEVICE rid[1];
    // rid[0].usUsagePage = 0x01;  // 通用桌面设备 (Generic Desktop Controls)
    // rid[0].usUsage = 0x02;      // 鼠标设备 (Mouse)
    // rid[0].dwFlags = RIDEV_INPUTSINK; // 允许在窗口失去焦点时也能接收数据
    // rid[0].hwndTarget = nullptr;   // 接收消息的窗口句柄

    // if (!RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE)))
    // {
    //     qDebug() << "注册原始输入设备失败，错误码：" << GetLastError() ;
    //     //MessageBox(NULL, L"注册原始输入设备失败！", L"错误", MB_OK);
    // }
}

// 卸载钩子
void UninstallKeyboardHook()
{
    if (g_hKbHook)
    {
        UnhookWindowsHookEx(g_hKbHook);
        g_hKbHook = NULL;
    }
    if (g_hMsHook)
    {
        UnhookWindowsHookEx(g_hMsHook);
        g_hMsHook = NULL;
    }
}


KeyBoardMonitor::KeyBoardMonitor(QObject *parent)
    : QThread{parent}
{
    g_keyMon = this;
    start();
}

void KeyBoardMonitor::DoStop()
{
    m_bExit = true;
    UninstallKeyboardHook();
    quit();
}


void KeyBoardMonitor::run()
{
    qDebug() << "实时监听键盘，按ESC退出程序";

#if 0
    while (true)
    {
        if(m_bExit) break;
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
        if(m_bExit) break;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    qDebug() <<  "UninstallKeyboardHook" ;
    UninstallKeyboardHook();
#endif
}