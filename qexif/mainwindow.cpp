#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings(new QSettings("tags.ini", QSettings::IniFormat, this))
{
    ui->setupUi(this);

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
}

MainWindow::~MainWindow()
{
    delete ui;
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
