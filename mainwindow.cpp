#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "translationworker.h"

// MainWindow 实现
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings(nullptr),
    m_translationThread(nullptr),
    m_translationWorker(nullptr),
    m_isTranslating(false),
    m_totalTranslations(0),
    m_currentTranslation(0)
{
    ui->setupUi(this);
    initializeUI();
    loadSettings();
    setupLanguageCheckboxes();
    
    // 启用拖拽
    setAcceptDrops(true);
    ui->edit_filePath->setAcceptDrops(false); // 禁用LineEdit的拖拽，使用MainWindow的
    connect(AppObject::instance(),&AppObject::sigDebug,this,&MainWindow::onLogMessage);
    ui->label_github->setTextFormat(Qt::RichText);
    ui->label_github->setText(u8"<a href='https://github.com/dependon/Spark-godot-translation-qt'>github</a>");
    ui->label_github->setOpenExternalLinks(true);

    ui->label_bilibili->setTextFormat(Qt::RichText);
    ui->label_bilibili->setText(u8"<a href='https://space.bilibili.com/144159485'>作者bilibili</a>");
    ui->label_bilibili->setOpenExternalLinks(true);
}

MainWindow::~MainWindow()
{
    if (m_translationThread && m_translationThread->isRunning()) {
        if (m_translationWorker) {
            m_translationWorker->stopTranslation();
        }
        m_translationThread->quit();
        m_translationThread->wait(3000);
    }
    
    delete m_settings;
    delete ui;
}

void MainWindow::initializeUI()
{
    // 初始化支持的语言
    m_supportedLanguages["auto"] = u8"自动检测";
    m_supportedLanguages["en"] = u8"英语";
    m_supportedLanguages["zh"] = u8"中文";
    m_supportedLanguages["cht"] = u8"繁体中文";
    m_supportedLanguages["yue"] = u8"粤语";
    m_supportedLanguages["wyw"] = u8"文言文";
    m_supportedLanguages["jp"] = u8"日语";
    m_supportedLanguages["kor"] = u8"韩语";
    m_supportedLanguages["spa"] = u8"西班牙语";
    m_supportedLanguages["fra"] = u8"法语";
    m_supportedLanguages["th"] = u8"泰语";
    m_supportedLanguages["ara"] = u8"阿拉伯语";
    m_supportedLanguages["ru"] = u8"俄语";
    m_supportedLanguages["pt"] = u8"葡萄牙语";
    m_supportedLanguages["de"] = u8"德语";
    m_supportedLanguages["it"] = u8"意大利语";
    m_supportedLanguages["el"] = u8"希腊语";
    m_supportedLanguages["nl"] = u8"荷兰语";
    m_supportedLanguages["pl"] = u8"波兰语";
    m_supportedLanguages["bul"] = u8"保加利亚语";
    m_supportedLanguages["est"] = u8"爱沙尼亚语";
    m_supportedLanguages["dan"] = u8"丹麦语";
    m_supportedLanguages["fin"] = u8"芬兰语";
    m_supportedLanguages["cs"] = u8"捷克语";
    m_supportedLanguages["rom"] = u8"罗马尼亚语";
    m_supportedLanguages["slo"] = u8"斯洛文尼亚语";
    m_supportedLanguages["swe"] = u8"瑞典语";
    m_supportedLanguages["hu"] = u8"匈牙利语";
    m_supportedLanguages["vie"] = u8"越南语";
    
    // 设置窗口标题
    setWindowTitle(u8"星火Godot 翻译工具");
    
    // 初始化进度条
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);
    
    // 设置按钮初始状态
    ui->btn_stop->setEnabled(false);

    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::loadSettings()
{
    m_settings = new QSettings("config.ini", QSettings::IniFormat, this);
    
    // 加载API配置
    ui->edit_id->setText(m_settings->value("api/appId", "").toString());
    ui->edit_key->setText(m_settings->value("api/secretKey", "").toString());

    // 加载隐藏API ID和Key的设置
    bool idHide = m_settings->value("settings/idHide", false).toBool();
    bool keyHide = m_settings->value("settings/keyHide", false).toBool();
    ui->checkBox_idHide->setChecked(idHide);
    ui->checkBox_keyHide->setChecked(keyHide);
    ui->edit_id->setEchoMode(idHide ? QLineEdit::Password : QLineEdit::Normal);
    ui->edit_key->setEchoMode(keyHide ? QLineEdit::Password : QLineEdit::Normal);
    
    // 加载文件路径
    QString lastFilePath = m_settings->value("file/lastPath", "").toString();
    if (!lastFilePath.isEmpty() && QFile::exists(lastFilePath)) {
        ui->edit_filePath->setText(lastFilePath);
        loadCSVFile(lastFilePath);
    }
    
    // 加载延迟时间设置
    int delayTime = m_settings->value("settings/delayTime", 100).toInt();
    ui->edit_delay->setText(QString::number(delayTime));
}

