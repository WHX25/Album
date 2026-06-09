#ifndef PROTREEWIDGET_H
#define PROTREEWIDGET_H

#include <QTreeWidget>
#include <QProgressDialog>
#include "protreethread.h"
#include "opentreethread.h"
#include "slideshowdlg.h"
#include <QtMultimedia/QMediaPlayer>
//#include <QtMultimedia/QMediaPlayerList>
#include <QAudioOutput>

class SlideShowDlg;
class ProTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    ProTreeWidget(QWidget *parent = nullptr);
    void AddProToTree(const QString& name,const QString& path);
private:
    QSet<QString> _set_path;
    QAction * _action_import;
    QAction * _action_setstart;
    QAction * _action_closepro;
    QAction * _action_slideshow;
    //右键单击某个item之后弹出菜单
    QTreeWidgetItem * _right_btn_item;
    QTreeWidgetItem * _active_item;
    QTreeWidgetItem * _selected_item;
    QProgressDialog * _dialog_progress;
    QProgressDialog * _open_progressdlg;
    std::shared_ptr<ProTreeThread> _thread_create_pro;
    std::shared_ptr<OpenTreeThread> _thread_open_pro;
    std::shared_ptr<SlideShowDlg> _slide_show_dlg;

    QMediaPlayer * _player;
    QStringList _playlist;
    int _current_index;  //放到第几个
    bool isempty_list;  //容器里面是否为空
    QAudioOutput *audio_output=nullptr;
    QString GetMusicUrl();
private slots:
    void SlotItemPressed(QTreeWidgetItem *pressedItem, int column);
    void SlotDoubleClickItem(QTreeWidgetItem *item, int column);
    void SlotImport();
    void SlotSetActive();
    void SlotClosePro();
    void SlotUpdateProgress(int count);
    void SlotCancelProgress();
    void SlotFinishProgress();

    void SlotUpOpenProgress(int count);
    void SlotCancelOpenProgress();
    void SlotFinishOpenProgress();
    void SlotSlideShow();
public slots:
    void SlotOpenPro(const QString& path);
    void SlotNextShow();
    void SlotPreShow();
    void SlotSetMusic();
    void SlotStartMusic();
    void SlotStopMusic();
signals:
    void SigCancelProgress();
    void SigCancelOpenProgress();
    void SigUpdateSelected(const QString&);
    void SigUpdatePic(const QString&);
    void SigClearSelected();
};

#endif // PROTREEWIDGET_H
