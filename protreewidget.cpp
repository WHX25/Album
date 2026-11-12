#include "protreewidget.h"
#include "const.h"
#include "protreeitem.h"
#include "removeprodialog.h"
#include <QDir>
#include <QMenu>
#include <QHash>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QGuiApplication>

ProTreeWidget::ProTreeWidget(QWidget *parent):QTreeWidget(parent)
    ,_right_btn_item(nullptr),_active_item(nullptr),_dialog_progress(nullptr),
    _selected_item(nullptr),_thread_create_pro(nullptr),_thread_open_pro(nullptr),_open_progressdlg(nullptr),
    _current_index(0),isempty_list(false)
{
    //qRegisterMetaType<QVector<int> >("QVector<int>");
    this->header()->hide();
    connect(this,&ProTreeWidget::itemPressed,this,&ProTreeWidget::SlotItemPressed);
    _action_import = new QAction(QIcon(":/icon/import.png"),tr("导入文件"),this);
    _action_setstart = new QAction(QIcon(":/icon/core.png"), tr("设置活动项目"),this);
    _action_closepro = new QAction(QIcon(":/icon/close.png"), tr("关闭项目"), this);
    _action_slideshow = new QAction(QIcon(":/icon/slideshow.png"), tr("轮播图播放"),this);
    connect(_action_import,&QAction::triggered,this,&ProTreeWidget::SlotImport);
    connect(_action_setstart,&QAction::triggered,this,&ProTreeWidget::SlotSetActive);
    connect(_action_closepro,&QAction::triggered,this,&ProTreeWidget::SlotClosePro);
    connect(this,&ProTreeWidget::itemDoubleClicked,this,&ProTreeWidget::SlotDoubleClickItem);
    connect(_action_slideshow,&QAction::triggered,this,&ProTreeWidget::SlotSlideShow);
    // connect(_thread_create_pro.get(),&ProTreeThread::SigUpdateUi,this,[this](){
    //     _right_btn_item->setExpanded(true);
    //     this->viewport()->update();
    //     this->updateGeometries();
    // });

    _player = new QMediaPlayer(this);
    audio_output=new QAudioOutput(this);
    _player->setAudioOutput(audio_output);
}

void ProTreeWidget::AddProToTree(const QString &name, const QString &path)
{
    QDir dir(path);
    QString file_path = dir.absoluteFilePath(name);
    if(_set_path.find(file_path) != _set_path.end()){
        return;
    }
    QDir pro_path(file_path);
    if(!pro_path.exists()){
        bool enable = pro_path.mkpath(file_path);
        if(!enable){
            return;
        }
    }

    _set_path.insert(file_path);
    auto* item = new ProTreeItem(this,name,file_path,TreeItemPro);
    item->setData(0,Qt::DisplayRole,name);
    item->setData(0,Qt::DecorationRole,QIcon(":/icon/dir.png"));
    item->setData(0,Qt::ToolTipRole,file_path);
    this->addTopLevelItem(item);
}

QString ProTreeWidget::GetMusicUrl()
{
    if(_playlist.isEmpty())
    {
        isempty_list=true;
        qDebug()<<"音频为空直接退出";
        return "";
    }
    qDebug() << "Calling GetMusicUrl()";
    if(_current_index>=_playlist.size())
    {
        _current_index=0;
    }

    QString tmp=_playlist[_current_index];
    _current_index++;
    qDebug()<<"返回音频地址"<<tmp;
    qDebug()<<"_currentIndex="<<_current_index;
    return tmp;
}

void ProTreeWidget::SlotItemPressed(QTreeWidgetItem *pressedItem, int column)
{
    if(QGuiApplication::mouseButtons() == Qt::RightButton){
        QMenu menu(this);
        qDebug() << "menu addr is " << &menu << Qt::endl;
        int itemtype = (int)(pressedItem->type());
        if(itemtype == TreeItemPro){
            _right_btn_item = pressedItem;
            menu.addAction(_action_import);
            menu.addAction(_action_setstart);
            menu.addAction(_action_closepro);
            menu.addAction(_action_slideshow);
            menu.exec(QCursor::pos());
        }
    }
}

void ProTreeWidget::SlotDoubleClickItem(QTreeWidgetItem *item, int column)
{
    if(QGuiApplication::mouseButtons() == Qt::LeftButton){
        auto * tree_doubleItem = dynamic_cast<ProTreeItem*>(item);
        if(!tree_doubleItem){
            return;
        }
        int itemtype = tree_doubleItem->type();
        if(itemtype == TreeItemPic){
            emit SigUpdateSelected(tree_doubleItem->GetPath());
            _selected_item = item;
        }
    }
}

