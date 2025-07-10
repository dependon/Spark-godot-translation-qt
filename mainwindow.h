#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QProgressBar>
#include <QTextEdit>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QThread>
#include "translationworker.h"
#include <QStandardPaths>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QApplication>
#include <QDateTime>
#include <QMap>
#include <QHash>
#include <QList>
#include <QStringList>
#include <QRegularExpression>
#include <QTextStream>
#include <QFile>
#include "appobject.h"

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    //保存百度翻译appid和Secret Key到配置文件
    void on_btn_saveSetting_clicked();

    //选择csv文件
    void on_btn_select_clicked();

    //edit_filePath修改了如果是csv文件需要更新目标语言和读取到缓存中
    void on_edit_filePath_textChanged(const QString &arg1);

    //全选翻译语言
    void on_btn_allSelect_clicked();

    //取消全选翻译语言
    void on_btn_cancelSelect_clicked();

    //清空界面显示的日志
    void on_btn_clearLog_clicked();

    //开始翻译
    void on_btn_start_clicked();

    //停止翻译
    void on_btn_stop_clicked();

    // 翻译进度更新
    void onTranslationProgress(int current, int total, const QString &currentText, const QString &translatedText, const QString &targetLang);
    void onTranslationFinished();
    void onTranslationError(const QString &error);
    void onLogMessage(const QString &message);

    //输出当前翻译过的内容
    void on_btn_saveCsv_clicked();

    // 隐藏/显示API ID
    void on_checkBox_idHide_stateChanged(int state);

    // 隐藏/显示API Key
    void on_checkBox_keyHide_stateChanged(int state);

private:

    void initializeUI();
    void loadSettings();
    void saveSettings();
    void setupLanguageCheckboxes();
    void loadCSVFile(const QString &filePath);
    void updateSourceLanguageCombo();
    void addLogMessage(const QString &message);
    void updateProgress(int value, const QString &text = "");
    void resetTranslationButtons();
    QStringList getSelectedTargetLanguages();
    void selectAllLanguages(bool select);
    
    // CSV相关方法
    QList<QStringList> parseCSV(const QString &filePath);
    void saveCSV(const QString &filePath, const QList<QStringList> &data);
    
    Ui::MainWindow *ui;
    QSettings *m_settings;
    QList<QStringList> m_csvData;
    QStringList m_csvHeaders;
    QList<QCheckBox*> m_languageCheckboxes;
    QThread *m_translationThread;
    TranslationWorker *m_translationWorker;
    
    // 支持的28种语言
    QMap<QString, QString> m_supportedLanguages;
    
    // 翻译状态
    bool m_isTranslating;
    int m_totalTranslations;
    int m_currentTranslation;
};

#endif // MAINWINDOW_H
