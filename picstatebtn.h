#ifndef PICSTATEBTN_H
#define PICSTATEBTN_H

#include <QPushButton>

class PicStateBtn : public QPushButton
{
    Q_OBJECT
public:
    PicStateBtn(QWidget *parent = nullptr);
    void setIcons(const QString& normal,const QString& hover,const QString press,
                  const QString& normal_pause,const QString& hover_pause,const QString& press_pause);
    virtual ~PicStateBtn(){}
protected:
    bool event(QEvent* event)override;
private:
    void setNormalIcon();
    void setHoverIcon();
    void setPressIcon();

    void setPauseNormalIcon();
    void setPauseHoverIcon();
    void setPausePressIcon();

    QString _normal;
    QString _hover;
    QString _press;
    QString _normal_pause;
    QString _hover_pause;
    QString _press_pause;
    int _cur_state;
public slots:
    void SlotStart();
    void SlotStop();
};

#endif // PICSTATEBTN_H
