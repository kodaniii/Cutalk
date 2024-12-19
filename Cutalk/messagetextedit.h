#ifndef MESSAGETEXTEDIT_H
#define MESSAGETEXTEDIT_H

#include <QObject>
#include <QTextEdit>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMimeType>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QPainter>
#include <QVector>
#include "defs.h"


class MessageTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    //初始化MessageTextEdit最大高度60
    explicit MessageTextEdit(QWidget *parent = nullptr);

    ~MessageTextEdit();

    //从MessageTextEdit中提取消息，存储到QVector<MsgInfo> mGetMsgList
    QVector<MsgInfo> getMsgList();

    void insertFileFromUrl(const QStringList &urls);
signals:
    void send();

protected:
    //拖拽进入事件，如果是自身，忽略
    void dragEnterEvent(QDragEnterEvent *event);
    //拖拽放下事件，插入拖拽的文件或图片
    void dropEvent(QDropEvent *event);
    //按键处理，如果按下回车Enter，发送消息
    void keyPressEvent(QKeyEvent *e);

private:
    //插入图片，如果图片过大，按比例缩放
    void insertImages(const QString &url);
    //插入文本文件，检查文件大小和类型
    void insertTextFile(const QString &url);
    bool canInsertFromMimeData(const QMimeData *source) const;
    void insertFromMimeData(const QMimeData *source);

private:
    //判断文件是否为图片
    bool isImage(QString url);
    //插入消息到消息列表中
    void insertMsgList(QVector<MsgInfo> &list,QString flag, QString text, QPixmap pix);
    //从文本中读取URL列表
    QStringList getUrl(QString text);
    //获取文件图标及大小信息，在图标上添加文件名和大小
    QPixmap getFileIconPixmap(const QString &url);
    //获取文件大小，转换为KB、MB、GB格式
    QString getFileSize(qint64 size);

private slots:
    void textEditChanged();

private:
    //原始数据
    QVector<MsgInfo> mMsgList;
    QVector<MsgInfo> mGetMsgList;
};

#endif // MESSAGETEXTEDIT_H
