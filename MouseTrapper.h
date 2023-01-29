#pragma once

#include "ui_MouseTrapper.h"
#include "Plugins/QHotkey/qhotkey.h"
#include <QtWidgets/QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QDesktopServices>

#include <windows.h>


// Reference https://gist.github.com/kaimingguo/f812ab420772c7f09e8bfd733c849345
struct ScreenArray
{
    QList<RECT> m_monitorInfos;

    static BOOL CALLBACK MonitorEnumProc(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM data) {
        MONITORINFOEX mi = { 0 };

        mi.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfo(monitor, &mi);

        if (mi.dwFlags == DISPLAY_DEVICE_MIRRORING_DRIVER) 
        {
            // Using mirroring driver for multi-screen.
        }
        else 
        {
            auto p = reinterpret_cast<ScreenArray*>(data);
            p->m_monitorInfos.push_front(*rect);
        }
        return TRUE;
    }

    void UpdateMonitorInfo() 
    {
        m_monitorInfos.clear();
        EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)this);
    }
};

class MouseTrapper : public QMainWindow
{
    Q_OBJECT

public:
    MouseTrapper(QWidget *parent = nullptr);
    ~MouseTrapper();
    
    void UpdateTrapButtonColor();

public slots:
    void QuitApplication();

    void UpdateMouse();
    void UpdateMonitorInfo();
    void PressedTrapMouse();
    void ActivatedSystemTray(QSystemTrayIcon::ActivationReason reason);

protected:
    void closeEvent(QCloseEvent* event);
    void hideEvent(QHideEvent* event);

private:
    Ui::MouseTrapperClass ui;
    bool m_isTrapping = false;
    ScreenArray m_screenArray;

    QHotkey* m_trapHotkey;
    QTimer* m_tickTimer;

    QSystemTrayIcon m_systemTray;
    QMenu m_trayMenu;
};
