#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

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

    connect(ui->pushButton, SIGNAL(clicked(bool)), SLOT(onSelectFolder()));
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

void MainWindow::onSelectFolder()
{
    QString path = QFileDialog::getExistingDirectory(this, "Select folder with jpeg files");
    if (!path.isEmpty())
    {
        qDebug() << "Selected path: " << path;
        ui->dropLabel->tagInFolder(path);
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
    text.append("Drop folder to this program and it will fill"\
                " Exif.Image.Copyright and Exif.Image.Artist"\
                " tags for all jpeg images in the dropped folder.\n\n");
    text.append("Author: Anton Filimonov <anton.filimonov@gmail.com>.\n\n");
    text.append("This program uses exiv2 library (www.exiv2.org).\n\n");
    text.append("This program is distributed under GPLv3 license.");

    QMessageBox::about(this, "About Copyright Tagger",
                       text);
}
