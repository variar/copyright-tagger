#include "droparea.hpp"

#include <QMimeData>
#include <QDebug>
#include <QFile>
#include <QQueue>
#include <QMessageBox>

#include <exiv2/exiv2.hpp>
#include <exiv2/error.hpp>

void Tagger::tagFiles(const QStringList& files,
                      const QString& artist,
                      const QString& copyright)
{
    int progress = 0;
    foreach(QString file, files)
    {
        try
        {
            setExifDataForFile(file, artist, copyright);
        }
        catch(const std::exception& err)
        {
            qCritical() << "Failed to set data for " << file
                        << " error: " << err.what();

            emit error(file);
        }

        progress++;
        emit progressChanged(progress);
    }
    emit finished();
}

void Tagger::setExifDataForFile(const QString& filename,
                            const QString& artist,
                            const QString& copyright) const
{
    qDebug() << "File path: " << filename;

#ifdef EXV_UNICODE_PATH
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename.toStdWString());
#else
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename.toLocal8Bit().data());
#endif

    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    Exiv2::IptcData& iptcData = image->iptcData();
    Exiv2::XmpData& xmpData = image->xmpData();
    if (!copyright.isEmpty())
    {
        exifData["Exif.Image.Copyright"] = copyright.toLocal8Bit().data();
        iptcData["Iptc.Application2.Copyright"] = copyright.toLocal8Bit().data();
        xmpData["Xmp.dc.rights"] = copyright.toLocal8Bit().data();
    }
    if (!artist.isEmpty())
    {
        exifData["Exif.Image.Artist"] = artist.toLocal8Bit().data();
        iptcData["Iptc.Application2.Byline"] = artist.toLocal8Bit().data();
        xmpData["Xmp.dc.creator"] = artist.toLocal8Bit().data();
    }

    image->setExifData(exifData);
    image->setIptcData(iptcData);
    image->setXmpData(xmpData);
    image->writeMetadata();
}

void Crawler::collectFiles(const QList<QUrl> urls)
{
    QStringList jpegFiles;

    foreach (QUrl url, urls)
    {
        qInfo() << "Scanning " << url.toLocalFile();
        const QFileInfo pathInfo(url.toLocalFile());

        if (!pathInfo.exists())
        {
            qWarning() << "Path not exists: " << pathInfo.absoluteFilePath();
            continue;
        }

        if (pathInfo.isFile() && isJpeg(pathInfo.fileName()))
        {
            jpegFiles.append(pathInfo.absoluteFilePath());
        }
        else if (pathInfo.isDir())
        {
            jpegFiles << getJpegFilesInDir(pathInfo.absoluteFilePath());
        }
        else
        {
            qWarning() << "Path not jpeg: " << pathInfo.absoluteFilePath();
        }
    }

    qDebug() << "Found " << jpegFiles.size() << " jpeg files";

    emit finished(jpegFiles);
}


QStringList Crawler::getJpegFilesInDir(const QString& dir) const
{
    QQueue<QString> dirsToWalk;
    dirsToWalk.enqueue(dir);

    QStringList filters;
    filters << "*.jpeg" << "*.jpg";
    QStringList jpegFiles;

    while (!dirsToWalk.empty())
    {
        const QDir currentDir(dirsToWalk.dequeue());
        const QFileInfoList childDirs = currentDir.entryInfoList(QDir::Dirs);
        foreach (QFileInfo d, childDirs)
        {
            if (d.fileName() != "." && d.fileName() != "..")
                dirsToWalk.enqueue(d.absoluteFilePath());
        }
        const QFileInfoList files = currentDir.entryInfoList(filters, QDir::Files);
        foreach (QFileInfo f, files)
        {
           jpegFiles.append(f.absoluteFilePath());
        }
    }

    return jpegFiles;
}

DropArea::DropArea(QWidget *parent) : QLabel(parent)
{
    qRegisterMetaType<QList<QUrl>>();
    setAcceptDrops(true);

    m_crawler = new Crawler();
    m_tagger = new Tagger();

    m_crawler->moveToThread(&m_worker);
    m_tagger->moveToThread(&m_worker);

    connect(m_crawler, SIGNAL(finished(QStringList)),
            SLOT(onCrawlerFinished(QStringList)), Qt::QueuedConnection);

    connect(m_tagger, SIGNAL(finished()),
            SLOT(onTaggerFinished()), Qt::QueuedConnection);

    connect(m_tagger, SIGNAL(progressChanged(int)),
            SIGNAL(progressChanged(int)), Qt::QueuedConnection);

    connect(m_tagger, SIGNAL(error(QString)),
            SLOT(onError(QString)), Qt::QueuedConnection);

    connect(this, SIGNAL(runCrawler(QList<QUrl>)),
            m_crawler, SLOT(collectFiles(QList<QUrl>)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(runTagger(QStringList,QString,QString)),
            m_tagger, SLOT(tagFiles(QStringList,QString,QString)),
            Qt::QueuedConnection);

    connect(this, SIGNAL(destroyed(QObject*)), &m_worker, SLOT(quit()));

    m_worker.start();
}

DropArea::~DropArea()
{
    m_worker.quit();
    m_worker.wait();
}

void DropArea::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void DropArea::startTagFiles(const QList<QUrl> &urls)
{
    startCrawler(urls);
}

void DropArea::startCrawler(const QList<QUrl> &urls)
{
    m_errorList.clear();
    setAcceptDrops(false);
    emit runCrawler(urls);
}

void DropArea::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty())
    {
        startCrawler(urls);
    }
}

void DropArea::onCrawlerFinished(const QStringList& jpegFiles)
{
    m_filesCount = jpegFiles.size();
    if (m_filesCount == 0)
    {
        QMessageBox::warning(this, "Завершено", "Не найдено jpeg файлов");
        emit taggingFinished();
        setAcceptDrops(true);
        return;
    }

    emit taggingStarted(jpegFiles.size());
    emit runTagger(jpegFiles, m_artist, m_copyright);
}

void DropArea::onTaggerFinished()
{
    emit taggingFinished();
    setAcceptDrops(true);
    if (m_errorList.empty())
    {
        QMessageBox::information(this, "Успешно завершено",
                                 QString("Теги проставлены для %1 файлов").arg(m_filesCount));
    }
    else
    {
        QString errors = m_errorList.join("\r\n");
        QMessageBox::warning(this, "Ошибка",
                             QString("Для следующих файлов теги не проставлены: %1").arg(errors));
    }
}

void DropArea::onError(QString error)
{
    m_errorList << error;
}


