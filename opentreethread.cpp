#include "opentreethread.h"
#include "protreeitem.h"
#include "const.h"
#include <QDir>

OpenTreeThread::OpenTreeThread(const QString &src_path,QObject *parent)
    :QThread(parent),_src_path(src_path)
{
    qRegisterMetaType<QVector<ImportEntry>>("QVector<ImportEntry>");
}

void OpenTreeThread::run()
{
    _entries.clear();
    _file_count = 0;
    RecursiveProTree(_src_path,QString());

    if(isInterruptionRequested()){
        emit SigOpenCanceled();
        return;
    }

    emit SigOpenReady(_entries);
}

void OpenTreeThread::RecursiveProTree(const QString &src_path, const QString &relative_parent_path)
{
    if(isInterruptionRequested()){
        return;
    }

    QDir src_dir(src_path);
    src_dir.setFilter(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot);
    src_dir.setSorting(QDir::Name | QDir::DirsFirst);
    const QFileInfoList entries = src_dir.entryInfoList();
    for(const QFileInfo &fileInfo : entries){
        if(isInterruptionRequested()){
            return;
        }
        const QString name = fileInfo.fileName();
        QString relativePath = relative_parent_path.isEmpty()?name:QDir(relative_parent_path).filePath(name);
        relativePath = QDir::fromNativeSeparators(QDir::cleanPath(relativePath));
        const QString targetPath = fileInfo.absoluteFilePath();

        if(fileInfo.isDir()){
            _entries.append({relativePath,name,targetPath,TreeItemDir});
            emit SigUpdateProgress(++_file_count);

            RecursiveProTree(targetPath,relativePath);
            continue;
        }

        const QString suffix = fileInfo.suffix().toLower();
        if(suffix != "png" && suffix != "jpg" && suffix != "jpeg"){
            continue;
        }
        _entries.append({relativePath,name,targetPath,TreeItemPic});
        emit SigUpdateProgress(++_file_count);
    }
}

void OpenTreeThread::SlotCancelProgress()
{
    requestInterruption();
}
