#ifndef AICHATMANAGER_H
#define AICHATMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QList>
#include <QDateTime>

class AIChatManager : public QObject
{
    Q_OBJECT

public:
    struct Message {
        enum Role {
            User,
            Assistant
        } role; // 标识消息发送者（用户或AI助手）
        QString content;    // 消息文本内容
        QDateTime timestamp;    // 消息时间戳，记录消息产生的时间
    };

    explicit AIChatManager(QObject *parent = nullptr);
    ~AIChatManager();

    void setApiKey(const QString &apiKey);  // 设置API密钥
    void sendMessage(const QString &message);   // 发送用户消息并触发API调用
    QList<Message> getMessages() const; // 获取当前对话历史
    void clearMessages();   // 清空对话历史

signals:
    void messageReceived(const QString &response);  // 成功收到AI回复时发射，携带回复内容
    void errorOccurred(const QString &error);   // 发生错误时发射，携带错误描述

private slots:
    void onNetworkReplyFinished(QNetworkReply *reply);  // 处理网络请求完成事件

private:
    QNetworkAccessManager *networkManager;  // 负责网络请求
    QString apiKey; // 智谱AI的API密钥
    QList<Message> messages;    // 存储对话历史
    
    void processResponse(const QJsonObject &response);  // 解析API返回的JSON数据
};

#endif // AICHATMANAGER_H