void ProTreeWidget::SlotImport()
{

    if(_thread_create_pro && _thread_create_pro->isRunning()){
        return;
    }
    auto *rootItem = _right_btn_item;
    if(!rootItem){
        return;
    }
    const QString targetPath = dynamic_cast<ProTreeItem*>(rootItem)->GetPath();

    QFileDialog file_dialog;
    file_dialog.setFileMode(QFileDialog::Directory);
    file_dialog.setWindowTitle(tr("选择导入的文件夹"));
    file_dialog.setDirectory(targetPath);
    file_dialog.setViewMode(QFileDialog::Detail);

    if(file_dialog.exec() != QDialog::Accepted || file_dialog.selectedFiles().isEmpty()){
        return;
    }

    const QString sourcePath = file_dialog.selectedFiles().first();
    _importRootItem = rootItem;

    //创建模态对话框
    _dialog_progress = new QProgressDialog(tr("正在导入图片..."),tr("取消"),0,0,this);
    _dialog_progress->setWindowTitle(tr("请稍候"));
    _dialog_progress->setFixedWidth(PROGRESS_WIDTH);
    _dialog_progress->setWindowModality(Qt::WindowModal);
    //耗时操作放在线程中操作
    _thread_create_pro = std::make_shared<ProTreeThread>(sourcePath,targetPath);
    //连接更新进度框操作
    connect(_thread_create_pro.get(),&ProTreeThread::SigUpdateProgress,this,&ProTreeWidget::SlotUpdateProgress);

    connect(_thread_create_pro.get(),&ProTreeThread::SigImportReady,this,&ProTreeWidget::SlotImportReady);

    connect(_thread_create_pro.get(),&ProTreeThread::SigImportFailed,this,&ProTreeWidget::SlotImportFailed);

    connect(_thread_create_pro.get(),&ProTreeThread::SigImportCanceled,this,&ProTreeWidget::SlotImportCalceled);

    connect(_dialog_progress,&QProgressDialog::canceled,this,&ProTreeWidget::SlotCancelProgress);

    connect(this,&ProTreeWidget::SigCancelProgress,_thread_create_pro.get(),&ProTreeThread::SlotCancelProgress);
    _thread_create_pro->start();

    _dialog_progress->exec();

}

void ProTreeWidget::SlotSetActive()
{
    if(!_right_btn_item){
        return;
    }
    QFont nullFont;
    nullFont.setBold(false);

    if(_active_item){
        _active_item->setFont(0,nullFont);
    }

    _active_item = _right_btn_item;
    nullFont.setBold(true);
    _active_item->setFont(0,nullFont);
}

void ProTreeWidget::SlotClosePro()
{
    if (_thread_create_pro && _thread_create_pro->isRunning()) {
        return;
    }

    RemoveProDialog remove_pro_dialog;
    auto res = remove_pro_dialog.exec();
    if(res != QDialog::Accepted){
        return;
    }

    bool b_remove = remove_pro_dialog.IsRemoved();
    auto index_right_btn = this->indexOfTopLevelItem(_right_btn_item);
    auto* protreeitem = dynamic_cast<ProTreeItem*>(_right_btn_item);
    auto* selecteditem = dynamic_cast<ProTreeItem*>(_selected_item);
    auto delete_path = protreeitem->GetPath();
    _set_path.remove(delete_path);
    if(b_remove){
        QDir remove_dir(delete_path);
        remove_dir.removeRecursively();
    }

    if(protreeitem == _active_item){
        _active_item = nullptr;
    }

    if(selecteditem && protreeitem == selecteditem->GetRoot()){
        selecteditem = nullptr;
        _selected_item = nullptr;
        emit SigClearSelected();
    }

    delete this->takeTopLevelItem(index_right_btn);
    _right_btn_item = nullptr;
}

void ProTreeWidget::SlotUpdateProgress(int count)
{
    if(_dialog_progress){
        _dialog_progress->setLabelText(tr("正在导入图片，已处理 %1 项...").arg(count));
    }
}

void ProTreeWidget::SlotCancelProgress()
{
    if(_thread_create_pro && _thread_create_pro->isRunning()){
        emit SigCancelProgress();

        if(_dialog_progress){
            _dialog_progress->setLabelText(tr("正在取消..."));
        }
    }
}

