#include "aichatmanager.h"
#include <QDateTime>
#include <QUrl>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QDebug>

AIChatManager::AIChatManager(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &AIChatManager::onNetworkReplyFinished);
}

AIChatManager::~AIChatManager()
{
    delete networkManager;
}

void AIChatManager::setApiKey(const QString &apiKey)
{
    this->apiKey = apiKey;
}

void AIChatManager::sendMessage(const QString &message)
{
    // 检查API密钥
    if (apiKey.isEmpty()) {
        emit errorOccurred("API Key is not set");
        return;
    }

    // 添加用户消息到消息列表
    Message userMessage;
    userMessage.role = Message::User;
    userMessage.content = message;
    userMessage.timestamp = QDateTime::currentDateTime();
    messages.append(userMessage);

    // 构建API请求
    QUrl url("https://open.bigmodel.cn/api/paas/v4/chat/completions");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey).toUtf8());

    // 构建请求体
    QJsonObject requestBody;
    requestBody["model"] = "glm-4.7-flash"; // 指定模型
    
    QJsonArray messagesArray;
    for (const Message &msg : messages) {
        QJsonObject messageObj;
        messageObj["role"] = (msg.role == Message::User) ? "user" : "assistant";
        messageObj["content"] = msg.content;
        messagesArray.append(messageObj);
    }
    requestBody["messages"] = messagesArray;

    QJsonDocument doc(requestBody);
    QByteArray data = doc.toJson();

    networkManager->post(request, data);
}

QList<AIChatManager::Message> AIChatManager::getMessages() const
{
    return messages;
}

void AIChatManager::clearMessages()
{
    messages.clear();
}

void AIChatManager::onNetworkReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred("JSON parsing error: " + parseError.errorString());
        reply->deleteLater();
        return;
    }
    if (doc.isObject()) {
        processResponse(doc.object());
    } else {
        emit errorOccurred("Invalid response format");
    }

    reply->deleteLater();
}

void AIChatManager::processResponse(const QJsonObject &response)
{
    if (response.contains("choices")) {
        QJsonArray choices = response["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices[0].toObject();
            if (choice.contains("message")) {
                QJsonObject messageObj = choice["message"].toObject();
                if (messageObj.contains("content")) {
                    QString result = messageObj["content"].toString();
                    
                    // 添加助手消息到消息列表
                    Message assistantMessage;
                    assistantMessage.role = Message::Assistant;
                    assistantMessage.content = result;
                    assistantMessage.timestamp = QDateTime::currentDateTime();
                    messages.append(assistantMessage);
                    
                    emit messageReceived(result);
                    return;
                }
            }
        }
    } else if (response.contains("result")) {
        // 兼容旧版API响应格式
        QString result = response["result"].toString();
        
        // 添加助手消息到消息列表
        Message assistantMessage;
        assistantMessage.role = Message::Assistant;
        assistantMessage.content = result;
        assistantMessage.timestamp = QDateTime::currentDateTime();
        messages.append(assistantMessage);
        
        emit messageReceived(result);
        return;
    } else if (response.contains("error")) {
        QJsonObject errorObj = response["error"].toObject();
        QString errorMsg = errorObj["message"].toString();
        emit errorOccurred(errorMsg);
        return;
    }
    emit errorOccurred("Unknown response format");
}
