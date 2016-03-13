#include "mainwindow.hpp"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QSettings>

void myMessageHandler(QtMsgType, const QMessageLogContext &, const QString& msg)
{
    QString txt;
    QDateTime date;

    txt = msg;
    txt.prepend(" - ");
    txt.prepend(date.currentDateTime().toString());

    QFile outFile("log.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts.setCodec("UTF-8");
    ts << txt <<"\r\n"<<flush;
}

int main(int argc, char *argv[])
{
    QSettings settings("tags.ini", QSettings::IniFormat);
    if (settings.value("debug.log", false).toBool())
    {
        qInstallMessageHandler(myMessageHandler);
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
