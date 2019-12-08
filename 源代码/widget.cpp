#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QListWidgetItem>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //设置拖拽文件释放时可用
    this->setAcceptDrops(true);

    thread.start();
    filesMD5.moveToThread(&thread);


    qRegisterMetaType< QHash<QByteArray,QStringList> >("QHash<QByteArray,QStringList>");
    connect(&filesMD5,&fileDetecate::sendDuplicateFile,this,&Widget::getDuplicateFiles);

    connect(this,&Widget::sendFilePath,&filesMD5,&fileDetecate::duplicateFileDetect);

    connect(&filesMD5,&fileDetecate::sendProgress,this,&Widget::setProgressBar);

    //使按钮可用
    connect(&filesMD5,&fileDetecate::sendBtnEnabled,this,&Widget::setBtn);

    connect(&filesMD5,&fileDetecate::sendMessageBox,this,&Widget::setMessageBox);

}


Widget::~Widget()
{
    thread.quit();
    thread.wait();
    delete ui;
}

void Widget::on_pushButton_clicked()
{

    ui->listWidgetMD5->clear();
    ui->listWidgetFiles->clear();
    //获取文件夹路径
    QString path = QFileDialog::getExistingDirectory(this,"choose a folder",".");

    if(path.isEmpty())
    {
        return;
    }
    ui->lineEdit->setText(path);

    ui->progressBar->setValue(0);

    emit sendFilePath(path);

    ui->pushButton->setEnabled(false);
    qDebug()<<"void Widget::on_pushButton_clicked()";
}

void Widget::getDuplicateFiles(const QHash<QByteArray, QStringList> &duplicateFiles)
{
    ui->listWidgetMD5->clear();
    this->duplicateFiles = duplicateFiles;
    QHash<QByteArray,QStringList>::const_iterator itr = duplicateFiles.begin();
    for(itr;itr!=duplicateFiles.end();itr++)
    {
        qDebug()<<"MD5:"<<itr.key()<<"file:"<<itr.value().count();
        if(itr.value().count()>1)
        {
            ui->listWidgetMD5->addItem(itr.key());
        }
    }
}

void Widget::setProgressBar(int current, int total)
{
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(current);
    if(current==total)
    {
        ui->pushButton->setEnabled(true);
    }
}


void Widget::on_listWidgetMD5_currentTextChanged(const QString &currentText)
{
    ui->listWidgetFiles->clear();

    currentKey = currentText.toLocal8Bit();
    QStringList fileList = this->duplicateFiles[currentText.toLocal8Bit()];

    ui->listWidgetFiles->addItems(fileList);
}

void Widget::on_listWidgetFiles_itemDoubleClicked(QListWidgetItem *item)
{
    QString path = item->text();
    qDebug()<<path;
    path = path.section("/",0,-2);
    qDebug()<<path;
    QString folderPath = QString("%1%2").arg("file:///").arg(path);
    QDesktopServices::openUrl(QUrl(folderPath, QUrl::TolerantMode));
}

void Widget::setBtn()
{
    ui->pushButton->setEnabled(true);
}

void Widget::setMessageBox()
{
    QMessageBox::information(this,"folder empty","文件夹为空，请重新选择");
}

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void Widget::dropEvent(QDropEvent *event)
{
    if(ui->pushButton->isEnabled())
    {
        ui->listWidgetMD5->clear();
        ui->listWidgetFiles->clear();
        ui->progressBar->setValue(0);

        QList<QUrl> urls = event->mimeData()->urls();
        if(urls.isEmpty())
            return;
        QString path = urls.first().toLocalFile();
        qDebug()<<path;

        QFileInfo info(path);
        if(!info.isDir())
        {
            ui->lineEdit->clear();
            QMessageBox::information(this,"error","请选择一个文件夹");
            return;
        }
        ui->lineEdit->setText(path);

        emit sendFilePath(path);

        ui->pushButton->setEnabled(false);
    }
    else
    {
        qDebug()<<"Button can't be used";
        return;
    }

}

void Widget::on_deleteBtn_clicked()
{
    int currentRow = ui->listWidgetFiles->currentRow();
    qDebug()<<"currentRow:"<<currentRow;
    //如果没有选中任何item，则返回
    if(currentRow<0)
    {
        return;
    }
    QString path = ui->listWidgetFiles->currentItem()->text();
    if(path.isEmpty())
    {
        return;
    }
    qDebug()<<path;
    QFile::remove(path);
    duplicateFiles[currentKey].removeOne(path);
    QStringList fileList = this->duplicateFiles[currentKey];
    ui->listWidgetFiles->clear();
    ui->listWidgetFiles->addItems(fileList);
}

void Widget::on_openFolderBtn_clicked()
{
    int currentRow = ui->listWidgetFiles->currentRow();
    qDebug()<<"currentRow:"<<currentRow;
    //如果没有选中任何item，则返回
    if(currentRow<0)
    {
        return;
    }
    QString path = ui->listWidgetFiles->currentItem()->text();
    if(path.isEmpty())
    {
        return;
    }
    qDebug()<<path;
   // path = path.section("/",0,-2);
   // qDebug()<<path;
   // QString folderPath = QString("%1%2").arg("file:///").arg(path);
   // QDesktopServices::openUrl(QUrl(folderPath, QUrl::TolerantMode));

    QProcess process;
    path.replace("/", "\\");   // 只能识别 "\"
    QString cmd = QString("explorer.exe /select,\"%1\"").arg(path);
    qDebug() << cmd;
    process.startDetached(cmd);
}