void MainWindow::saveSettings()
{
    if (!m_settings) return;
    
    // 保存API配置
    m_settings->setValue("api/appId", ui->edit_id->text());
    m_settings->setValue("api/secretKey", ui->edit_key->text());

    // 保存隐藏API ID和Key的设置
    m_settings->setValue("settings/idHide", ui->checkBox_idHide->isChecked());
    m_settings->setValue("settings/keyHide", ui->checkBox_keyHide->isChecked());
    
    // 保存文件路径
    m_settings->setValue("file/lastPath", ui->edit_filePath->text());
    
    // 保存延迟时间设置
    if(ui->edit_delay->text().toInt() > 0)
    {
        m_settings->setValue("settings/delayTime", ui->edit_delay->text().toInt());
    }
    
    m_settings->sync();
}

void MainWindow::setupLanguageCheckboxes()
{
    // 清除现有的复选框
    qDeleteAll(m_languageCheckboxes);
    m_languageCheckboxes.clear();
    
    // 创建28种目标语言的复选框（排除auto）
    QList<QPair<QString, QString>> sortedLanguages;
    QMapIterator<QString, QString> it(m_supportedLanguages);
    while (it.hasNext()) {
        it.next();
        if (it.key() == "auto") continue; // 跳过自动检测
        sortedLanguages.append(qMakePair(it.key(), it.value()));
    }
    
    // 按语言代码（key）排序
    std::sort(sortedLanguages.begin(), sortedLanguages.end(), [](const QPair<QString, QString> &a, const QPair<QString, QString> &b) {
        return a.first < b.first;
    });
    
    int row = 0;
    int col = 0;
    const int maxCols = 5; // 每行5个
    
    for (const QPair<QString, QString> &lang : sortedLanguages) {
        QCheckBox *checkbox = new QCheckBox(QString("%1 (%2)").arg(lang.second, lang.first));
        checkbox->setObjectName(lang.first);
        checkbox->setChecked(true); // 默认选中
        
        m_languageCheckboxes.append(checkbox);
        ui->LayoutLang->addWidget(checkbox, row, col);
        
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void MainWindow::loadCSVFile(const QString &filePath)
{
    if (!filePath.endsWith(".csv", Qt::CaseInsensitive)) {
        return;
    }
    
    if (!QFile::exists(filePath)) {
        addLogMessage("文件不存在: " + filePath);
        return;
    }
    
    try {
        m_csvData = parseCSV(filePath);
        if (!m_csvData.isEmpty()) {
            m_csvHeaders = m_csvData.first();
            updateSourceLanguageCombo();
            updatePreviewTable();
            addLogMessage(QString(u8"成功加载CSV文件: %1 行数据").arg(m_csvData.size() - 1));
        }
    } catch (const std::exception &e) {
        addLogMessage(u8"加载CSV文件失败: " + QString::fromStdString(e.what()));
    }
}

void MainWindow::updateSourceLanguageCombo()
{
    ui->comboBox_originLang->clear();
    if (!m_csvHeaders.isEmpty()) {
        ui->comboBox_originLang->addItems(m_csvHeaders);
    }
}

void MainWindow::addLogMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->textEditLog->append(QString("[%1] %2").arg(timestamp, message));
    
    // 自动滚动到底部
    QTextCursor cursor = ui->textEditLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textEditLog->setTextCursor(cursor);
}

void MainWindow::updateProgress(int value, const QString &text)
{
    ui->progressBar->setValue(value);
    if (!text.isEmpty()) {
        addLogMessage(text);
    }
}

void MainWindow::resetTranslationButtons()
{
    ui->btn_start->setEnabled(true);
    ui->btn_stop->setEnabled(false);
    m_isTranslating = false;
}

QStringList MainWindow::getSelectedTargetLanguages()
{
    QStringList selectedLangs;
    for (QCheckBox *checkbox : m_languageCheckboxes) {
        if (checkbox->isChecked()) {
            selectedLangs.append(checkbox->objectName());
        }
    }
    return selectedLangs;
}

void MainWindow::selectAllLanguages(bool select)
{
    for (QCheckBox *checkbox : m_languageCheckboxes) {
        checkbox->setChecked(select);
    }
}

QList<QStringList> MainWindow::parseCSV(const QString &filePath)
{
    QList<QStringList> data;
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error(u8"无法打开文件");
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields;
        
        // 简单的CSV解析（支持引号包围的字段）
        bool inQuotes = false;
        QString currentField;
        
        for (int i = 0; i < line.length(); ++i) {
            QChar c = line[i];
            
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                fields.append(currentField);
                currentField.clear();
            } else {
                currentField.append(c);
            }
        }
        
        fields.append(currentField);
        data.append(fields);
    }
    
    return data;
}

