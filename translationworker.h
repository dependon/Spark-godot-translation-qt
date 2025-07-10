#ifndef TRANSLATIONWORKER_H
#define TRANSLATIONWORKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QEventLoop>
#include <QTimer>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QDebug>

class TranslationWorker : public QObject
{
    Q_OBJECT

public:
    explicit TranslationWorker(QObject *parent = nullptr);
    void setConfig(const QString &appId, const QString &secretKey);
    void setTranslationData(const QStringList &sourceTexts, const QString &fromLang, const QStringList &targetLangs, bool forceRetranslate);
    void setExistingTranslations(const QHash<QString, QHash<int, QString>> &existingTranslations);
    void setDelayTime(int delayMs);
    void stopTranslation();

public slots:
    void startTranslation();

signals:
    void progressUpdated(int current, int total, const QString &currentText, const QString &translatedText, const QString &targetLang);
    void translationFinished();
    void translationError(const QString &error);
    void logMessage(const QString &message);

private:
    QString translateText(const QString &text, const QString &from, const QString &to);
    QString generateSign(const QString &query, const QString &salt);
    QString getCacheKey(const QString &text, const QString &from, const QString &to);

    QString m_appId;
    QString m_secretKey;
    QStringList m_sourceTexts;
    QString m_fromLang;
    QStringList m_targetLangs;
    bool m_forceRetranslate;
    bool m_shouldStop;
    int m_delayTime = 50;
    QMutex m_mutex;
    QNetworkAccessManager *m_networkManager;
    QHash<QString, QString> m_translationCache;
    QHash<QString, QHash<int, QString>> m_existingTranslations;
};

#endif // TRANSLATIONWORKER_H
