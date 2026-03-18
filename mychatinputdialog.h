#ifndef MYCHATINPUTDIALOG_H
#define MYCHATINPUTDIALOG_H

#include <QWidget>
#include "aichatmanager.h"

namespace Ui {
class MyChatInputDialog;
}

class MyChatInputDialog : public QWidget
{
    Q_OBJECT

public:
    explicit MyChatInputDialog(QWidget *parent = nullptr);
    ~MyChatInputDialog();

    void setApiKey(const QString &apiKey);

private slots:
    void sendMessage();
    void onMessageReceived(const QString &response);
    void onErrorOccurred(const QString &error);

private:
    Ui::MyChatInputDialog *ui;
    AIChatManager *chatManager;
    bool isLoading;
    void appendMessage(const QString &sender, const QString &message);
    void showLoading();
    void hideLoading();
};

#endif // MYCHATINPUTDIALOG_H
