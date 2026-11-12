#include "protreethread.h"
#include <QDir>
#include "const.h"
#include "protreeitem.h"

ProTreeThread::ProTreeThread(const QString &src_path, const QString &dist_path,QObject *parent)
    :QThread(parent),_src_path(src_path),_dist_path(dist_path)
{
    qRegisterMetaType<QVector<ImportEntry>>("QVector<ImportEntry>");
}

void ProTreeThread::CreateProTree(const QString &src_path, const QString &dist_path,const QString &relativeParentPath)
{
    if(isInterruptionRequested()){
        return;
    }
    QDir sourceDir(src_path);
    sourceDir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    sourceDir.setSorting(QDir::Name | QDir::DirsFirst);

    const QFileInfoList entries = sourceDir.entryInfoList();
    for(const QFileInfo &fileInfo : entries){
        if(isInterruptionRequested()){
            return;
        }
        const QString name = fileInfo.fileName();
        QString relativePath = relativeParentPath.isEmpty()?name:QDir(relativeParentPath).filePath(name);
        relativePath = QDir::fromNativeSeparators(QDir::cleanPath(relativePath));

        const QString targetPath = QDir(dist_path).filePath(name);

        if(fileInfo.isDir()){
            const bool existedBefore = QDir(targetPath).exists();

            if(!QDir().mkpath(targetPath)){
                if(_firstError.isEmpty()){
                    _firstError = QString("Cannot create directory: %1").arg(targetPath);
                }
                continue;
            }
            if(!existedBefore){
                _createdDirs.append(targetPath);
            }

            _entries.append({relativePath,name,targetPath,TreeItemDir});

            emit SigUpdateProgress(++_file_count);

            CreateProTree(fileInfo.absoluteFilePath(),targetPath,relativePath);
            continue;
        }

        const QString suffix = fileInfo.suffix().toLower();
        if(suffix != "png" && suffix != "jpg" && suffix != "jpeg"){
            continue;
        }
        if(isInterruptionRequested()){
            return;
        }

        const QString sourcePath = fileInfo.absoluteFilePath();
        const QString absoluteTargetPath = QFileInfo(targetPath).absoluteFilePath();

        if(QFileInfo(sourcePath).absoluteFilePath() != absoluteTargetPath){
            if(!QFile::copy(sourcePath,absoluteTargetPath)){
                if(_firstError.isEmpty()){
                    _firstError = QString("Cannot copy file: %1").arg(sourcePath);
                }
                continue;
            }
            _createdFiles.append(absoluteTargetPath);
        }

        _entries.append({relativePath,name,absoluteTargetPath,TreeItemPic});
        emit SigUpdateProgress(++_file_count);
    }
}

void ProTreeThread::RollbackCreatedFiles()
{
    for(auto iter = _createdFiles.crbegin();iter != _createdFiles.crend();++iter){
        QFile::remove(*iter);
    }

    for(auto iter = _createdDirs.crbegin();iter != _createdDirs.crend();++iter){
        QDir().rmdir(*iter);
    }
}

void ProTreeThread::run()
{
    _entries.clear();
    _createdFiles.clear();
    _createdDirs.clear();
    _firstError.clear();
    _file_count = 0;

    CreateProTree(_src_path,_dist_path,QString());
    if(isInterruptionRequested()){
        RollbackCreatedFiles();
        emit SigImportCanceled();
        return;
    }
    if(!_firstError.isEmpty()){
        RollbackCreatedFiles();
        emit SigImportFailed(_firstError);
        return;
    }

    emit SigImportReady(_entries);
}

void ProTreeThread::SlotCancelProgress()
{
    requestInterruption();
}
