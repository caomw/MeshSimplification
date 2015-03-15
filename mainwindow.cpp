#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "meshviewer.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(new MeshViewer);
}

MainWindow::~MainWindow()
{
    delete ui;
}
