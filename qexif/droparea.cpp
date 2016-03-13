#include "droparea.hpp"

#include <QMimeData>
#include <QDebug>
#include <QFile>
#include <QQueue>
#include <QMessageBox>

#include <exiv2/exiv2.hpp>
#include <exiv2/error.hpp>

void Tagger::run()
{
    int progress = 0;
    foreach(QString file, m_files)
    {
        try
        {
            setExifDataForFile(file, m_artist, m_copyright);
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
}

void Tagger::setExifDataForFile(const QString& filename,
                            const QString& artist,
                            const QString& copyright)
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

void Crawler::run()
{
    QStringList jpegFiles;

    foreach (QUrl url, m_urls)
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


QStringList Crawler::getJpegFilesInDir(const QString& dir)
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
    setAcceptDrops(true);

    m_crawler = new Crawler(this);
    m_tagger = new Tagger(this);

    connect(m_crawler, SIGNAL(finished(QStringList)),
            SLOT(onCrawlerFinished(QStringList)));

    connect(m_tagger, SIGNAL(finished()),
            SLOT(onTaggerFinished()), Qt::BlockingQueuedConnection);

    connect(m_tagger, SIGNAL(progressChanged(int)),
            SIGNAL(progressChanged(int)), Qt::QueuedConnection);

    connect(m_tagger, SIGNAL(error(QString)),
            SLOT(onError(QString)), Qt::QueuedConnection);
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
    m_crawler->setUrls(urls);
    m_crawler->start();
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
    m_tagger->setFiles(jpegFiles);
    m_tagger->setArtist(m_artist);
    m_tagger->setCopyright(m_copyright);
    m_tagger->start();
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


