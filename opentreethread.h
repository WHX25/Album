#ifndef OPENTREETHREAD_H
#define OPENTREETHREAD_H

#include "const.h"
#include <QThread>
#include <QVector>
#include <QTreeWidget>

class OpenTreeThread : public QThread
{
    Q_OBJECT
public:
    explicit OpenTreeThread(const QString& src_path,QObject *parent = nullptr);
    //void OpenProTree(const QString& src_path,int &file_count,QTreeWidget* self);
protected:
    virtual void run();
private:
    void RecursiveProTree(const QString& src_path,const QString &relative_parent_path);
    QString _src_path;
    QVector<ImportEntry> _entries;
    int _file_count = 0;
signals:
    void SigOpenReady(QVector<ImportEntry>);
    void SigUpdateProgress(int);
    void SigOpenCanceled();
public slots:
    void SlotCancelProgress();
};

#endif // OPENTREETHREAD_H
