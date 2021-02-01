#include "mainwindow.h"
#include <QtWidgets>
#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

MainWindow::MainWindow()

{
    MainWindow::state = FlashFixState::unloaded;

    // Set layout in QWidget
    QWidget *window = new QWidget();

    // Set layout
    QGridLayout *layout = new QGridLayout(window);

    QLabel *infolabel = new QLabel;
    infolabel->setText("Select the file to patch (Flash.ocx)");
    layout->addWidget(infolabel,0,0,1,2);

    QPushButton *loadBtn = new QPushButton;
    loadBtn->setText("open file");
    layout->addWidget(loadBtn,1,0,1,2);

    fileLabel = new QLabel;
    fileLabel->setText("No file loaded");
    layout->addWidget(fileLabel,2,0,1,2);

    setInfBtn = new QPushButton;
    setInfBtn->setText("Set due date to infinity");
    layout->addWidget(setInfBtn,3,0,1,2);

    // Save Button
    setDefBtn = new QPushButton;
    setDefBtn->setText("Restore default due date");
    layout->addWidget(setDefBtn,4,0,1,2);

    saveAsBtn = new QPushButton;
    saveAsBtn->setText("Save file to ...");
    layout->addWidget(saveAsBtn,5,0,1,2);

    // Set QWidget as the central layout of the main window
    setCentralWidget(window);
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint );
    setUnifiedTitleAndToolBarOnMac(true);

    connect( loadBtn, SIGNAL(clicked()),this,SLOT(openFileClicked()));
    connect( setInfBtn, SIGNAL(clicked()),this,SLOT(patchFileClicked()));
    connect( setDefBtn, SIGNAL(clicked()),this,SLOT(restoreFileClicked()));
    connect( saveAsBtn, SIGNAL(clicked()),this,SLOT(saveFileClicked()));
    updateByState();

}


void MainWindow::updateByState(){
    QString stateStringFlashFix;

    switch (state) {
    case unloaded:
        setInfBtn->setEnabled(false);
        setDefBtn->setEnabled(false);
        saveAsBtn->setEnabled(false);

        fileLabel->setText("No file loaded");
        break;
    case loadedDefDueDate:
        setInfBtn->setEnabled(true);
        setDefBtn->setEnabled(false);
        saveAsBtn->setEnabled(true);

        stateStringFlashFix = QString("File loaded (due date: default):\n");
        stateStringFlashFix.append(path);
        fileLabel->setText(stateStringFlashFix);
        break;
    case loadedInfDueDate:
        setInfBtn->setEnabled(false);
        setDefBtn->setEnabled(true);
        saveAsBtn->setEnabled(true);

        stateStringFlashFix = QString("File loaded (due date: infinity):\n");
        stateStringFlashFix.append(path);
        fileLabel->setText(stateStringFlashFix);
        break;
    default:
        std::cerr << "Unknown state" << std::endl;
        break;
    }
}

void MainWindow::patchFileClicked(){
    uint8_t* dueDate = nullptr;

    if (state == loadedDefDueDate) {
        dueDate = reinterpret_cast<uint8_t*>(data.data()+FlashFixPos);
        std::memcpy(dueDate, &timestamp_infinity, 0x8);
        state = loadedInfDueDate;
    }
    updateByState();
}
void MainWindow::restoreFileClicked(){
    uint8_t* dueDate = nullptr;

    if (state == loadedInfDueDate) {
        dueDate = reinterpret_cast<uint8_t*>(data.data()+FlashFixPos);
        std::memcpy(dueDate, &timestamp_default, 0x8);
        state = loadedDefDueDate;
    }
    updateByState();
}

void MainWindow::openFileClicked() {
    QString filepath = QFileDialog::getOpenFileName();
    std::cout << "Try to open file from: " << filepath.toStdString() << std::endl;

    std::ifstream inputFile (filepath.toStdString(),std::ios::binary);
    if(inputFile.good()){
        // read file
        std::vector<char> const file(
            (std::istreambuf_iterator<char>(inputFile)),
            (std::istreambuf_iterator<char>())
            );
        MainWindow::path = filepath;
        MainWindow::data = std::vector<uint8_t>(file.begin(), file.end());

        if (file.size() == FlashBinarySize) {
            uint8_t* dueDate = reinterpret_cast<uint8_t*>(data.data()+FlashFixPos);
            if (std::memcmp(dueDate, &timestamp_default, 8) == 0) {
                std::cout << "File is: Flash.ocx (default due date)" << std::endl;
                MainWindow::state = FlashFixState::loadedDefDueDate;

            } else if (std::memcmp(dueDate, &timestamp_infinity, 8) == 0) {
                std::cout << "File is: Flash.ocx (infinity due date)" << std::endl;
                MainWindow::state = FlashFixState::loadedInfDueDate;
            } else {
                std::cout << "Unknown value at due date position." << std::endl;
                printErrorMessage("Unknown file! The file has an unknown value at due date position.");
                state = FlashFixState::unloaded;
            }
        } else {
            std::cout << "Unknown file with size " << std::to_string(file.size()) << std::endl;
            printErrorMessage("Unknown file! A file of the following size is expected: 0x15D0998 bytes");
            state = FlashFixState::unloaded;
        }
    } else {
        printErrorMessage("Error while reading the file");
        state = FlashFixState::unloaded;
    }
    inputFile.close();
    updateByState();
}

void MainWindow::saveFileClicked() {
    if ((state == loadedDefDueDate) || (state == loadedInfDueDate)) {
        QString filter("Flash OCX file (*.ocx)");

        printInfoMessage("Important: Save the file (temporarily) to an user directory (e.g. Desktop) to avoid permission issues!\nAvoid direct write operations to: C:\\Windows\\SysWOW64\\Macromed\\Flash\\Flash.ocx\nReplace the native Flash.ocx manually.");
        QString filepath = QFileDialog::getSaveFileName(0, "Save file", QDir::currentPath(), filter);

        // try to write file
        std::ofstream outFile (filepath.toStdString(),std::ios::out | std::ios::binary);
        if (outFile.good()){
            outFile.write(reinterpret_cast<char*>(data.data()),data.size());
            outFile.flush();
            printInfoMessage("File was successfully written!");
        } else {
            printErrorMessage("Error during file writing.","You can try to run this program as run as administrator.");
        }
        outFile.close();
    }
    updateByState();
}

void MainWindow::printErrorMessage(QString msg,QString detailMsg){
    QMessageBox errMsg;
    errMsg.setIcon(QMessageBox::Warning);
    if (detailMsg != nullptr)
        errMsg.setInformativeText(detailMsg);
    errMsg.setText(msg);
    errMsg.exec();
}

void MainWindow::printInfoMessage(QString msg,QString detailMsg){
    QMessageBox infoMsg;
    infoMsg.setIcon(QMessageBox::Information);
    if (detailMsg != nullptr)
        infoMsg.setInformativeText(detailMsg);
    infoMsg.setText(msg);
    infoMsg.exec();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
    //event->ignore();
}
