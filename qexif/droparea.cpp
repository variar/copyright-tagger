#include "droparea.hpp"

#include <QMimeData>
#include <QDebug>
#include <QFile>
#include <QQueue>
#include <QtConcurrent>

#include <exiv2/exiv2.hpp>

namespace
{

bool isJpeg(const QString& path)
{
    return path.endsWith(".jpg", Qt::CaseInsensitive)
            || path.endsWith(".jpeg", Qt::CaseInsensitive);
}

QStringList GetJpegFilesInDir(const QString& dir)
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

void SetExifDataForFile(const QString& filename,
                        const QString& artist,
                        const QString& copyright)
{
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename.toLocal8Bit().data());
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();
    if (!copyright.isEmpty())
    {
        exifData["Exif.Image.Copyright"] = copyright.toLocal8Bit().data();
    }
    if (!artist.isEmpty())
    {
        exifData["Exif.Image.Artist"] = artist.toLocal8Bit().data();
    }

    image->setExifData(exifData);
    image->writeMetadata();
}

}

DropArea::DropArea(QWidget *parent) : QLabel(parent)
{
    setAcceptDrops(true);
    connect(&m_taggingWatcher, SIGNAL(progressValueChanged(int)),
            this, SIGNAL(progressChanged(int)));
    connect(&m_taggingWatcher, SIGNAL(finished()),
            this, SIGNAL(taggingFinished()));
}

void DropArea::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void DropArea::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    QList<QString> jpegFiles;

    foreach (QUrl url, urls)
    {
        const QFileInfo pathInfo(url.path());

        if (!pathInfo.exists())
            continue;

        if (pathInfo.isFile() && isJpeg(pathInfo.fileName()))
        {
            jpegFiles.append(pathInfo.absoluteFilePath());
        }
        else if (pathInfo.isDir())
        {
            jpegFiles << GetJpegFilesInDir(pathInfo.absoluteFilePath());
        }
    }

    emit taggingStarted(jpegFiles.size());

    int progress = 0;
    foreach(QString file, jpegFiles)
    {
        SetExifDataForFile(file, m_artist, m_copyright);
        progress++;
        emit progressChanged(progress);
    }

    emit taggingFinished();
    //m_taggingWatcher.setFuture(QtConcurrent::map(jpegFiles, SetExifDataForFile));
}


