#ifndef CONST_H
#define CONST_H

#endif // CONST_H

enum TreeItemType{
    TreeItemPro = 1,  //条目
    TreeItemDir = 2,  //文件夹
    TreeItemPic = 3,  //图片
};

enum PicBtnState{
    PicBtnStateNormal = 1,
    PicBtnStateHover = 2,
    PicBtnStatePress = 3,
    PicBtnStateNormal_Pause = 4,
    PicBtnStateHover_Pause = 5,
    PicBtnStatePress_Pause = 6,
};

const int PROGRESS_WIDTH = 300;
const int PROGRESS_MAX = 300;
const int PREITEM_SIZE = 100;
const int PREICON_SIZE = 90;