void ProTreeWidget::SlotImportReady(QVector<ImportEntry> entries)
{
    if(!_importRootItem){
        return;
    }

    QHash<QString,QTreeWidgetItem*> directoryItems;
    directoryItems.insert(QString(),_importRootItem);

    auto *rootProItem = dynamic_cast<ProTreeItem*>(_importRootItem);
    ProTreeItem* previousPic = rootProItem ? rootProItem->GetLastPicChild():nullptr;

    for(const ImportEntry &entry : entries){
        const QString relativePath = QDir::cleanPath(entry.relativePath);

        QString parentPath = QFileInfo(relativePath).path();
        if(parentPath == "."){
            parentPath.clear();
        }

        QTreeWidgetItem *parentItem = directoryItems.value(parentPath,_importRootItem);

        auto *item = new ProTreeItem(parentItem,entry.name,entry.targetPath,_importRootItem,entry.type);

        item->setData(0,Qt::DisplayRole,entry.name);
        item->setData(0,Qt::DecorationRole,entry.type==TreeItemDir?QIcon(":/icon/dir.png"):QIcon(":/icon/pic.png"));
        item->setData(0,Qt::ToolTipRole,entry.relativePath);

        if(entry.type == TreeItemDir){
            directoryItems.insert(relativePath,item);
        }else if(entry.type == TreeItemPic){
            auto *picItem = static_cast<ProTreeItem*>(item);

            if(previousPic){
                previousPic->SetNextItem(picItem);
                picItem->SetPreItem(previousPic);
            }

            previousPic = picItem;
        }
    }

    _importRootItem->setExpanded(true);
    _importRootItem = nullptr;

    if(_dialog_progress){
        _dialog_progress->hide();
        _dialog_progress->deleteLater();
        _dialog_progress = nullptr;
    }
}

void ProTreeWidget::SlotImportFailed(QString message)
{
    _importRootItem = nullptr;

    if(_dialog_progress){
        _dialog_progress->hide();
        _dialog_progress->deleteLater();
        _dialog_progress = nullptr;
    }

    QMessageBox::warning(this,tr("导入失败"),message);
}

void ProTreeWidget::SlotImportCalceled()
{
    _importRootItem = nullptr;

    if(_dialog_progress){
        _dialog_progress->hide();
        _dialog_progress->deleteLater();
        _dialog_progress = nullptr;
    }
}



void ProTreeWidget::SlotUpOpenProgress(int count)
{
    if(_open_progressdlg){
        _open_progressdlg->setLabelText(tr("正在处理目录，已处理 %1 项").arg(count));
    }
}

void ProTreeWidget::SlotCancelOpenProgress()
{
    if(_thread_open_pro && _thread_open_pro->isRunning()){
        emit SigCancelOpenProgress();
    }
}

void ProTreeWidget::SlotOpenReady(QVector<ImportEntry> entries)
{
    const QString projectPath = _openingPath;
    const QString projectName = QFileInfo(projectPath).fileName();

    auto *rootItem = new ProTreeItem(this,projectName,projectPath,TreeItemPro);

    rootItem->setData(0,Qt::DisplayRole,projectName);
    rootItem->setData(0,Qt::DecorationRole,QIcon(":/icon/dir.png"));
    rootItem->setData(0,Qt::ToolTipRole,projectPath);

    QHash<QString,QTreeWidgetItem*> directoryItems;
    directoryItems.insert(QString(),rootItem);

    ProTreeItem *previousPic = nullptr;

    for(const ImportEntry &entry : entries){
        const QString relativePath = QDir::cleanPath(entry.relativePath);

        QString parentPath = QFileInfo(relativePath).path();
        if(parentPath == "."){
            parentPath.clear();
        }
        QTreeWidgetItem *parentItem = directoryItems.value(parentPath,rootItem);

        auto *item = new ProTreeItem(parentItem,entry.name,entry.targetPath,rootItem,entry.type);

        item->setData(0,Qt::DisplayRole,entry.name);
        item->setData(0,Qt::DecorationRole,entry.type==TreeItemDir?QIcon(":/icon/dir.png"):QIcon(":/icon/pic.png"));
        item->setData(0,Qt::ToolTipRole,entry.targetPath);

        if(entry.type == TreeItemDir){
            directoryItems.insert(relativePath,item);
        }else if(entry.type == TreeItemPic){
            auto *picItem = static_cast<ProTreeItem*>(item);
            if(previousPic){
                previousPic->SetNextItem(picItem);
                picItem->SetPreItem(previousPic);
            }

            previousPic = item;
        }
    }

    rootItem->setExpanded(true);
    _openingPath.clear();

    if(_open_progressdlg){
        _open_progressdlg->hide();
        _open_progressdlg->deleteLater();
        _open_progressdlg = nullptr;
    }
}

