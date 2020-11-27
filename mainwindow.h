#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent * event);

private:
    Ui::MainWindow *ui;
    //void refresh();
    //void calibrate();

private slots:
   void on_btnConnectClicked();
   void on_btnRefreshClicked();
   void on_btnCalibrateClicked();
   void on_btnMoveClicked();
   void on_serialReceived();
   void on_sliderMoved(int value);

};
#endif // MAINWINDOW_H
