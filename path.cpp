#include "path.h"

#include <QRandomGenerator>

Path::Path(QObject *parent)
    : QObject{parent}, QRunnable()
{
    setAutoDelete(true);   //任务结束自动析构

    m_awaitPath << ":/map/await_1.gif"
                << ":/map/await_2.gif"; // 添加待机动画的路径

    m_walkLeftPath << ":/map/walk_left_1.gif"
                   << ":/map/walk_left_2.gif";  // 添加左走动画的路径

    m_walkRightPath << ":/map/walk_right_1.gif"
                    << ":/map/walk_right_2.gif";    // 添加右走动画的路径
}

void Path::recv(const ENUM_ACTION action)
{
    m_action = action;
}

QString Path::getActionPath(QStringList &list) const
{
    int index = QRandomGenerator::global()->bounded(list.size());
    return list.at(index);
}

void Path::run()
{
    QString path;
    switch (m_action) {
    case ENUM_ACTION_AWAIT:
        path = getActionPath(m_awaitPath);
        break;
    case ENUM_ACTION_WALK_LEFT:
        path = getActionPath(m_walkLeftPath);
        break;
    case ENUM_ACTION_WALK_RIGHT:
        path = getActionPath(m_walkRightPath);
        break;
    default:
        break;
    }
    emit(finish(path));
}
