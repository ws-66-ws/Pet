#include "petwidget.h"
#include "ui_petwidget.h"

#include <QThreadPool>
#include "path.h"
#include <QFile>
#include <QRandomGenerator>
#include <QSettings>

PetWidget::PetWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PetWidget)
{
    ui->setupUi(this);

    isDragging = false; // 窗口默认不能拖拽

    setWindow();    // 设置窗口

    setSysTray();   // 设置系统托盘

    // 加载动画
    movie = new QMovie(":/map/default_await.gif");  // 从资源加载(默认待机动画)
    ui->pet_l->setMovie(movie);
    movie->start();
    isMoving = false;   // 移动动画默认没移动
    
    // 连接动画完成信号
    connect(movie, &QMovie::finished, this, &PetWidget::onAnimationFinished);

    // 创建待机切换定时器（单次触发）
    idleTimer = new QTimer(this);
    idleTimer->setSingleShot(true);
    connect(idleTimer, &QTimer::timeout, this, &PetWidget::switchIdleAnimation);

    // 启动待机定时器 (随机间隔(3~10s))
    int initInterval = 3000 + QRandomGenerator::global()->bounded(10001);
    idleTimer->start(initInterval);

    currentMoveAnimation = nullptr;

    moveTimer = new QTimer(this);
    moveTimer->setSingleShot(true); // 移动定时器为单次触发
    connect(moveTimer, &QTimer::timeout, this, [this](){
        if (m_randomMoveEnabled) {
            petRandomMove();
        }
    });

    // 初始化聊天对话框
    chatDialog = new MyChatInputDialog(this);
    // 从配置文件读取API密钥
    QFile file(":/apiConfig.txt");//用QFile类型存放文件路径
    if(file.open(QIODevice::ReadOnly))//用只读的形式打开文件，打开成功返回ture，打开失败返回flase
    {
        QByteArray baData = file.readAll();//把文件所有的内容读出来，并用QByteArray(字节数组)类型存储
        QString apiKey = baData.toStdString().c_str();//转换成字符串
        qDebug() << apiKey;//打印
        file.close();
        chatDialog->setApiKey(apiKey);
    }
    else {
        qWarning() << "API key not found in config file. Please set it in PetConfig.ini";
    }

    // 初始化随机移动标志
    m_randomMoveEnabled = false;
}

PetWidget::~PetWidget()
{
    delete chatDialog;
    delete ui;
}

PetWidget &PetWidget::getInstance()
{
    static PetWidget instance;
    return instance;
}

void PetWidget::setWindow()
{
    // 设置窗口标志：无边框、置顶
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    // 设置透明背景
    setAttribute(Qt::WA_TranslucentBackground);
    // 初始化位置（屏幕右下角）
    QRect screenRect = getScreenGeometry();
    if (!screenRect.isNull()) {
        move(screenRect.width() - width(), screenRect.height() - height());
    }
}

void PetWidget::setSysTray()
{
    // 创建托盘图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/map/sysmap.png"));  // 设置图标
    trayIcon->setToolTip("初音桌宠");

    // 显示托盘图标
    trayIcon->show();

    // 创建菜单
    trayMenu = new QMenu(this);

    // 添加动作
    chatAction = new QAction("聊天", this);
    moveAction = new QAction("随机移动", this);
    stopAction = new QAction("停止", this);
    QAction *closeAction = new QAction("关闭", this);
    // closeAction->setShortcut(Qt::ALT + Qt::Key_F4);  // 设置快捷键

    // 将动作加入菜单
    trayMenu->addAction(chatAction);
    trayMenu->addAction(moveAction);
    trayMenu->addAction(stopAction);
    trayMenu->addSeparator();               // 添加分隔线
    trayMenu->addAction(closeAction);

    // 设置托盘图标的上下文菜单
    trayIcon->setContextMenu(trayMenu);

    // 连接信号槽
    connect(chatAction, &QAction::triggered, this, [this](){
        switchChatAnimation();
        chatDialog->show();
    });
    connect(moveAction, &QAction::triggered, this, [this](){
        if (!isMoving) {
            m_randomMoveEnabled = true;
            petRandomMove();  // 立即执行第一次移动
        }
    });
    connect(stopAction, &QAction::triggered, this, &PetWidget::petStop);
    connect(closeAction, &QAction::triggered, this, &PetWidget::close);

    // 初始化菜单项的启用状态
    updateMenuActions();
}

