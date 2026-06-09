#include "protreewidget.h"
#include "const.h"
#include "protreeitem.h"
#include "removeprodialog.h"
#include <QDir>
#include <QMenu>
#include <QHeaderView>
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
    QFileDialog file_dialog;
    file_dialog.setFileMode(QFileDialog::Directory);
    file_dialog.setWindowTitle("选择导入的文件夹");
    QString path = "";
    if(!_right_btn_item){
        qDebug() << "_right_btn_item is empty" << Qt::endl;
        path = QDir::currentPath();
        return;
    }
    path = dynamic_cast<ProTreeItem*>(_right_btn_item)->GetPath();

    file_dialog.setDirectory(path);
    file_dialog.setViewMode(QFileDialog::Detail);

    QStringList fileNames;
    if(file_dialog.exec()){
        fileNames = file_dialog.selectedFiles();
    }
    if(fileNames.length()<=0){
        return;
    }

    QString import_path = fileNames.at(0);
    int file_count = 0;
    //创建模态对话框
    _dialog_progress = new QProgressDialog(this);
    //耗时操作放在线程中操作
    _thread_create_pro = std::make_shared<ProTreeThread>(std::ref(import_path),std::ref(path),
                                                        _right_btn_item,std::ref(file_count),
                                                         this,_right_btn_item,nullptr);
    //连接更新进度框操作
    connect(_thread_create_pro.get(),&ProTreeThread::SigUpdateProgress,this,&ProTreeWidget::SlotUpdateProgress);

    connect(_thread_create_pro.get(),&ProTreeThread::SigFinishProgress,this,&ProTreeWidget::SlotFinishProgress);

    connect(_dialog_progress,&QProgressDialog::canceled,this,&ProTreeWidget::SlotCancelProgress);

    connect(this,&ProTreeWidget::SigCancelProgress,_thread_create_pro.get(),&ProTreeThread::SlotCancelProgress);
    _thread_create_pro->start();

    _dialog_progress->setWindowTitle("Please wait...");
    _dialog_progress->setFixedWidth(PROGRESS_WIDTH);
    _dialog_progress->setRange(0,PROGRESS_MAX);
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
    qDebug() << "count is " << count;
    if(!_dialog_progress){
        qDebug() << "dialog_progress is empty!!!";
        return;
    }
    _dialog_progress->setValue(count%PROGRESS_MAX);
}

void ProTreeWidget::SlotCancelProgress()
{
    emit SigCancelProgress();
    if(_dialog_progress){
        delete _dialog_progress;
        _dialog_progress = nullptr;
    }
}

void ProTreeWidget::SlotFinishProgress()
{
    _dialog_progress->setValue(PROGRESS_MAX);
    _dialog_progress->deleteLater();
}

void ProTreeWidget::SlotUpOpenProgress(int count)
{
    if(!_open_progressdlg){
        return;
    }

    _open_progressdlg->setValue(count%PROGRESS_MAX);
}

void ProTreeWidget::SlotCancelOpenProgress()
{
    emit SigCancelOpenProgress();
    if(_open_progressdlg){
        delete _open_progressdlg;
        _open_progressdlg = nullptr;
    }
}

void ProTreeWidget::SlotFinishOpenProgress()
{
    if(!_open_progressdlg){
        return;
    }
    _open_progressdlg->setValue(PROGRESS_MAX);
    _open_progressdlg->deleteLater();
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
    if(_set_path.find(path) != _set_path.end()){
        return;
    }

    _set_path.insert(path);
    int file_count = 0;
    QDir pro_path(path);
    QString proname = pro_path.dirName();
    _thread_open_pro = std::make_shared<OpenTreeThread>(path,file_count,this,nullptr);

    _open_progressdlg = new QProgressDialog(this);

    connect(_thread_open_pro.get(),&OpenTreeThread::SigUpdateProgress,this,&ProTreeWidget::SlotUpOpenProgress);

    connect(_thread_open_pro.get(),&OpenTreeThread::SigFinishProgress,this,&ProTreeWidget::SlotFinishOpenProgress);

    connect(_open_progressdlg,&QProgressDialog::canceled,this,&ProTreeWidget::SlotCancelOpenProgress);

    connect(this,&ProTreeWidget::SigCancelOpenProgress,_thread_open_pro.get(),&OpenTreeThread::SlotCancelProgress);
    _thread_open_pro->start();

    _open_progressdlg->setWindowTitle("Please wait...");
    _open_progressdlg->setFixedWidth(PROGRESS_WIDTH);
    _open_progressdlg->setRange(0,PROGRESS_MAX);
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
