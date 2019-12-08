#ifndef FILEDETECATE_H
#define FILEDETECATE_H

#include <QObject>
#include <QHash>

class fileDetecate : public QObject
{
    Q_OBJECT
public:
    explicit fileDetecate(QObject *parent = nullptr);

    //获取文件夹中所有文件路径
    QStringList getFiles(const QString &path);
    QStringList filesOfDifferentSize(const QStringList &fileList);
    //计算文件MD5
    QByteArray getFilesMD5(const QString &path);

signals:

    void sendDuplicateFile(const QHash<QByteArray,QStringList> &duplicateFiles);

    void sendProgress(int current,int total);

    void sendBtnEnabled();

    void sendMessageBox();
public slots:

    void duplicateFileDetect(const QString &path);


};

#endif // FILEDETECATE_H