void MainWindow::saveCSV(const QString &filePath, const QList<QStringList> &data)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error(u8"无法创建文件");
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    for (const QStringList &row : data) {
        QStringList escapedRow;
        for (const QString &field : row) {
            QString escapedField = field;
            if (field.contains(',') || field.contains('"') || field.contains('\n')) {
                escapedField = '"' + field;
                escapedField.replace('"', "\"\"");
                escapedField += '"';
            }
            escapedRow.append(escapedField);
        }
        out << escapedRow.join(',') << '\n';
    }
}

// 拖拽事件处理
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            if (filePath.endsWith(".csv", Qt::CaseInsensitive)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString filePath = urls.first().toLocalFile();
        if (filePath.endsWith(".csv", Qt::CaseInsensitive)) {
            ui->edit_filePath->setText(filePath);
            loadCSVFile(filePath);
            event->acceptProposedAction();
        }
    }
}

// 槽函数实现
void MainWindow::on_btn_saveSetting_clicked()
{
    QString appId = ui->edit_id->text().trimmed();
    QString secretKey = ui->edit_key->text().trimmed();
    
    if (appId.isEmpty() || secretKey.isEmpty()) {
        QMessageBox::warning(this, u8"警告", u8"请填写完整的API配置信息");
        return;
    }
    
    saveSettings();
    QMessageBox::information(this, u8"成功", u8"配置已保存到config.ini文件");
    addLogMessage(u8"API配置已保存");
}

void MainWindow::on_btn_select_clicked()
{
    QString lastDir = m_settings ? m_settings->value("file/lastDir", QDir::homePath()).toString() : QDir::homePath();
    QString filePath = QFileDialog::getOpenFileName(this, u8"选择CSV文件", lastDir, u8"CSV文件 (*.csv)");
    if (!filePath.isEmpty()) {
        ui->edit_filePath->setText(filePath);
        loadCSVFile(filePath);
        
        // 保存文件所在目录
        if (m_settings) {
            m_settings->setValue("file/lastDir", QFileInfo(filePath).absolutePath());
            m_settings->sync();
        }
    }
}

void MainWindow::on_edit_filePath_textChanged(const QString &arg1)
{
    if (arg1.endsWith(".csv", Qt::CaseInsensitive)) {
        loadCSVFile(arg1);
    }
}

void MainWindow::on_btn_allSelect_clicked()
{
    selectAllLanguages(true);
    addLogMessage(u8"已选择所有目标语言");
}

void MainWindow::on_btn_cancelSelect_clicked()
{
    selectAllLanguages(false);
    addLogMessage(u8"已取消选择所有目标语言");
}

void MainWindow::on_btn_clearLog_clicked()
{
    ui->textEditLog->clear();
}

