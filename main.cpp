#include "mainwindow.h"

#include <QApplication>
#include <QTextCodec>
#include <QDir>
#include <QStandardPaths>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QLibraryInfo>
#include <QFile>
#include <QIcon> // 添加QIcon头文件
#include "appobject.h"

#define INSTANCE_LOCK_PATH ".spark-godot-translation"

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    QString txt;
    QString timestampS = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh:mm:ss");
    switch (type) {
    case QtDebugMsg:
        txt = timestampS + QString(" Debug: %1").arg(msg);
        break;
    case QtInfoMsg:
        txt = timestampS + QString(" Info: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = timestampS + QString(" Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = timestampS + QString(" Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = timestampS + QString(" Fatal: %1").arg(msg);
        break;
    }
    emit app->sigDebug(txt);

    QString timestamp = QDateTime::currentDateTime().toString("yyyy_MM_dd");
#ifdef Q_OS_LINUX
    QString logFolder = QDir::homePath() + "/"+INSTANCE_LOCK_PATH +"/log";
    QDir().mkpath(logFolder); // 创建log文件夹

    QString logFileName = QString("%1/log/log_%2.txt").arg(QDir::homePath() + "/"+INSTANCE_LOCK_PATH ).arg(timestamp);
#else
    QString logFolder = "log";
    QDir().mkpath(logFolder); // 创建log文件夹

    QString logFileName = QString("%1/log/log_%2.txt").arg(QCoreApplication::applicationDirPath()).arg(timestamp);
#endif
    QFile outFile(logFileName);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&outFile);
    textStream << txt << endl;

    // 删除10天前的日志文件
    QDir logDir(logFolder);
    QStringList filters;
    filters << "log_*.txt";
    logDir.setNameFilters(filters);

    QFileInfoList fileList = logDir.entryInfoList();
    for (QFileInfo fileInfo: fileList) {
        QDateTime fileTime = fileInfo.birthTime();
        if (fileTime.daysTo(QDateTime::currentDateTime()) > 10) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
 
     qInstallMessageHandler(customMessageHandler);
     a.setWindowIcon(QIcon(":/icon.png")); // 设置应用程序图标

    // 设置应用程序信息
    QApplication::setApplicationName("Spark Godot Translation");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("Spark Studio");
    QApplication::setOrganizationDomain("spark-studio.com");

    // 设置UTF-8编码
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // 加载QSS样式表
    QFile styleFile(":/styles.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&styleFile);
        QString styleSheet = stream.readAll();
        a.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qWarning() << u8"警告: 无法加载样式表文件 styles.qss";
    }

    // 创建主窗口
    MainWindow w;
    w.show();


    // 详细的SSL诊断信息
    qDebug() << u8"Qt版本:" << QT_VERSION_STR;
    qDebug() << u8"Qt库路径:" << QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    qDebug() << u8"OpenSSL构建版本:" << QSslSocket::sslLibraryBuildVersionString();
    qDebug() << u8"OpenSSL运行时版本:" << QSslSocket::sslLibraryVersionString();
    qDebug() << u8"OpenSSL支持情况:" << QSslSocket::supportsSsl();
    
    if (!QSslSocket::supportsSsl()) {
        qWarning() << u8"警告: OpenSSL不可用，HTTPS请求可能失败";
        qWarning() << u8"请确保OpenSSL库文件在系统PATH中或Qt安装目录中";
        qWarning() << u8"对于Qt 5.12.4，需要OpenSSL 1.1.x版本的库文件";
    } else {
        qDebug() << u8"SSL配置成功，可以进行HTTPS请求";
        
        // 设置默认SSL配置
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        QSslConfiguration::setDefaultConfiguration(sslConfig);
        qDebug() << u8"已设置默认SSL配置";
    }
    
    return a.exec();
}
