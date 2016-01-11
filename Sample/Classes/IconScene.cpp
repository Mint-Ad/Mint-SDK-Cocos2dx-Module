#include "IconScene.h"

// IMobileCocos2dxModule.hの読み込み
#include "IMobileCocos2dxModule.h"

USING_NS_CC;

// 広告を非表示にするためのID
int adViewID1;
int adViewID4;
int adViewID6;

// 前のシーンより パブリッシャーID メディアID　スポットID　を受信します。
Scene* Icon::createScene(const char *pid,const char *mid,const char *sid)
{
    
    auto scene = Scene::create();
    auto layer = Icon::create();
    scene->addChild(layer);
    
    // デバイスの論理サイズを取得
    cocos2d::Size screenSize = IMobileCocos2dxModule::getDisplaySize();
    
    IMobileCocos2dxModuleIconParams icon1;
    // アイコンの個数を１個に指定
    icon1.iconNumber = 1;
    
    IMobileCocos2dxModuleIconParams icon6;
    // アイコンの個数を６個に指定
    icon6.iconNumber = 6;
    // タイトルを非表示に指定
    icon6.iconTitleEnable = false;
    
    IMobileCocos2dxModule::registerSpotInline(pid, mid, sid);
    if (adViewID1 == 0) {
        // アイコン1個を中心より少し上に表示
        adViewID1 = IMobileCocos2dxModule::show(sid, IMobileCocos2dxModule::AdType::ICON, screenSize.width / 2 - 40 , screenSize.height / 2 - 120, icon1);
    }else{
        IMobileCocos2dxModule::setVisibility(adViewID1, true);
    }
    
    if (adViewID4 == 0) {
        // アイコン4個を中心に表示
        adViewID4 = IMobileCocos2dxModule::show(sid, IMobileCocos2dxModule::AdType::ICON, IMobileCocos2dxModule::AdAlignPosition::CENTER, IMobileCocos2dxModule::AdValignPosition::MIDDLE);
    }else{
        IMobileCocos2dxModule::setVisibility(adViewID4, true);
    }
    
    if (adViewID6 == 0) {
        // アイコン6個を中心より少し下に表示
        adViewID6 = IMobileCocos2dxModule::show(sid, IMobileCocos2dxModule::AdType::ICON, 0 , screenSize.height / 2 + 50, icon6);
    }else{
        IMobileCocos2dxModule::setVisibility(adViewID6, true);
    }
    
    return scene;
}
bool Icon::init()
{
    // 画面サイズの取得
    Size winSize = Director::getInstance()->getVisibleSize();
    
    // 背景色の追加
    auto backColor = LayerColor::create(Color4B(49,77,121,255), winSize.width, winSize.height);
    addChild(backColor);
    
    // 戻るボタンのサイズ、配置場所の取得
    Size backBtnSize;
    Point backBtnPoint;
    // 戻るボタン
    auto backBtnLabel = Label::createWithSystemFont("戻る", "Arial", 20);
    backBtnLabel->setColor(Color3B::BLACK);
    backBtnSize = backBtnLabel->getContentSize();
    auto backBtnItem = MenuItemLabel::create(backBtnLabel, CC_CALLBACK_0(Icon::pushBackBtn, this));
    Menu* backMenu = Menu::create(backBtnItem, NULL);
    backMenu->setPosition(winSize.width / 2,50);
    backBtnPoint = backMenu->getPosition();
    // 戻るボタンの背景
    Sprite *backBtnBack = Sprite::create();
    backBtnBack->setPosition(backBtnPoint);
    backBtnBack->setTextureRect(Rect(0, 0, backBtnSize.width+60, backBtnSize.height+20));
    backBtnBack->setColor(Color3B(231,232,233));
    addChild(backBtnBack);
    addChild(backMenu);
    
    return true;
}

// 前のシーンへ戻る
void Icon::pushBackBtn(){
    // 表示の際にIDを取得し、そのIDで広告を非表示にします。
    IMobileCocos2dxModule::setVisibility(adViewID1, false);
    IMobileCocos2dxModule::setVisibility(adViewID4, false);
    IMobileCocos2dxModule::setVisibility(adViewID6, false);
    Director::getInstance()->popScene();
}
