#include "MouseTrapper.h"
#include <QCloseEvent>
#include <QHideEvent>

#define DEFAULT_DELAY 100

MouseTrapper::MouseTrapper(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    m_tickTimer = new QTimer(this);
    m_tickTimer->start(DEFAULT_DELAY);
    m_trapHotkey = new QHotkey(QKeySequence(Qt::CTRL + Qt::Key_F11), true, this);

    m_systemTray.setIcon(QIcon(QPixmap(":/MouseTrapper/icon.ico")));
    m_systemTray.setToolTip("MouseTrapper");
    m_systemTray.show();
    m_trayMenu.addAction("Exit", [](...) {
        QApplication::quit();
    });
    setWindowIcon(QIcon(QPixmap(":/MouseTrapper/icon.ico")));

    connect(m_tickTimer, &QTimer::timeout, this, &MouseTrapper::UpdateMouse);
    connect(ui.m_monitorInfoBtn, &QPushButton::pressed, this, &MouseTrapper::UpdateMonitorInfo);
    connect(ui.m_trapMouseBtn, &QPushButton::pressed, this, &MouseTrapper::PressedTrapMouse);
    connect(m_trapHotkey, &QHotkey::activated, this, &MouseTrapper::PressedTrapMouse);
    connect(&m_systemTray, &QSystemTrayIcon::activated, this, &MouseTrapper::ActivatedSystemTray);

    UpdateMonitorInfo();
    UpdateTrapButtonColor();
}

MouseTrapper::~MouseTrapper()
{
    delete m_trapHotkey;
    delete m_tickTimer;
}

void MouseTrapper::UpdateMouse()
{
    POINT mousePos;
    if (!GetCursorPos(&mousePos))
        return;

    ui.m_mousePosLabel->setText(QString("Mouse Position(%1, %2)").arg(mousePos.x).arg(mousePos.y));

    if (!m_isTrapping)
        return;

    // Merge valid rect
    QRect screenRect;
    for (size_t i = 0; i < ui.m_monitorInfoListWidget->count(); i++)
    {
        if (ui.m_monitorInfoListWidget->item(i)->checkState() != Qt::CheckState::Checked)
            continue;

        RECT& rect = m_screenArray.m_monitorInfos[i];
        if (screenRect.size().isNull())
        {
            screenRect = QRect(rect.left, rect.top, rect.right, rect.bottom);
        }
        else
        {
            screenRect = QRect(qMin(screenRect.left(), rect.left), qMin(screenRect.top(), rect.top),
                qMax(screenRect.right(), rect.right), qMax(screenRect.bottom(), rect.bottom));
        }
    }
    
    // Clip cursor
    if (!screenRect.size().isNull() && !screenRect.contains(QCursor::pos()))
    {
        RECT clipPos;
        clipPos.left = screenRect.left();
        clipPos.top = screenRect.top();
        clipPos.right = screenRect.right();
        clipPos.bottom = screenRect.bottom();
        ClipCursor(&clipPos);
    }
}

void MouseTrapper::UpdateMonitorInfo()
{
    ui.m_monitorInfoListWidget->clear();
    m_screenArray.UpdateMonitorInfo();
    if (m_screenArray.m_monitorInfos.isEmpty())
        return;

    for (size_t i = 0; i < m_screenArray.m_monitorInfos.size(); i++)
    {
        RECT& rect = m_screenArray.m_monitorInfos[i];
        QString itemStr = QString("Monitor %1 - Top Left (%2, %3) Bottom Right (%4, %5)").arg(i + 1)
            .arg(rect.left).arg(rect.top).arg(rect.right).arg(rect.bottom);
        ui.m_monitorInfoListWidget->addItem(itemStr);

        QListWidgetItem* item = ui.m_monitorInfoListWidget->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
    }
}

void MouseTrapper::PressedTrapMouse()
{
    m_isTrapping = !m_isTrapping;

    if (m_isTrapping)
    {
        ui.m_trapMouseBtn->setText("Trapped");
    }
    else
    {
        ui.m_trapMouseBtn->setText("Trap Mouse");
        ClipCursor(NULL);
    }
    ui.m_monitorInfoListWidget->setDisabled(m_isTrapping);
    UpdateTrapButtonColor();
}

void MouseTrapper::UpdateTrapButtonColor()
{
    if (m_isTrapping)
    {
        QPalette pal = ui.m_trapMouseBtn->palette();
        pal.setColor(QPalette::Button, QColor(Qt::green));
        ui.m_trapMouseBtn->setAutoFillBackground(true);
        ui.m_trapMouseBtn->setPalette(pal);
        ui.m_trapMouseBtn->update();
    }
    else
    {
        QPalette pal = ui.m_trapMouseBtn->palette();
        pal.setColor(QPalette::Button, QColor(Qt::red));
        ui.m_trapMouseBtn->setAutoFillBackground(true);
        ui.m_trapMouseBtn->setPalette(pal);
        ui.m_trapMouseBtn->update();
    }
}

void MouseTrapper::closeEvent(QCloseEvent* event)
{
    if (!isHidden())
    {
        hide();
        event->ignore();
    }
}
void MouseTrapper::hideEvent(QHideEvent* event)
{
    if (!isHidden())
    {
        hide();
        event->ignore();
    }
}

void MouseTrapper::QuitApplication()
{
    m_systemTray.hide();
}

void MouseTrapper::ActivatedSystemTray(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
    {
        showNormal();
    }
    else if (reason == QSystemTrayIcon::Context) 
    {
        m_trayMenu.popup(QCursor::pos());
    }
}