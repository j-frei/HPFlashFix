#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

enum FlashFixState { unloaded, loadedDefDueDate, loadedInfDueDate };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openFileClicked();
    void patchFileClicked();
    void restoreFileClicked();
    void saveFileClicked();

private:
    void printInfoMessage(QString msg, QString detailMsg = nullptr);
    void printErrorMessage(QString msg, QString detailMsg = nullptr);
    void updateByState();

    QLabel *fileLabel;
    QPushButton *setInfBtn;
    QPushButton *setDefBtn;
    QPushButton *saveAsBtn;

    FlashFixState state;
    QString path;
    std::vector<uint8_t> data;

    uint32_t FlashBinarySize = 0x15D0998;

    // Check via python3:
    // >>> from datetime import datetime
    // >>> import struct
    // >>> struct.pack("<d", datetime(year=2021,month=1,day=12,hour=1, minute=0).timestamp()*1000)
    uint8_t timestamp_default[8] = { 0x00, 0x00, 0x40, 0x46, 0x3E, 0x6F, 0x77, 0x42 };

    // Check via python3:
    // >>> import struct, math
    // >>> print(struct.pack("<d", math.inf))
    uint8_t timestamp_infinity[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x7F };

    uint32_t FlashFixPos = 0xF89C00;
};

#endif // MAINWINDOW_H