void ProTreeWidget::SlotOpenCanceled()
{
    _set_path.remove(_openingPath);
    _openingPath.clear();

    if(_open_progressdlg){
        _open_progressdlg->hide();
        _open_progressdlg->deleteLater();
        _open_progressdlg = nullptr;
    }
}



void ProTreeWidget::SlotSlideShow()
{
    if(!_right_btn_item){
        return;
    }
    auto * right_pro_item = dynamic_cast<ProTreeItem*>(_right_btn_item);
    auto * last_child_item = right_pro_item->GetLastPicChild();
    if(!last_child_item){
        return;
    }

    auto * first_child_item = right_pro_item->GetFirstPicChild();
    if(!first_child_item){
        return;
    }
    qDebug()<<"first_child_item path is "<<first_child_item->GetPath();
    qDebug()<<"last_child_item path is "<<last_child_item->GetPath();

    _slide_show_dlg = std::make_shared<SlideShowDlg>(this,first_child_item,last_child_item);
    _slide_show_dlg->setModal(true);
    _slide_show_dlg->showMaximized();
}

void ProTreeWidget::SlotOpenPro(const QString &path)
{
    if((_thread_open_pro && _thread_open_pro->isRunning()) || _set_path.contains(path)){
        return;
    }

    _openingPath = path;
    _set_path.insert(path);

    _thread_open_pro = std::make_shared<OpenTreeThread>(path);

    _open_progressdlg = new QProgressDialog(this);
    _open_progressdlg->setWindowTitle(tr("正在打开项目"));
    _open_progressdlg->setLabelText(tr("正在扫描目录..."));
    _open_progressdlg->setCancelButtonText(tr("取消"));
    _open_progressdlg->setRange(0,0);
    _open_progressdlg->setMinimumDuration(0);
    _open_progressdlg->setFixedWidth(PROGRESS_WIDTH);
    _open_progressdlg->setWindowModality(Qt::WindowModal);

    connect(_thread_open_pro.get(),&OpenTreeThread::SigUpdateProgress,this,&ProTreeWidget::SlotUpOpenProgress);

    connect(_thread_open_pro.get(),&OpenTreeThread::SigOpenReady,this,&ProTreeWidget::SlotOpenReady);

    connect(_thread_open_pro.get(),&OpenTreeThread::SigOpenCanceled,this,&ProTreeWidget::SlotOpenCanceled);

    connect(_open_progressdlg,&QProgressDialog::canceled,this,&ProTreeWidget::SlotCancelOpenProgress);

    connect(this,&ProTreeWidget::SigCancelOpenProgress,_thread_open_pro.get(),&OpenTreeThread::SlotCancelProgress);

    _thread_open_pro->start();
    _open_progressdlg->exec();
}

void ProTreeWidget::SlotNextShow()
{
    if(!_selected_item){
        return;
    }
    auto* curItem = dynamic_cast<ProTreeItem*>(_selected_item)->GetNextItem();
    if(!curItem){
        return;
    }

    emit SigUpdatePic(curItem->GetPath());
    _selected_item = curItem;
    this->setCurrentItem(curItem);
}

void ProTreeWidget::SlotPreShow()
{
    if(!_selected_item){
        return;
    }
    auto* curItem = dynamic_cast<ProTreeItem*>(_selected_item)->GetPreItem();
    if(!curItem){
        return;
    }

    emit SigUpdatePic(curItem->GetPath());
    _selected_item = curItem;
    this->setCurrentItem(curItem);
}

void ProTreeWidget::SlotSetMusic()
{
    QFileDialog file_dialog;
    file_dialog.setFileMode(QFileDialog::ExistingFile);
    file_dialog.setWindowTitle(tr("选择导入的音频文件"));
    file_dialog.setDirectory(QDir::currentPath());
    file_dialog.setViewMode(QFileDialog::Detail);
    file_dialog.setNameFilter("(*.mp3)");
    QStringList fileNames;
    if(file_dialog.exec()){
        fileNames = file_dialog.selectedFiles();
    }else{
        return;
    }

    _playlist.clear();
    for(auto filename:fileNames){
        _playlist.append(filename);
    }

    if(_player->mediaStatus() != QMediaPlayer::PlayingState){
        _current_index = 0;
    }

    _player->setSource(QUrl::fromLocalFile(GetMusicUrl()));
}

void ProTreeWidget::SlotStartMusic()
{
    _player->play();
}

void ProTreeWidget::SlotStopMusic()
{
    _player->stop();
}
