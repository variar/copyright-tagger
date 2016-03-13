#ifndef DROPAREA_HPP
#define DROPAREA_HPP

#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDir>
#include <QFutureWatcher>
#include <QThread>

class Tagger : public QThread
{
    Q_OBJECT
public:
    explicit Tagger(QObject* parent = 0) : QThread(parent)
    {}

    void setFiles(const QStringList& files)
    {
        m_files = files;
    }

    void setArtist(const QString& artist)
    {
        m_artist = artist;
    }

    void setCopyright(const QString& copyright)
    {
        m_copyright = copyright;
    }

protected:
    void run() override;

signals:
    void progressChanged(int);

private:
    void setExifDataForFile(const QString& filename,
                            const QString& artist,
                            const QString& copyright);

private:
    QStringList m_files;
    QString m_artist;
    QString m_copyright;

};

class Crawler : public QThread
{
    Q_OBJECT
public:
    explicit Crawler(QObject* parent = 0) : QThread(parent)
    {}

    void setUrls(const QList<QUrl> urls)
    {
        m_urls = urls;
    }

signals:
    void finished(const QStringList& files);

protected:
    void run() override;

private:
    QStringList getJpegFilesInDir(const QString& dir);

    static bool isJpeg(const QString& path)
    {
        return path.endsWith(".jpg", Qt::CaseInsensitive)
                || path.endsWith(".jpeg", Qt::CaseInsensitive);
    }

private:
    QList<QUrl> m_urls;
};

class DropArea : public QLabel
{
    Q_OBJECT
public:
    explicit DropArea(QWidget *parent = 0);

    void tagInFolder(const QString& path);

signals:
    void progressChanged(int progress);
    void taggingStarted(int totalFiles);
    void taggingFinished();

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

private:
    void startCrawler(const QList<QUrl>& urls);

private:
    QString m_artist;
    QString m_copyright;

    Crawler* m_crawler;
    Tagger* m_tagger;

    QFutureWatcher<void> m_taggingWatcher;
};

#endif // DROPAREA_HPP
