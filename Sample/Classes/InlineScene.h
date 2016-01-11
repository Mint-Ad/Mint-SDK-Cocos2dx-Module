#ifndef __INLINE_SCENE_H__
#define __INLINE_SCENE_H__

#include <iostream>
#include "cocos2d.h"
#include <extensions/cocos-ext.h>

#include "IMobileCocos2dxModule.h"

USING_NS_CC;

using namespace extension;

class Inline : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene(IMobileCocos2dxModule::AdType type,const char *passpid,const char *passmid,const char *passsid);
    
    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    void pushBackBtn();
    
    // implement the "static create()" method manually
    CREATE_FUNC(Inline);
    
};

#endif // __INLINE_SCENE_H__