void PetWidget::updateMenuActions()
{
    moveAction->setEnabled(!isMoving);   // 移动时不可用
    stopAction->setEnabled(isMoving);    // 空闲时不可用
}

void PetWidget::setAnimation(const QString &gifPath)
{
    if (!QFile::exists(gifPath)) {
        qWarning() << "GIF not found:" << gifPath;
        return;  // 或使用默认路径
    }
    movie->stop();                // 停止当前动画
    movie->setFileName(gifPath);   // 设置新文件
    if (movie->isValid()) {  // 成功加载文件
        movie->start();            // 启动新动画
    }
    else {
        qDebug() << "无效的 GIF 文件:" << gifPath;
    }
}

void PetWidget::petRandomMove()
{
    if (isMoving)
        return;

    // 生成随机目标位置
    QPoint newPos = generateRandomPosition();

    updateAnimationDirection(newPos.x());

    startMoveAnimation(newPos);

    // 移动开始时暂停待机切换
    if (idleTimer->isActive())
        idleTimer->stop();
}

void PetWidget::petStop()
{
    if (!isMoving)
        return;

    // 关闭随机移动模式
    m_randomMoveEnabled = false;

    // 停止定时器
    if (moveTimer && moveTimer->isActive()) {
        moveTimer->stop();
    }


    // 停止并清理当前动画
    if (currentMoveAnimation) {
        currentMoveAnimation->stop();          // 立即停止动画
        // 注意：由于动画设置了 DeleteWhenStopped，停止后会自动删除
        // 但我们需要将指针置空（动画删除时会通过 destroyed 信号置空，但为了立即生效也可以手动置空）
        currentMoveAnimation = nullptr;        // 指针不再有效
    }

    // 恢复空闲动画
    setAnimation(":/map/default_await.gif");

    // 更新状态
    isMoving = false;
    updateMenuActions();

    // 启动待机切换定时器
    int interval = 3000 + QRandomGenerator::global()->bounded(10001);
    idleTimer->start(interval);
}

void PetWidget::switchIdleAnimation()
{
    // 如果正在移动，不切换，但重新启动定时器等待
    if (isMoving) {
        int interval = 3000 + QRandomGenerator::global()->bounded(10001);
        idleTimer->start(interval);
        return;
    }

    // 创建任务类对象
    Path *myPath = new Path(this);

    connect(this, &PetWidget::starting, myPath, &Path::recv);

    connect(myPath, &Path::finish, this, [this](QString path){
        setAnimation(path);
    });

    emit starting(ENUM_ACTION_AWAIT);
    QThreadPool::globalInstance()->start(myPath);

    // 重新启动定时器
    int interval = 3000 + QRandomGenerator::global()->bounded(10001);
    idleTimer->start(interval);
}

void PetWidget::switchChatAnimation()
{
    // 停止当前动画
    if (movie->state() == QMovie::Running) {
        movie->stop();
    }
    
    // 设置聊天动画（使用默认待机动画作为聊天动画）
    setAnimation(":/map/default_await.gif");
}

void PetWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPosition().toPoint() - pos();
        isDragging = true;

        if (isMoving) {
            petStop();  // 这会更新状态并禁用相关按钮
        }
        event->accept();
    }
    else if (event->button() == Qt::RightButton) {    // 右键显示菜单
        trayMenu->popup(event->globalPosition().toPoint());
        event->accept();
    }
}

void PetWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && (event->buttons() & Qt::LeftButton)) {
        move((event->globalPosition() - dragPosition).toPoint());
        event->accept();
    }
}

void PetWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        event->accept();
    }
}

QPoint PetWidget::generateRandomPosition() const
{
    // 获取屏幕大小
    QRect screenRect = getScreenGeometry();

    if (screenRect.isNull())
        return pos();

    // 生成随机新位置，确保不超出屏幕且与当前位置有一定距离
    QPoint currentPos = pos();
    QPoint newPos;
    int attempts = 0;
    const int maxAttempts = 10;
    const int minDistance = 50; // 最小移动距离

    do {
        int newX = QRandomGenerator::global()->bounded(screenRect.width() - width());
        int newY = QRandomGenerator::global()->bounded(screenRect.height() - height());
        newPos = QPoint(newX, newY);
        attempts++;
    } while (QLineF(currentPos, newPos).length() < minDistance && attempts < maxAttempts);

    return newPos;
}

void PetWidget::updateAnimationDirection(int newX)
{
    // 创建任务类对象
    Path *myPath = new Path(this);

    connect(this, &PetWidget::starting, myPath, &Path::recv);

    connect(myPath, &Path::finish, this, &PetWidget::setAnimation);

    // 判断newX在m_x左边还是右边
    if(newX <= x())  // 左移
    {
        qDebug() << "左移了";
        emit starting(ENUM_ACTION_WALK_LEFT);
    }
    else // 右移
    {
        qDebug() << "右移了";
        emit starting(ENUM_ACTION_WALK_RIGHT);
    }
    QThreadPool::globalInstance()->start(myPath);
}

void PetWidget::startMoveAnimation(const QPoint &endPos)
{
    if (currentMoveAnimation) {
        currentMoveAnimation->stop();
        currentMoveAnimation->deleteLater();
        currentMoveAnimation = nullptr;
    }

    currentMoveAnimation = new QPropertyAnimation(this, "pos");
    currentMoveAnimation->setDuration(1500);
    currentMoveAnimation->setStartValue(pos());
    currentMoveAnimation->setEndValue(endPos);
    currentMoveAnimation->setEasingCurve(QEasingCurve::OutQuad); // 平滑移动

    connect(currentMoveAnimation, &QPropertyAnimation::finished, this, [this]() {
        isMoving = false;
        setAnimation(":/map/default_await.gif");  // 设置默认待机动画

        // 如果随机移动模式开启，启动下一次移动定时器
        if (m_randomMoveEnabled) {
            //  3~10秒移动一次
            int interval = 3000 + QRandomGenerator::global()->bounded(10001);
            moveTimer->start(interval);
        }

        // 重新启动待机切换定时器
        int idleInterval = 3000 + QRandomGenerator::global()->bounded(12001);
        idleTimer->start(idleInterval);

        // 注意：不要在这里设置currentMoveAnimation为nullptr，因为动画可能还没有被删除
        // 依赖destroyed信号来处理
    });
    connect(currentMoveAnimation, &QPropertyAnimation::destroyed, this, [this](QObject *obj) {
        if (currentMoveAnimation == obj)
            currentMoveAnimation = nullptr;
    });

    currentMoveAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    isMoving = true;
    updateMenuActions();
}

QRect PetWidget::getScreenGeometry() const
{
    QScreen *screen = QGuiApplication::primaryScreen();
    // screen->geometry();  // 屏幕完整尺寸(不排除任务栏等区域)
    return screen ? screen->availableGeometry() : QRect();  // 屏幕完整尺寸(排除任务栏等区域)
}

void PetWidget::onAnimationFinished()
{
    // 如果正在移动，不处理
    if (isMoving) {
        return;
    }
    
    // 检查当前动画是否是非默认待机动画
    QString currentFileName = movie->fileName();
    if (currentFileName != ":/map/default_await.gif" && 
        (currentFileName == ":/map/await_1.gif" || currentFileName == ":/map/await_2.gif")) {
        // 切换回默认待机动画
        setAnimation(":/map/default_await.gif");
    }
}

