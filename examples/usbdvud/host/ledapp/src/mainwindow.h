#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void VendorUniqueSetLedCmd( int ledNo, int on );
    void UpdateLedsOnForm( void );
    uint8_t VendorUniqueGetLedsCmd( void );
    void ConnectUSBDevice( void );
    void DisconnectUSBDevice( void );

private:
    Ui::MainWindow *ui;
    
private slots:
    void on_pushButton0_clicked( void );
    void on_pushButton1_clicked( void );
    void on_pushButton2_clicked( void );
    void on_pushButton3_clicked( void );
    void on_pushButton4_clicked( void );
};

#endif // MAINWINDOW_H
