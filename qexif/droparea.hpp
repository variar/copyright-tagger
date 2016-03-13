#ifndef DROPAREA_HPP
#define DROPAREA_HPP

#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDir>
#include <QFutureWatcher>

class DropArea : public QLabel
{
    Q_OBJECT
public:
    explicit DropArea(QWidget *parent = 0);


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

private:
    QString m_artist;
    QString m_copyright;

    QFutureWatcher<void> m_taggingWatcher;
};

#endif // DROPAREA_HPP
