
#include "FileDetecate.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

#define READ_SIZE 100*1024

fileDetecate::fileDetecate(QObject *parent) : QObject(parent)
{

}

//遍历目录中所有文件 将文件名返回
QStringList fileDetecate::getFiles(const QString &path)
{
    QStringList ret;
    QDir dir(path);
    if(dir.isEmpty())
    {
        return QStringList();
    }
    //.表示当前目录  ..表示上一级目录
    QFileInfoList infoList = dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Files|QDir::Dirs);
    for(int i =0;i<infoList.count();i++)
    {
        QFileInfo info = infoList.at(i);
        if(info.isDir())
        {
            //如果是目录 就进入目录寻找文件
            QString subDir = info.absoluteFilePath();
            QStringList files = getFiles(subDir);

            ret.append(files);
        }
        else
        {
            //如果不是目录就加入ret中
            QString filePath = info.absoluteFilePath();
            ret.append(filePath);
        }
    }
    qDebug()<<"QStringList fileDetecate::getFiles(const QString &path)";
    return ret;
}

QStringList fileDetecate::filesOfDifferentSize(const QStringList &fileList)
{
    if(fileList.isEmpty())
    {
        return QStringList();
    }
    QStringList ret;
    QHash<qint64,QStringList> hash;
    for(int i=0;i<fileList.count();i++)
    {
        QString filePath = fileList.at(i);
        QFileInfo fileInfo(filePath);
        hash[fileInfo.size()].append(filePath);
    }
    QHash<qint64,QStringList>::iterator itr = hash.begin();
    for(itr;itr!=hash.end();itr++)
    {
        if(itr.value().count() > 1)
        {
            ret.append(itr.value());
        }
    }
    qDebug()<<"QStringList fileDetecate::filesOfDifferentSize(const QStringList &fileList)";
    return ret;
}

//计算文件MD5
QByteArray fileDetecate::getFilesMD5(const QString &path)
{
    if(path.isEmpty())
    {
        return QByteArray();
    }
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        QCryptographicHash hash(QCryptographicHash::Md5);
        while(!file.atEnd())
        {
            QByteArray fileContent = file.read(READ_SIZE);   //一次读取100K
            hash.addData(fileContent);
        }
        QByteArray md5 = hash.result().toHex();          //计算文件MD5

        file.close();
        return md5;
    }
    qDebug()<<"QByteArray fileDetecate::getFilesMD5(const QString &path)";
    return QByteArray();           //如果文件打开失败 返回空的QByteArray


}

void fileDetecate::duplicateFileDetect(const QString &path)
{
    if(path.isEmpty())
    {
        qDebug()<<"void fileDetecate::duplicateFileDetect(const QString &path):File is empty1";
        emit sendBtnEnabled();
        return;
    }
    QHash<QByteArray,QStringList> duplicateFiles;

    QStringList files = getFiles(path);
    if(files.isEmpty())
    {
        qDebug()<<"void fileDetecate::duplicateFileDetect(const QString &path):File is empty2";
        //所选文件夹为空，发送信号 使主窗口弹出messagebox
        emit sendMessageBox();
        emit sendBtnEnabled();
        return;
    }
    //过滤掉大小相同文件数量为1的文件
    QStringList filesFiltered = filesOfDifferentSize(files);

    if(filesFiltered.isEmpty())
    {
        qDebug()<<"void fileDetecate::duplicateFileDetect(const QString &path):File is empty3";
        emit sendProgress(100,100);
        emit sendBtnEnabled();
        return;
    }
    int count = filesFiltered.count();
    for(int i=0;i<count;i++)
    {
        QString filePath = filesFiltered.at(i);
        QByteArray md5 = getFilesMD5(filePath);

        duplicateFiles[md5].append(filePath);

        emit sendProgress(i+1,count);
    }

    emit sendDuplicateFile(duplicateFiles);
    qDebug()<<"void fileDetecate::duplicateFileDetect(const QString &path)";
}
