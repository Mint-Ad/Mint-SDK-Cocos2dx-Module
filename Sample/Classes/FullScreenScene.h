#ifndef __ALLAD_SCENE_H__
#define __ALLAD_SCENE_H__

#include <iostream>
#include "cocos2d.h"
#include <extensions/cocos-ext.h>

USING_NS_CC;

using namespace extension;

class FullScreen : public cocos2d::Layer
{
public:
    //前のシーンより パブリッシャーID メディアID　スポットID　を受信します。
    static cocos2d::Scene* createScene(const char *passpid,const char *passmid,const char *passsid);
    virtual bool init();
    // 広告表示ボタン
    void pushShowBtn();
    // 戻るボタン
    void pushBackBtn();
    
    CREATE_FUNC(FullScreen);
    
};

#endif // __ALLAD_SCENE_H__
