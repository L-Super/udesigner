//
// Created by Listening on 2022/1/3.
//

#ifndef UDESIGNER_UDESIGNER_H
#define UDESIGNER_UDESIGNER_H

#include <QWidget>
#include <QFileInfo>

QT_BEGIN_NAMESPACE
namespace Ui { class UDesigner; }
QT_END_NAMESPACE

class UDesigner : public QWidget {
Q_OBJECT

public:
    explicit UDesigner(QWidget *parent = nullptr);
    ~UDesigner() override;

    // 获取盘符
    void GetDrives();
    // 选择盘符
    void SelectDrive(const QString& drive);
    // 获取图标
    void GetIcon();
    // 拷贝文件到对应磁盘
    bool CopyFile2Drive(const QString& file);
    // 写入inf文件到磁盘
    bool WriteInf2Drive();
    // 隐藏盘符图标
    void HideIcon2Drive();
	// 移除图标
	void RemoveIcon();

private slots:
    void on_okBtn_clicked();
	void on_cancelBtn_clicked();
	void on_helpBtn_clicked();

private:
    Ui::UDesigner *ui;
    QString selectDrive; //选择的盘符
    QString icoPath;
    QFileInfo icoInfo;
    QFileInfo infInfo; // inf文件
};


#endif //UDESIGNER_UDESIGNER_H
