#ifndef DROPAREA_HPP
#define DROPAREA_HPP

#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDir>
#include <QThread>

Q_DECLARE_METATYPE(QList<QUrl>)

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    void run() override
    {
        exec();
    }
};

class Tagger : public QObject
{
    Q_OBJECT
public:
    explicit Tagger(QObject* parent = 0) : QObject(parent)
    {}

public slots:
    void tagFiles(const QStringList& files,
                  const QString& artist,
                  const QString& copyright);

signals:
    void progressChanged(int);
    void error(QString);
    void finished();

private:
    void setExifDataForFile(const QString& filename,
                            const QString& artist,
                            const QString& copyright) const;
};

class Crawler : public QObject
{
    Q_OBJECT
public:
    explicit Crawler(QObject* parent = 0) : QObject(parent)
    {}

public slots:
    void collectFiles(const QList<QUrl> urls);

signals:
    void finished(const QStringList& files);

private:
    QStringList getJpegFilesInDir(const QString& dir) const;

    static bool isJpeg(const QString& path)
    {
        return path.endsWith(".jpg", Qt::CaseInsensitive)
                || path.endsWith(".jpeg", Qt::CaseInsensitive);
    }

};

class DropArea : public QLabel
{
    Q_OBJECT
public:
    explicit DropArea(QWidget *parent = 0);
    ~DropArea();

    void startTagFiles(const QList<QUrl> &urls);

signals:
    void progressChanged(int progress);
    void taggingStarted(int totalFiles);
    void taggingFinished();

    void runCrawler(const QList<QUrl> &urls);
    void runTagger(const QStringList& files,
                   const QString& artist,
                   const QString& copyright);

public slots:

    void setArtist(const QString& artist)
    {
        m_artist = artist;
    }

    void setCopyright(const QString& copyright)
    {
        m_copyright = copyright;
    }


protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onCrawlerFinished(const QStringList& files);
    void onTaggerFinished();
    void onError(QString error);

private:
    void startCrawler(const QList<QUrl>& urls);

private:
    QString m_artist;
    QString m_copyright;

    quint32 m_filesCount;
    QStringList m_errorList;

    WorkerThread m_worker;

    Crawler* m_crawler;
    Tagger* m_tagger;
};

#endif // DROPAREA_HPP
