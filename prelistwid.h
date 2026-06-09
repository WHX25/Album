#ifndef PRELISTWID_H
#define PRELISTWID_H

#include <QListWidget>
#include <QTreeWidgetItem>

class PreListWid : public QListWidget
{
    Q_OBJECT
public:
    PreListWid(QWidget *parent = nullptr);
    virtual ~PreListWid();
    void AddListItem(const QString& path);
private:
    QMap<QString,QListWidgetItem*> _set_items;
    int _global;
    QPoint _pos_origin;
    int _last_index;
public slots:
    void SlotUpPrelist(QTreeWidgetItem* tree_item);
    void SlotUpSelect(QTreeWidgetItem* tree_item);
    void SlotItemPressed(QListWidgetItem* item);
signals:
    void SigUpSelectShow(QString path);
};

#endif // PRELISTWID_H
