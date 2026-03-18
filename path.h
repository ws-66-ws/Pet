#ifndef PATH_H
#define PATH_H

#include <QObject>
#include <QRunnable>
#include "myaction.h"

class Path : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Path(QObject *parent = nullptr);

    void recv(const ENUM_ACTION action);

    QString getActionPath(QStringList &list) const; // 获取动作路径

    void run() override;

signals:
    void finish(const QString &path);

private:
    ENUM_ACTION m_action;   // 动作

    QStringList m_awaitPath;   // 待机动画路径列表

    QStringList m_walkLeftPath;   // 左走动画路径列表

    QStringList m_walkRightPath;   // 右走动画路径列表
};

#endif // PATH_H
