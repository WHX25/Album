#ifndef PROTREETHREAD_H
#define PROTREETHREAD_H
#include "const.h"
#include <QThread>
#include <QTreeWidget>
#include <QTreeWidgetItem>

class ProTreeThread:public QThread
{
    Q_OBJECT
public:
    ProTreeThread(const QString &src_path, const QString &dist_path,QObject *parent = nullptr);
private:
    void CreateProTree(const QString &src_path, const QString &dist_path,const QString &relativeParentPath);
    void RollbackCreatedFiles();

    QString _src_path;
    QString _dist_path;
    int _file_count = 0;
    // QTreeWidgetItem * _parent_item;
    // QTreeWidget* _self;
    // QTreeWidgetItem* _root;
    // bool _bstop;
    QVector<ImportEntry> _entries;
    QStringList _createdFiles;
    QStringList _createdDirs;
    QString _firstError;
protected:
    virtual void run();
signals:
    void SigUpdateProgress(int);

    void SigImportReady(QVector<ImportEntry>);
    void SigImportFailed(QString);
    void SigImportCanceled();
    // void SigFinishProgress(int);
    // void SigUpdateUi();
public slots:
    void SlotCancelProgress();
};

#endif // PROTREETHREAD_H
