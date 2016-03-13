#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings(new QSettings("tags.ini", QSettings::IniFormat, this))
{
    ui->setupUi(this);

    connect(ui->dropLabel, SIGNAL(taggingStarted(int)),
            this, SLOT(onTaggingStared()));
    connect(ui->dropLabel, SIGNAL(taggingFinished()),
            this, SLOT(onTaggingFinished()));

    connect(ui->dropLabel, SIGNAL(progressChanged(int)),
            ui->progressBar, SLOT(setValue(int)));
    connect(ui->dropLabel, SIGNAL(taggingStarted(int)),
            ui->progressBar, SLOT(setMaximum(int)));
    connect(ui->dropLabel, SIGNAL(taggingFinished()),
            ui->progressBar, SLOT(reset()));

    connect(ui->lineEdit, SIGNAL(textEdited(QString)),
            ui->dropLabel, SLOT(setArtist(QString)));
    connect(ui->lineEdit_2, SIGNAL(textEdited(QString)),
            ui->dropLabel, SLOT(setCopyright(QString)));

    connect(ui->lineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(updateSettings()));
    connect(ui->lineEdit_2, SIGNAL(textEdited(QString)),
            this, SLOT(updateSettings()));

    ui->lineEdit->setText(m_settings->value("image.artist", "").toString());
    ui->lineEdit_2->setText(m_settings->value("image.copyright", "").toString());

    ui->dropLabel->setArtist(ui->lineEdit->text());
    ui->dropLabel->setCopyright(ui->lineEdit_2->text());

    connect(ui->actionAboutQt, SIGNAL(triggered(bool)), SLOT(showAboutQtDialog()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), SLOT(showAboutDialog()));

    connect(ui->selectFileButton, SIGNAL(clicked(bool)), SLOT(onSelectFiles()));
    connect(ui->selectFolderButton, SIGNAL(clicked(bool)), SLOT(onSelectFolders()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTaggingStared()
{
    ui->lineEdit->setEnabled(false);
    ui->lineEdit_2->setEnabled(false);
    ui->progressBar->setEnabled(true);
}

void MainWindow::onTaggingFinished()
{
    ui->lineEdit->setEnabled(true);
    ui->lineEdit_2->setEnabled(true);
    ui->progressBar->setEnabled(false);
}

void MainWindow::onSelectFiles()
{
    QList<QUrl> urls = QFileDialog::getOpenFileUrls(this,
                                                    "Select one one or more jpeg files",
                                                    QUrl(), "Jpeg files (*.jpg *.jpeg)");
    if (!urls.isEmpty())
    {
        ui->dropLabel->startTagFiles(urls);
    }
}

void MainWindow::onSelectFolders()
{
    QUrl url = QFileDialog::getExistingDirectoryUrl(this, "Select folder with jpeg files");
    if (!url.isEmpty())
    {
        QList<QUrl> urls;
        urls << url;
        ui->dropLabel->startTagFiles(urls);
    }
}

void MainWindow::updateSettings()
{
    m_settings->setValue("image.artist", ui->lineEdit->text());
    m_settings->setValue("image.copyright", ui->lineEdit_2->text());
}

void MainWindow::showAboutQtDialog()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::showAboutDialog()
{
    QString text;
    text.append("Copyright Tagger v 0.1.0\n\n");
    text.append("Copyright Tagger заполнит теги Exif.Image.Copyright и Exif.Image.Artist"\
                " для всех выбранных файлов в формате JPEG.\n\n");
    text.append("Автор: Антон Филимонов <anton.filimonov@gmail.com>.\n\n");
    text.append("Для работы с Exif используется exiv2 (www.exiv2.org).\n\n");
    text.append("Программа распространяется на условиях лицензии GPLv3.");

    QMessageBox::about(this, "О программе Copyright Tagger",
                       text);
}
