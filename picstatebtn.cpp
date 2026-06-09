#include "picstatebtn.h"
#include "const.h"
#include <QEvent>

PicStateBtn::PicStateBtn(QWidget *parent)
{

}

void PicStateBtn::setIcons(const QString &normal, const QString &hover, const QString press, const QString &normal_pause, const QString &hover_pause, const QString &press_pause)
{
    _normal = normal;
    _hover = hover;
    _press = press;

    _normal_pause = normal_pause;
    _hover_pause = hover_pause;
    _press_pause = press_pause;

    QPixmap tmpPixmap;
    tmpPixmap.load(normal);
    this->resize(tmpPixmap.size());
    this->setIcon(tmpPixmap);
    this->setIconSize(tmpPixmap.size());
    _cur_state = PicBtnStateNormal;
}

bool PicStateBtn::event(QEvent *event)
{
    switch(event->type()){
        case QEvent::Enter:
            if(_cur_state < PicBtnStateNormal_Pause){
                setHoverIcon();
            }else{
                setPauseHoverIcon();
            }
            break;
        case QEvent::Leave:
            if(_cur_state < PicBtnStateNormal_Pause){
                setNormalIcon();
            }else{
                setPauseNormalIcon();
            }
            break;
        case QEvent::MouseButtonPress:
            if(_cur_state < PicBtnStateNormal_Pause){
                setPressIcon();
            }else{
                setPausePressIcon();
            }
            break;
        case QEvent::MouseButtonRelease:
            if(_cur_state < PicBtnStateNormal_Pause){
                setPauseHoverIcon();
            }else{
                setHoverIcon();
            }
            break;
        default:
            break;
    }
        return QPushButton::event(event);
}

void PicStateBtn::setNormalIcon()
{
    QPixmap tmpPixmap;
    tmpPixmap.load(_normal);
    this->setIcon(tmpPixmap);
    _cur_state = PicBtnStateNormal;
}

void PicStateBtn::setHoverIcon()
{
    QPixmap tmpPixmap;
    tmpPixmap.load(_hover);
    this->setIcon(tmpPixmap);
    _cur_state = PicBtnStateHover;
}

void PicStateBtn::setPressIcon()
{
    QPixmap tmpPixmap;
    tmpPixmap.load(_press);
    this->setIcon(tmpPixmap);
    _cur_state = PicBtnStatePress;
}

void PicStateBtn::setPauseNormalIcon()
{
    QPixmap tmpPixmap;
    tmpPixmap.load(_normal_pause);
    this->setIcon(tmpPixmap);
    _cur_state = PicBtnStateNormal_Pause;
}

void PicStateBtn::setPauseHoverIcon()
{
    QPixmap tmpPixmap;
    tmpPixmap.load(_hover_pause);
    this->setIcon(tmpPixmap);
    _cur_state = PicBtnStateHover_Pause;
}

void PicStateBtn::setPausePressIcon()
{
    QPixmap tmpPixmap;
    tmpPixmap.load(_press_pause);
    this->setIcon(tmpPixmap);
    _cur_state = PicBtnStatePress_Pause;
}

void PicStateBtn::SlotStart()
{
    setPauseNormalIcon();
}

void PicStateBtn::SlotStop()
{
    setNormalIcon();
}
