#include "FullScreenScene.h"

// IMobileCocos2dxModule.hの読み込み
#include "IMobileCocos2dxModule.h"

USING_NS_CC;

const char *showpid;
const char *showmid;
const char *showsid;

// 前のシーンより パブリッシャーID メディアID　スポットID　を受信します。
Scene* FullScreen::createScene(const char *pid,const char *mid,const char *sid)
{
    // 前のシーンより取得したメディアIDをメンバー変数にセットしておきます（任意の表示するため）
    showpid = pid;
    showmid = mid;
    showsid = sid;
    
    auto scene = Scene::create();
    auto layer = FullScreen::create();
    scene->addChild(layer);

    // 広告スポットの登録
    IMobileCocos2dxModule::registerSpotFullScreen(pid, mid, sid);
    return scene;
}
bool FullScreen::init()
{
    // 画面サイズの取得
    Size winSize = Director::getInstance()->getVisibleSize();
    
    // 背景色の追加
    auto backColor = LayerColor::create(Color4B(49,77,121,255), winSize.width, winSize.height);
    addChild(backColor);
    
    // 広告表示ボタンのサイズ、配置場所
    Size showBtnSize;
    Point showBtnPoint;
    // 広告表示ボタン
    auto showBtnLabel = Label::createWithSystemFont("広告表示", "Arial", 20);
    showBtnLabel->setColor(Color3B::BLACK);
    showBtnSize = showBtnLabel->getContentSize();
    auto showBtnItem = MenuItemLabel::create(showBtnLabel, CC_CALLBACK_0(FullScreen::pushShowBtn, this));
    Menu* showMenu = Menu::create(showBtnItem, NULL);
    showMenu->setPosition(winSize.width / 2,winSize.height / 2);
    showBtnPoint = showMenu->getPosition();
    // 広告表示ボタンの背景
    Sprite *showBtnBack = Sprite::create();
    showBtnBack->setPosition(showBtnPoint);
    showBtnBack->setTextureRect(Rect(0, 0, showBtnSize.width+60, showBtnSize.height+20));
    showBtnBack->setColor(Color3B(231,232,233));
    addChild(showBtnBack);
    addChild(showMenu);
    
    // 戻るボタンのサイズ、配置場所
    Size backBtnSize;
    Point backBtnPoint;
    // 戻るボタン
    auto backBtnLabel = Label::createWithSystemFont("戻る", "Arial", 20);
    backBtnLabel->setColor(Color3B::BLACK);
    backBtnSize = backBtnLabel->getContentSize();
    auto backBtnItem = MenuItemLabel::create(backBtnLabel, CC_CALLBACK_0(FullScreen::pushBackBtn, this));
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
// 広告表示ボタン
void FullScreen::pushShowBtn(){
    IMobileCocos2dxModule::show(showsid);
}

// 前のシーンへ戻る
void FullScreen::pushBackBtn(){
    Director::getInstance()->popScene();
}
