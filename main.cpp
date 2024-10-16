#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Cutalk_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    //qss file path :/
    QFile qss(":/style/stylesheet.qss");
    if(qss.open(QFile::ReadOnly)){
        qDebug("open qss succeed");
        QString qssStyle = QLatin1String(qss.readAll());
        a.setStyleSheet(qssStyle);
        qss.close();
    }
    else{
        qDebug("open qss failed");
    }

    MainWindow w;
    w.show();
    return a.exec();
}