void MainWindow::on_btn_start_clicked()
{
    // 检查API配置
    QString appId = ui->edit_id->text().trimmed();
    QString secretKey = ui->edit_key->text().trimmed();
    
    if (appId.isEmpty() || secretKey.isEmpty()) {
        QMessageBox::warning(this, u8"警告", u8"请先配置百度翻译API");
        return;
    }
    
    // 检查CSV文件
    if (m_csvData.isEmpty()) {
        QMessageBox::warning(this, u8"警告", u8"请先加载CSV文件");
        return;
    }
    
    // 检查源语言列
    QString sourceColumn = ui->comboBox_originLang->currentText();
    if (sourceColumn.isEmpty()) {
        QMessageBox::warning(this, u8"警告", u8"请选择源语言列");
        return;
    }
    
    // 检查目标语言
    QStringList targetLangs = getSelectedTargetLanguages();
    if (targetLangs.isEmpty()) {
        QMessageBox::warning(this, u8"警告", u8"请至少选择一种目标语言");
        return;
    }
    
    // 获取源文本
    int sourceColumnIndex = m_csvHeaders.indexOf(sourceColumn);
    if (sourceColumnIndex == -1) {
        QMessageBox::warning(this, u8"警告", u8"源语言列不存在");
        return;
    }
    
    QStringList sourceTexts;
    for (int i = 1; i < m_csvData.size(); ++i) { // 跳过标题行
        if (sourceColumnIndex < m_csvData[i].size()) {
            sourceTexts.append(m_csvData[i][sourceColumnIndex]);
        } else {
            sourceTexts.append("");
        }
    }
    
    // 设置UI状态
    m_isTranslating = true;
    ui->btn_start->setEnabled(false);
    ui->btn_stop->setEnabled(true);
    ui->progressBar->setValue(0);
    
    // 创建翻译线程
    m_translationThread = new QThread(this);
    m_translationWorker = new TranslationWorker();
    m_translationWorker->moveToThread(m_translationThread);
    
    // 收集已翻译的数据
    QHash<QString, QHash<int, QString>> existingTranslations;
    for (const QString &targetLang : targetLangs) {
        int targetColumnIndex = m_csvHeaders.indexOf(targetLang);
        if (targetColumnIndex != -1) {
            QHash<int, QString> langTranslations;
            for (int i = 1; i < m_csvData.size(); ++i) { // 跳过标题行
                if (targetColumnIndex < m_csvData[i].size()) {
                    langTranslations[i-1] = m_csvData[i][targetColumnIndex]; // i-1因为sourceTexts跳过了标题行
                }
            }
            existingTranslations[targetLang] = langTranslations;
        }
    }
    
    // 设置翻译配置
    m_translationWorker->setConfig(appId, secretKey);
    m_translationWorker->setTranslationData(sourceTexts, "auto", targetLangs, ui->checkbox_tsed->isChecked());
    m_translationWorker->setExistingTranslations(existingTranslations);
    
    // 设置延迟时间
    int delayTime = m_settings->value("settings/delayTime", 50).toInt();
    m_translationWorker->setDelayTime(delayTime);
    
    // 连接信号
    connect(m_translationThread, &QThread::started, m_translationWorker, &TranslationWorker::startTranslation);
    connect(m_translationWorker, &TranslationWorker::progressUpdated, this, &MainWindow::onTranslationProgress);
    connect(m_translationWorker, &TranslationWorker::translationFinished, this, &MainWindow::onTranslationFinished);
    connect(m_translationWorker, &TranslationWorker::translationError, this, &MainWindow::onTranslationError);
    connect(m_translationWorker, &TranslationWorker::logMessage, this, &MainWindow::onLogMessage);
    
    // 启动翻译
    m_translationThread->start();
    addLogMessage(u8"开始翻译...");
}

void MainWindow::on_btn_stop_clicked()
{
    if (m_translationWorker) {
        m_translationWorker->stopTranslation();
        addLogMessage(u8"正在停止翻译...");
    }
    ui->btn_start->setEnabled(true);
    ui->btn_stop->setEnabled(false);
}

// 翻译进度回调
void MainWindow::onTranslationProgress(int current, int total, const QString &currentText, const QString &translatedText, const QString &targetLang)
{
    int progress = (current * 100) / total;
    ui->progressBar->setValue(progress);
    
    addLogMessage(QString("[%1] %2 -> %3").arg(targetLang, currentText.left(50), translatedText.left(50)));
    
    // 更新CSV数据
    QString targetColumnName = targetLang;
    int targetColumnIndex = m_csvHeaders.indexOf(targetColumnName);
    
    // 如果目标语言列不存在，添加它
    if (targetColumnIndex == -1) {
        m_csvHeaders.append(targetColumnName);
        targetColumnIndex = m_csvHeaders.size() - 1;
        
        // 为所有行添加新列
        for (int i = 0; i < m_csvData.size(); ++i) {
            while (m_csvData[i].size() <= targetColumnIndex) {
                if (i == 0) {
                    // 第一行是标题行，添加语言代码
                    m_csvData[i].append(targetColumnName);
                } else {
                    // 其他行添加空字符串
                    m_csvData[i].append("");
                }
            }
        }
    }
    
    // 更新翻译结果（计算在数据中的实际行索引）
    int dataRowIndex = ((current - 1) % (m_csvData.size() - 1)) + 1;
    if (dataRowIndex < m_csvData.size() && targetColumnIndex < m_csvData[dataRowIndex].size()) {
        m_csvData[dataRowIndex][targetColumnIndex] = translatedText;
        // 更新预览表格显示
        updatePreviewTable();
    }
}

