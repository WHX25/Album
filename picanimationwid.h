#ifndef PICANIMATIONWID_H
#define PICANIMATIONWID_H

#include <QWidget>
#include <QTimer>
#include <QTreeWidgetItem>
#include <protreeitem.h>

class PicAnimationWid : public QWidget
{
    Q_OBJECT
public:
    explicit PicAnimationWid(QWidget *parent = nullptr);
    ~PicAnimationWid();
    void setPixmap(QTreeWidgetItem* item);
    void start();
    void Stop();
    void SlideNext();
    void SlidePre();
    void UpSelectPixmap(QTreeWidgetItem* item);
protected:
    virtual void paintEvent(QPaintEvent *event);
private:
    float _factor;
    QTimer* _timer;
    QPixmap _pixmap1;
    QPixmap _pixmap2;
    QTreeWidgetItem* _cur_item;
    QMap<QString,QTreeWidgetItem*> _map_items;
    bool _b_start;
signals:
    void SigUpPreList(QTreeWidgetItem*);
    void SigSelectItem(QTreeWidgetItem*);
    void SigStart();
    void SigStop();
    void SigStartMusic();
    void SigStopMusic();
private slots:
    void TimeOut();
public slots:
    void SlotUpSelectShow(QString path);
    void SlotStartOrStop();
};

#endif // PICANIMATIONWID_H
