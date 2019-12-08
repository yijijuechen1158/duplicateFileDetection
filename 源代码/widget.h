#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "FileDetecate.h"
#include <QThread>
#include <QListWidgetItem>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pushButton_clicked();

    void getDuplicateFiles(const QHash<QByteArray,QStringList> &duplicateFiles);

    void setProgressBar(int current,int total);

    void on_listWidgetMD5_currentTextChanged(const QString &currentText);

    void on_listWidgetFiles_itemDoubleClicked(QListWidgetItem *item);

    void setBtn();

    void setMessageBox();

    void on_deleteBtn_clicked();

    void on_openFolderBtn_clicked();

protected:
    void dragEnterEvent(QDragEnterEvent *event);

    void dropEvent(QDropEvent *event);

signals:

    void sendFilePath(const QString &path);

private:
    Ui::Widget *ui;

    fileDetecate filesMD5;

    QThread thread;

    QHash<QByteArray, QStringList> duplicateFiles;
    QByteArray currentKey;
};

#endif // WIDGET_H