void MainWindow::onTranslationFinished()
{
    resetTranslationButtons();
    ui->progressBar->setValue(100);
    addLogMessage(u8"翻译完成！");
    
    // 更新预览表格显示最终结果
    updatePreviewTable();
    
    // 保存翻译结果
    QString originalFilePath = ui->edit_filePath->text();
    QString outputFilePath = originalFilePath;
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    outputFilePath.replace(u8".csv", QString(u8"_%1.csv").arg(timestamp));
    
    try {
        saveCSV(outputFilePath, m_csvData);
        addLogMessage(u8"翻译结果已保存到: " + outputFilePath);
        QMessageBox::information(this, u8"完成", u8"翻译完成！\n结果已保存到: " + outputFilePath);
    } catch (const std::exception &e) {
        addLogMessage(u8"保存文件失败: " + QString::fromStdString(e.what()));
        QMessageBox::warning(this, u8"错误", u8"保存文件失败: " + QString::fromStdString(e.what()));
    }
    
    // 清理线程
    if (m_translationThread) {
        m_translationThread->quit();
        m_translationThread->wait();
        m_translationThread->deleteLater();
        m_translationThread = nullptr;
    }
    
    if (m_translationWorker) {
        m_translationWorker->deleteLater();
        m_translationWorker = nullptr;
    }
}

void MainWindow::onTranslationError(const QString &error)
{
    resetTranslationButtons();
    addLogMessage(u8"翻译错误: " + error);
    QMessageBox::critical(this, u8"翻译错误", error);
}

void MainWindow::onLogMessage(const QString &message)
{
    addLogMessage(message);
}

void MainWindow::updatePreviewTable()
{
    if (m_csvData.isEmpty()) {
        ui->table_previewData->clear();
        ui->table_previewData->setRowCount(0);
        ui->table_previewData->setColumnCount(0);
        return;
    }
    
    // 设置表格行数和列数
    ui->table_previewData->setRowCount(m_csvData.size());
    ui->table_previewData->setColumnCount(m_csvHeaders.size());
    
    // 启用行间颜色交替
    ui->table_previewData->setAlternatingRowColors(true);
    
    // 设置表头 - 将语言代码转换为中文显示
    QStringList chineseHeaders;
    for (const QString &header : m_csvHeaders) {
        // 检查是否为支持的语言代码，如果是则显示中文名称
        if (m_supportedLanguages.contains(header)) {
            chineseHeaders.append(m_supportedLanguages[header]);
        } else {
            chineseHeaders.append(header);
        }
    }
    ui->table_previewData->setHorizontalHeaderLabels(chineseHeaders);
    
    // 填充数据
    for (int row = 0; row < m_csvData.size(); ++row) {
        const QStringList &rowData = m_csvData[row];
        for (int col = 0; col < m_csvHeaders.size(); ++col) {
            QString cellText = (col < rowData.size()) ? rowData[col] : "";
            QTableWidgetItem *item = new QTableWidgetItem(cellText);
            
            // 第一行（标题行）设置为只读并加粗
            if (row == 0) {
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setBackground(QColor(240, 240, 240));
            }
            
            ui->table_previewData->setItem(row, col, item);
        }
    }
    
    // 自动调整列宽
    ui->table_previewData->resizeColumnsToContents();
    
    // 限制最大列宽，避免过宽
    for (int col = 0; col < ui->table_previewData->columnCount(); ++col) {
        if (ui->table_previewData->columnWidth(col) > 300) {
            ui->table_previewData->setColumnWidth(col, 300);
        }
    }
}



void MainWindow::on_checkBox_idHide_stateChanged(int state)
{
    ui->edit_id->setEchoMode(state == Qt::Checked ? QLineEdit::Password : QLineEdit::Normal);
    saveSettings();
}

void MainWindow::on_checkBox_keyHide_stateChanged(int state)
{
    ui->edit_key->setEchoMode(state == Qt::Checked ? QLineEdit::Password : QLineEdit::Normal);
    saveSettings();
}

