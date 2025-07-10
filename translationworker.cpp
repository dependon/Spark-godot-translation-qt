#include "translationworker.h"

TranslationWorker::TranslationWorker(QObject *parent) : QObject(parent),
    m_networkManager(new QNetworkAccessManager(this)),
    m_shouldStop(false)
{
    // 检查SSL支持状态
    if (!QSslSocket::supportsSsl()) {
        emit logMessage(u8"警告: OpenSSL不可用，HTTPS请求可能失败");
        emit logMessage(u8"请参考SSL_SETUP_GUIDE.md文档配置OpenSSL");
    }

    // 配置SSL以解决TLS初始化问题
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(sslConfig);
    
    emit logMessage(QString(u8"TranslationWorker初始化完成，SSL支持: %1")
                   .arg(QSslSocket::supportsSsl() ? u8"是" : u8"否"));
}

void TranslationWorker::setConfig(const QString &appId, const QString &secretKey)
{
    m_appId = appId;
    m_secretKey = secretKey;
}

void TranslationWorker::setTranslationData(const QStringList &sourceTexts, const QString &fromLang, const QStringList &targetLangs, bool forceRetranslate)
{
    m_sourceTexts = sourceTexts;
    m_fromLang = fromLang;
    m_targetLangs = targetLangs;
    m_forceRetranslate = forceRetranslate;
}

void TranslationWorker::setExistingTranslations(const QHash<QString, QHash<int, QString>> &existingTranslations)
{
    m_existingTranslations = existingTranslations;
}

void TranslationWorker::stopTranslation()
{
    QMutexLocker locker(&m_mutex);
    m_shouldStop = true;
}

void TranslationWorker::startTranslation()
{
    m_shouldStop = false;
    int totalTexts = m_sourceTexts.size();
    int totalTranslations = totalTexts * m_targetLangs.size();
    int currentIndex = 0;
    
    emit logMessage(QString(u8"开始翻译，共%1个文本，%2种目标语言，总计%3个翻译任务")
                   .arg(totalTexts).arg(m_targetLangs.size()).arg(totalTranslations));
    
    for (const QString &targetLang : m_targetLangs) {
        if (m_shouldStop) {
            emit logMessage(u8"翻译已停止");
            break;
        }
        
        for (int i = 0; i < m_sourceTexts.size(); ++i) {
            if (m_shouldStop) {
                emit logMessage(u8"翻译已停止");
                return;
            }
            
            const QString &sourceText = m_sourceTexts[i];
            if (sourceText.trimmed().isEmpty()) {
                currentIndex++;
                continue;
            }
            
            // 检查是否需要跳过已翻译的内容
            if (!m_forceRetranslate && m_existingTranslations.contains(targetLang)) {
                const QHash<int, QString> &langTranslations = m_existingTranslations[targetLang];
                if (langTranslations.contains(i) && !langTranslations[i].trimmed().isEmpty()) {
                    // 已经翻译过且不为空，跳过翻译
                    currentIndex++;
                    emit progressUpdated(currentIndex, totalTranslations, sourceText, langTranslations[i], targetLang);
                    continue;
                }
            }
            
            QString translatedText = translateText(sourceText, m_fromLang, targetLang);
            if (translatedText.isEmpty()) {
                emit translationError(QString(u8"翻译失败: %1").arg(sourceText.left(50)));
                return;
            }
            
            currentIndex++;
            emit progressUpdated(currentIndex, totalTranslations, sourceText, translatedText, targetLang);
            
            // 添加延迟以避免API限制
            QEventLoop loop;
            QTimer::singleShot(100, &loop, &QEventLoop::quit);
            loop.exec();
        }
    }
    
    if (!m_shouldStop) {
        emit translationFinished();
    }
}

