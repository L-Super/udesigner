//
// Created by Listening on 2022/1/3.
//

// You may need to build the project (run Qt uic code generator) to get "ui_UDesigner.h" resolved

#include "udesigner.h"
#include "ui_UDesigner.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>
#include <QProcess>
#include <QMessageBox>

#define cout qDebug()<<"["<<__func__<<__LINE__<<"]"

UDesigner::UDesigner(QWidget *parent) :
        QWidget(parent), ui(new Ui::UDesigner) {
    ui->setupUi(this);
//    setWindowIcon(QIcon("resources/U.png"));
    GetDrives();

    connect(ui->okBtn,&QPushButton::clicked,this,&UDesigner::on_okBtn_clicked);
    connect(ui->toolButton,&QToolButton::clicked,this,&UDesigner::GetIcon);
}

UDesigner::~UDesigner() {
    delete ui;
}

void UDesigner::GetDrives() {
    QFileInfoList drives =  QDir::drives();  //获取当前系统的盘符
    QStringList driveList;
    for(int i=0; i < drives.count(); i++)
    {
        driveList << drives[i].filePath();
    }
    ui->comboBox->addItems(driveList);
}

void UDesigner::SelectDrive(const QString &drive) {
    selectDrive = drive;
}

void UDesigner::GetIcon() {
    // standardLocations
    auto defaultPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    icoPath = QFileDialog::getOpenFileName(this,tr("选择图标文件"),
                                                    defaultPath,
                                                    tr("图片文件(*.ico)"));
    cout<<icoPath;
    icoInfo = QFileInfo(icoPath);
    ui->lineEdit->setText(icoPath);
}

// 拷贝文件到对应磁盘
bool UDesigner::CopyFile2Drive(const QString& filePath) {
    QFile file(filePath);
    QFileInfo fileInfo(file);

    if (!file.exists())
        return false;

    // fileName() 输出类似 "autorun.inf"
    cout<<fileInfo.fileName();
    auto desPath = selectDrive + "/";
    if(fileInfo.fileName() == icoInfo.fileName())
    {
        cout<<"ico copy and rename";
        if(!file.copy(desPath.append("icon.ico")))
        {
            cout<<"ico copy failed";
            return false;
        }
    }
    // 如果是inf文件，则复制后移除
    else if(fileInfo.baseName() == infInfo.baseName())
    {
        cout<<"move inf file";
        auto desfile = desPath.append(fileInfo.fileName());
        cout << desfile;
        // 重命名并移动到新路径
        if(!file.rename(desfile))
        {
            cout<<"inf file copy failed";
            return false;
        }
    }
    return true;
}

// 在磁盘中创建文件autorun.inf，并写入内容
// [autorun]
// icon = icon.ico
bool UDesigner::WriteInf2Drive() {
    QFile file("autorun.inf");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "[autorun]"<< "\n"<<"icon = icon.ico";
    file.close();
    if(!file.exists())
    {
        cout<<"file create failed";
        return false;
    }
    infInfo = QFileInfo(file);
    return true;
}

// 启动QProcess设置inf,ico文件隐藏
void UDesigner::HideIcon2Drive() {
    QProcess* p = new QProcess(this);
    //TODO:输出信息写入日志
//    connect(p,&QProcess::readyRead,this,[=]{
//        auto info = p->readAll();
//        cout<<info;
//    });
    connect(p, &QProcess::readyReadStandardOutput, this, [&]{
        QProcess *pProcess = (QProcess *)sender();
        auto output = pProcess->readAllStandardOutput();
        cout<<output;
    });
// 输出错误信息
    connect(p, &QProcess::readyReadStandardError, this, [&]{
        auto errorOutput = p->readAllStandardError();
        cout<<errorOutput;
    });

    //TODO:尾加上inf，ico文件
    QStringList icoArgs;
    icoArgs << "attrib" << "+s" << "+h" << icoInfo.fileName();
    QStringList infArgs;
    infArgs << "attrib" << "+s" << "+h"<<"autorun.inf";

    QStringList args;
    args<<"/c"<<selectDrive<<"&&";
//    args<<"/c"<<"D:"<<"&&";
    args<<"attrib" << "+s" << "+h" << "icon.ico"<<"&&";
    args<<"attrib" << "+s" << "+h" << "autorun.inf";

//    args<<"/c"<<"D:"<<"&&"<<"mkdir"<<"a";
//    args<<"&&"<<"mkdir"<<"b";
    p->start("cmd", args);
    if (!p->waitForFinished())
    {
        cout<<"QProcess fail";
        return;
    }
    QMessageBox::information(this,"Success!","恭喜！\n创建个性化U盘图标成功！");
}

void UDesigner::on_okBtn_clicked() {
    auto sDrive = ui->comboBox->currentText();
    // C:/ 去掉'/'
    QString drive = sDrive.left(sDrive.lastIndexOf("/"));
    cout <<drive;
    SelectDrive(drive);

    if(icoPath.isEmpty())
    {
        QMessageBox::warning(this,"警告","请选择图标",QMessageBox::Ok);
        return;
    }

    if(!CopyFile2Drive(icoPath))
    {
        cout<<"copy ico failed";
    }
    if(!WriteInf2Drive())
    {
        cout<<"write inf failed";
    }
    if(!CopyFile2Drive(infInfo.absoluteFilePath()))
    {
        cout<<"copy ico failed";
    }

    HideIcon2Drive();
}







