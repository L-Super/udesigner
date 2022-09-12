//
// Created by Listening on 2022/1/3.
//

// You may need to build the project (run Qt uic code generator) to get "ui_UDesigner.h" resolved

#include "udesigner.h"
#include "ui_UDesigner.h"
#include <QStandardPaths>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>
#include <QIcon>
#include <QDir>

#define cout qDebug()<<"["<<__func__<<__LINE__<<"]"

UDesigner::UDesigner(QWidget *parent)
        :
        QWidget(parent), ui(new Ui::UDesigner) {
    ui->setupUi(this);

    //TODO:rc文件暂不知道如何cmake
    setWindowIcon(QIcon(":resources/U.png"));
    GetDrives();
// 不知道为什么自动关联了信号
//    connect(ui->okBtn,&QPushButton::clicked,this,&UDesigner::on_okBtn_clicked);
    connect(ui->toolButton, &QToolButton::clicked, this, &UDesigner::GetIcon);
}

UDesigner::~UDesigner() {
    delete ui;
}

//获取当前系统的盘符
void UDesigner::GetDrives() {
    QStringList driveList;

//    QFileInfoList drives =  QDir::drives();
//    for(int i=0; i < drives.count(); i++)
//    {
//        driveList << drives[i].filePath();
//    }

            foreach(QFileInfo drive, QDir::drives()) {
            driveList << drive.filePath();
        }

    ui->comboBox->addItems(driveList);
}

void UDesigner::SelectDrive(const QString &drive) {
    selectDrive = drive;
}

void UDesigner::GetIcon() {
    // standardLocations
    auto defaultPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    icoPath = QFileDialog::getOpenFileName(this, tr("选择图标文件"),
                                           defaultPath,
                                           tr("图片文件(*.ico)"));
    cout << icoPath;
    icoInfo = QFileInfo(icoPath);
    ui->lineEdit->setText(icoPath);
}

// 拷贝文件到对应磁盘
bool UDesigner::CopyFile2Drive(const QString &filePath) {
    QFile file(filePath);
    QFileInfo fileInfo(file);

    if (!file.exists())
        return false;

    // fileName() 输出类似 "autorun.inf"
    cout << fileInfo.fileName();
    auto desPath = selectDrive + "/";
    if (fileInfo.fileName() == icoInfo.fileName()) {
        cout << "ico copy and rename";
        if (!file.copy(desPath.append("icon.ico"))) {
            cout << "ico copy failed";
            return false;
        }
    }
        // 如果是inf文件，则复制后移除
    else if (fileInfo.baseName() == infInfo.baseName()) {
        cout << "move inf file";
        auto desfile = desPath.append(fileInfo.fileName());
        cout << desfile;
        // 重命名并移动到新路径
        if (!file.rename(desfile)) {
            cout << "inf file copy failed";
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
    out << "[autorun]" << "\n" << "icon = icon.ico";
    file.close();
    if (!file.exists()) {
        cout << "file create failed";
        return false;
    }
    infInfo = QFileInfo(file);
    return true;
}

// 启动QProcess设置inf,ico文件隐藏
void UDesigner::HideIcon2Drive() {
    QProcess *p = new QProcess(this);
    //TODO:输出信息写入日志
    connect(p, &QProcess::readyReadStandardOutput, this, [&] {
        auto *pProcess = (QProcess *) sender();
        auto output = pProcess->readAllStandardOutput();
        cout << output;
    });
// 输出错误信息
    connect(p, &QProcess::readyReadStandardError, this, [&] {
        auto errorOutput = p->readAllStandardError();
        cout << errorOutput;
    });

    QStringList args;
    args << "/c" << selectDrive << "&&";
//    args<<"/c"<<"D:"<<"&&";
    args << "attrib" << "+s" << "+h" << "icon.ico" << "&&";
    args << "attrib" << "+s" << "+h" << "autorun.inf";

    p->start("cmd", args);
    if (!p->waitForFinished()) {
        cout << "QProcess fail";
        return;
    }

    QMessageBox *msgBox = new QMessageBox(this);
    QPixmap pixmap(":resources/img/info.png");
    msgBox->setIconPixmap(pixmap.scaled(40, 40));
    msgBox->setText("恭喜！\n创建个性化U盘图标成功！");
    msgBox->setInformativeText("请插拔U盘或重启电脑生效");
    msgBox->setWindowTitle("Success！");
    msgBox->exec();
}

// 移除图标
void UDesigner::RemoveIcon() {
    auto currentDrive = ui->comboBox->currentText();
    QFile icoFile(currentDrive + "icon.ico");
    QFile infFile(currentDrive + "autorun.inf");

    if (icoFile.exists()) {
        if (!icoFile.remove()) {
            cout << "icoFile remove failed";
        }
    }
    if (infFile.exists()) {
        if (infFile.remove()) {
            cout << "infFile remove failed";
        }
    }
}

void UDesigner::on_okBtn_clicked() {
    auto sDrive = ui->comboBox->currentText();
    // C:/ 去掉'/'
    QString drive = sDrive.left(sDrive.lastIndexOf("/"));
    cout << drive;
    SelectDrive(drive);

    if (icoPath.isEmpty()) {
        QMessageBox *msgBox = new QMessageBox(this);
        // 无效，需要new并指定父对象，才能显示ico图标
//        msgBox.setWindowIcon(QIcon(":resources/ico.ico"));
        QPixmap pixmap(":resources/img/warn.png");
        msgBox->setIconPixmap(pixmap.scaled(40, 40));
        msgBox->setText("请选择图标！");
        msgBox->setWindowTitle("警告");
        msgBox->exec();
        return;
    }

    if (!CopyFile2Drive(icoPath)) {
        cout << "copy ico failed";
    }
    if (!WriteInf2Drive()) {
        cout << "write inf failed";
    }
    if (!CopyFile2Drive(infInfo.absoluteFilePath())) {
        cout << "copy ico failed";
    }

    HideIcon2Drive();
}

void UDesigner::on_cancelBtn_clicked() {
    RemoveIcon();
    auto *rBox = new QMessageBox(this);
    rBox->setText("移除图标成功！");
    rBox->setInformativeText("重启或插拔U盘生效");
    rBox->setWindowTitle("成功");
    rBox->exec();
}

void UDesigner::on_helpBtn_clicked() {
    QString text{"UDesigner是一个自定义磁盘图标的小工具"};
    text.append("<p>链接：<a href = 'https://github.com/L-Super/udesigner' style='color:#bdc3c7'>Github</a></p>");
    auto box = new QMessageBox(this);
    box->setIconPixmap(
            QPixmap(":resources/img/info.png").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    box->setText(text);
    box->setInformativeText("<p align=\"right\">by Listening 2022.01.06</p>");
    box->setWindowTitle("关于");
    box->exec();
}







