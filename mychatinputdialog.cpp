#include "mychatinputdialog.h"
#include "ui_mychatinputdialog.h"

MyChatInputDialog::MyChatInputDialog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyChatInputDialog)
    , isLoading(false)
{
    ui->setupUi(this);
    
    // 设置窗口属性，确保对话框不会总是置顶
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    
    chatManager = new AIChatManager(this);
    
    connect(ui->send_pb, &QPushButton::clicked, this, &MyChatInputDialog::sendMessage);
    connect(chatManager, &AIChatManager::messageReceived, this, &MyChatInputDialog::onMessageReceived);
    connect(chatManager, &AIChatManager::errorOccurred, this, &MyChatInputDialog::onErrorOccurred);
    
    // 允许按Enter键发送消息
    connect(ui->input_le, &QLineEdit::returnPressed, this, &MyChatInputDialog::sendMessage);
}

MyChatInputDialog::~MyChatInputDialog()
{
    delete ui;
}

void MyChatInputDialog::setApiKey(const QString &apiKey)
{
    chatManager->setApiKey(apiKey);
}

void MyChatInputDialog::sendMessage()
{
    QString message = ui->input_le->text().trimmed();
    if (!message.isEmpty()) {
        appendMessage("我", message);
        showLoading();
        chatManager->sendMessage(message);
        ui->input_le->clear();
    }
}

void MyChatInputDialog::onMessageReceived(const QString &response)
{
    hideLoading();
    appendMessage("AI", response);
}

void MyChatInputDialog::onErrorOccurred(const QString &error)
{
    hideLoading();
    appendMessage("错误", error);
}

void MyChatInputDialog::appendMessage(const QString &sender, const QString &message)
{
    QString formattedMessage = QString("%1: %2\n").arg(sender).arg(message);
    ui->message_te->append(formattedMessage);
}

void MyChatInputDialog::showLoading()
{
    if (!isLoading) {
        isLoading = true;
        ui->send_pb->setText("发送中...");
        ui->send_pb->setEnabled(false);
        ui->input_le->setEnabled(false);
    }
}

void MyChatInputDialog::hideLoading()
{
    if (isLoading) {
        isLoading = false;
        ui->send_pb->setText("发送");
        ui->send_pb->setEnabled(true);
        ui->input_le->setEnabled(true);
    }
}
