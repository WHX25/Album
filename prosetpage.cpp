#include "prosetpage.h"
#include "ui_prosetpage.h"
#include <QDir>
#include <QFileDialog>

ProSetPage::ProSetPage(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::ProSetPage)
{
    ui->setupUi(this);
    registerField("proPath",ui->path_ed);
    registerField("proName",ui->name_ed);
    connect(ui->path_ed,&QLineEdit::textEdited,this,&ProSetPage::completeChanged);
    connect(ui->name_ed,&QLineEdit::textEdited,this,&ProSetPage::completeChanged);
    QString curPath = QDir::currentPath();
    ui->path_ed->setText(curPath);
    ui->path_ed->setCursorPosition(ui->path_ed->text().size());
    ui->path_ed->setClearButtonEnabled(true);
    ui->name_ed->setClearButtonEnabled(true);


}

ProSetPage::~ProSetPage()
{
    delete ui;
}

void ProSetPage::GetProSettings(QString &name, QString &path)
{
    name = ui->name_ed->text();
    path = ui->path_ed->text();
}

bool ProSetPage::isComplete() const
{
    if(ui->name_ed->text() == "" || ui->path_ed->text() == ""){
        return false;
    }
    QDir dir(ui->path_ed->text());
    if(!dir.exists()){
        ui->tips_lb->setText("project path is not exists");
        return false;
    }

    //判断路径
    QString absFilePath = dir.absoluteFilePath(ui->name_ed->text());
    QDir dist_dir(absFilePath);
    if(dist_dir.exists()){
        ui->tips_lb->setText("project has exists,change path or name!");
        return false;
    }

    ui->tips_lb->setText("");
    return QWizardPage::isComplete();
}

void ProSetPage::on_browse_btn_clicked()
{
    QFileDialog file_dialog;
    file_dialog.setFileMode(QFileDialog::Directory);
    file_dialog.setWindowTitle("选择导入的文件夹");
    auto path = QDir::currentPath();
    file_dialog.setDirectory(path);
    file_dialog.setViewMode(QFileDialog::Detail);

    QStringList fileNames;
    if(file_dialog.exec()){
        fileNames = file_dialog.selectedFiles();
    }

    if(fileNames.length() <= 0){
        return;
    }

    QString import_path = fileNames.at(0);
    ui->path_ed->setText(import_path);
}