QString TranslationWorker::translateText(const QString &text, const QString &from, const QString &to)
{
    if (text.trimmed().isEmpty()) {
        return text;
    }
    
    // 检查SSL支持
    if (!QSslSocket::supportsSsl()) {
        emit logMessage(u8"错误: OpenSSL不可用，无法进行HTTPS请求");
        emit logMessage(u8"请按照SSL_SETUP_GUIDE.md文档配置OpenSSL后重试");
        return QString(); // 返回空字符串表示失败
    }
    
    // 检查缓存
    QString cacheKey = getCacheKey(text, from, to);
    if (m_translationCache.contains(cacheKey)) {
        return m_translationCache[cacheKey];
    }
    
    // 构建请求参数
    QString salt = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString sign = generateSign(text, salt);
    
    QUrl url("https://fanyi-api.baidu.com/api/trans/vip/translate");
    QUrlQuery query;
    query.addQueryItem("q", text);
    query.addQueryItem("from", from);
    query.addQueryItem("to", to);
    query.addQueryItem("appid", m_appId);
    query.addQueryItem("salt", salt);
    query.addQueryItem("sign", sign);
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    
    // 设置SSL配置以避免TLS错误
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);
    
    QNetworkReply *reply = m_networkManager->post(request, query.toString(QUrl::FullyEncoded).toUtf8());
    
    // 设置超时
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.setInterval(30000); // 30秒超时
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            [this, reply](QNetworkReply::NetworkError error) {
                emit logMessage(QString(u8"网络请求错误代码: %1, 错误信息: %2")
                               .arg(error).arg(reply->errorString()));
            });
    
    timeoutTimer.start();
    loop.exec();
    timeoutTimer.stop();
    
    QString result;
    
    // 检查是否超时
    if (!timeoutTimer.isActive() && reply->isRunning()) {
        reply->abort();
        emit logMessage(u8"请求超时，已取消请求");
    } else if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        
        if (responseData.isEmpty()) {
            emit logMessage(u8"服务器返回空响应");
        } else {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
            
            if (parseError.error != QJsonParseError::NoError) {
                emit logMessage(QString(u8"JSON解析错误: %1").arg(parseError.errorString()));
            } else {
                QJsonObject obj = doc.object();
                
                if (obj.contains("trans_result")) {
                    QJsonArray transResult = obj["trans_result"].toArray();
                    if (!transResult.isEmpty()) {
                        QJsonObject firstResult = transResult[0].toObject();
                        result = firstResult["dst"].toString();
                        
                        // 缓存结果
                        m_translationCache[cacheKey] = result;
                        emit logMessage(QString(u8"翻译成功: %1 -> %2")
                                       .arg(text.left(20), result.left(20)));
                    } else {
                        emit logMessage(u8"翻译结果为空");
                    }
                } else if (obj.contains("error_code")) {
                    QString errorCode = obj["error_code"].toString();
                    QString errorMsg = obj["error_msg"].toString();
                    emit logMessage(QString(u8"百度API错误: %1 - %2").arg(errorCode, errorMsg));
                } else {
                    emit logMessage(u8"未知的API响应格式");
                }
            }
        }
    } else {
        QString errorDetail;
        switch (reply->error()) {
        case QNetworkReply::ConnectionRefusedError:
            errorDetail = u8"连接被拒绝";
            break;
        case QNetworkReply::RemoteHostClosedError:
            errorDetail = u8"远程主机关闭连接";
            break;
        case QNetworkReply::HostNotFoundError:
            errorDetail = u8"主机未找到";
            break;
        case QNetworkReply::TimeoutError:
            errorDetail = u8"请求超时";
            break;
        case QNetworkReply::SslHandshakeFailedError:
            errorDetail = u8"SSL握手失败";
            break;
        case QNetworkReply::TemporaryNetworkFailureError:
            errorDetail = u8"临时网络故障";
            break;
        case QNetworkReply::NetworkSessionFailedError:
            errorDetail = u8"网络会话失败";
            break;
        case QNetworkReply::BackgroundRequestNotAllowedError:
            errorDetail = u8"后台请求不被允许";
            break;
        case QNetworkReply::ProxyConnectionRefusedError:
            errorDetail = u8"代理连接被拒绝";
            break;
        case QNetworkReply::ProxyConnectionClosedError:
            errorDetail = u8"代理连接关闭";
            break;
        case QNetworkReply::ProxyNotFoundError:
            errorDetail = u8"代理未找到";
            break;
        case QNetworkReply::ProxyTimeoutError:
            errorDetail = u8"代理超时";
            break;
        case QNetworkReply::ProxyAuthenticationRequiredError:
            errorDetail = u8"代理需要认证";
            break;
        default:
            errorDetail = QString(u8"未知错误(代码:%1)").arg(reply->error());
            break;
        }
        emit logMessage(QString(u8"网络错误: %1 - %2").arg(errorDetail, reply->errorString()));
    }
    
    reply->deleteLater();
    return result;
}

QString TranslationWorker::generateSign(const QString &query, const QString &salt)
{
    QString str = m_appId + query + salt + m_secretKey;
    QByteArray hash = QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

QString TranslationWorker::getCacheKey(const QString &text, const QString &from, const QString &to)
{
    return QString("%1-%2-%3").arg(from, to, text.trimmed());
}
