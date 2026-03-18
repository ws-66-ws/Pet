#ifndef PETWIDGET_H
#define PETWIDGET_H

#include <QEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QWidget>
#include <QMovie>
#include <QTimer>
#include <QPropertyAnimation>
#include "myaction.h"
#include "mychatinputdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class PetWidget;
}
QT_END_NAMESPACE

class PetWidget : public QWidget
{
    Q_OBJECT

public:
    PetWidget(QWidget *parent = nullptr);
    ~PetWidget();

    static PetWidget &getInstance();// 单例

    void setWindow();   // 设置窗口

    void setSysTray();  // 设置系统托盘

    void updateMenuActions(); // 修改菜单按钮的 启用或禁用

    void setAnimation(const QString &gifPath);   // 设置动画

    void petRandomMove();  // 随机移动

    void petStop();     // 停止移动

    void switchIdleAnimation(); // 待机切换
    void switchChatAnimation(); // 聊天动画

signals:
    void starting(ENUM_ACTION action);

protected:
    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    //void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    Ui::PetWidget *ui;

    QSystemTrayIcon *trayIcon;  // 系统托盘

    QMenu *trayMenu;            // 菜单

    QAction *chatAction;    // 聊天
    QAction *moveAction;    // 随机移动
    QAction *stopAction;    // 停止移动

    QMovie *movie;          // 播放 GIF

    QPropertyAnimation *currentMoveAnimation;   // 用于保存当前正在运行的 移动 动画

    bool isMoving;      // 表示移动动画是否正在移动,默认不移动(false)

    QTimer *moveTimer;      // 用于随机移动的定时器

    bool m_randomMoveEnabled;   // 随机移动是否启动

    QPoint dragPosition;     // 用于拖拽,记录鼠标的位置

    bool isDragging;        // 表示鼠标是否可以拖拽,默认不能拖拽(false)

    QTimer *idleTimer;                  // 待机切换定时器

    MyChatInputDialog *chatDialog;  // 输入框

private slots:
    void onAnimationFinished();  // 处理动画完成事件

private:
    QPoint generateRandomPosition() const;  // 生成随机位置

    void updateAnimationDirection(int newX);    // 根据方向切换动画

    void startMoveAnimation(const QPoint &endPos);  // 创建并启动动画

    QRect getScreenGeometry() const;  // 获取屏幕几何
};
#endif // PETWIDGET_H
