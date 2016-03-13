#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateSettings();
    void showAboutDialog();
    void showAboutQtDialog();

private:
    Ui::MainWindow *ui;
    QSettings* m_settings;
};

#endif // MAINWINDOW_HPP