void MainWindow::on_btn_saveCsv_clicked()
{
    if (m_csvData.isEmpty()) {
        QMessageBox::warning(this, u8"警告", u8"没有可保存的CSV数据");
        return;
    }

    QString defaultFileName = ui->edit_filePath->text();
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    if (defaultFileName.isEmpty()) {
        defaultFileName = QString(u8"translated_%1.csv").arg(timestamp);
    } else {
        defaultFileName.replace(u8".csv", QString(u8"_output_%1.csv").arg(timestamp));
    }

    QString saveFilePath = QFileDialog::getSaveFileName(this, u8"保存CSV文件", defaultFileName, u8"CSV文件 (*.csv)");
    if (saveFilePath.isEmpty()) {
        return;
    }

    try {
        saveCSV(saveFilePath, m_csvData);
        addLogMessage(u8"CSV数据已保存到: " + saveFilePath);
        QMessageBox::information(this, u8"保存成功", u8"CSV数据已成功保存！");
    } catch (const std::exception &e) {
        addLogMessage(u8"保存CSV文件失败: " + QString::fromStdString(e.what()));
        QMessageBox::warning(this, u8"保存失败", u8"保存CSV文件失败: " + QString::fromStdString(e.what()));
    }
}

void MainWindow::on_btn_setDelay_clicked()
{
    QString delayText = ui->edit_delay->text().trimmed();
    
    // 检查输入是否为空
    if (delayText.isEmpty()) {
        QMessageBox::warning(this, u8"警告", u8"请输入延迟时间");
        return;
    }
    
    // 检查输入是否为正整数
    bool ok;
    int delayTime = delayText.toInt(&ok);
    
    if (!ok || delayTime <= 0) {
        QMessageBox::warning(this, u8"警告", u8"延迟时间必须是正整数（毫秒）");
        return;
    }
    
    // 保存设置到配置文件
    saveSettings();
    
    // 显示成功消息
    QMessageBox::information(this, u8"成功", QString(u8"延迟时间已设置为 %1 毫秒").arg(delayTime));
    addLogMessage(QString(u8"翻译延迟时间已设置为: %1 毫秒").arg(delayTime));
}

void MainWindow::on_btn_previewWrite_clicked()
{
    if (ui->table_previewData->rowCount() == 0 || ui->table_previewData->columnCount() == 0) {
        QMessageBox::warning(this, u8"警告", u8"预览表格为空，无法写入数据");
        return;
    }
    
    // 确认对话框
    int ret = QMessageBox::question(this, u8"确认写入", 
                                   u8"确定要将预览表格中的数据保存到新的CSV文件吗？",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    // 选择保存文件路径
    QString defaultFileName = ui->edit_filePath->text();
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    if (defaultFileName.isEmpty()) {
        defaultFileName = QString(u8"preview_data_%1.csv").arg(timestamp);
    } else {
        QFileInfo fileInfo(defaultFileName);
        QString baseName = fileInfo.completeBaseName();
        QString path = fileInfo.path();
        defaultFileName = QString("%1/%2_preview_%3.csv").arg(path, baseName, timestamp);
    }
    
    QString saveFilePath = QFileDialog::getSaveFileName(this, u8"保存预览数据到CSV文件", defaultFileName, u8"CSV文件 (*.csv)");
    if (saveFilePath.isEmpty()) {
        return;
    }
    
    try {
        // 构建要保存的CSV数据
        QList<QStringList> saveData;
        
        // 使用原始的CSV表头（语言代码），而不是显示的中文名称
        QStringList csvHeaders;
        for (int col = 0; col < ui->table_previewData->columnCount(); ++col) {
            if (col < m_csvHeaders.size()) {
                csvHeaders.append(m_csvHeaders[col]);
            } else {
                csvHeaders.append(QString("Column_%1").arg(col + 1));
            }
        }
        
        // 添加表头作为第一行
        saveData.append(csvHeaders);
        
        // 构建数据行（跳过第一行，因为第一行是表头数据）
        for (int row = 1; row < ui->table_previewData->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < ui->table_previewData->columnCount(); ++col) {
                QTableWidgetItem *item = ui->table_previewData->item(row, col);
                QString cellText = item ? item->text() : "";
                rowData.append(cellText);
            }
            saveData.append(rowData);
        }
        
        // 更新内部数据结构
        m_csvData.clear();
        m_csvData = saveData;
        
        // 保存到CSV文件
        saveCSV(saveFilePath, saveData);
        
        // 更新源语言下拉框
        updateSourceLanguageCombo();
        
        addLogMessage(u8"预览表格数据已成功保存到CSV文件: " + saveFilePath);
        QMessageBox::information(this, u8"保存成功", u8"预览表格数据已成功保存到CSV文件！");
        
    } catch (const std::exception &e) {
        addLogMessage(u8"保存CSV文件失败: " + QString::fromStdString(e.what()));
        QMessageBox::warning(this, u8"保存失败", u8"保存CSV文件失败: " + QString::fromStdString(e.what()));
    }
}
