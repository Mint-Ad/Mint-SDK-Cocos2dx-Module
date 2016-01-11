#include "InlineScene.h"

// IMobileCocos2dxModule.hの読み込み
#include "IMobileCocos2dxModule.h"

USING_NS_CC;

int adViewID;

typedef struct{
    const char *sidList[12];
    int adViewIdList[12];
    int adCount;
}AdList;

// 今まで表示された広告リスト
AdList adlist;

// 前のシーンより パブリッシャーID メディアID　スポットID　を受信します。
Scene* Inline::createScene(IMobileCocos2dxModule::AdType adtype,const char *pid,const char *mid,const char *sid)
{
    auto scene = Scene::create();
    auto layer = Inline::create();
    scene->addChild(layer);
    
    adViewID = 0;
    
    // 同じスポットIDがないかチェックする
    for (int i=0; i < 12; i++) {
        if (adlist.sidList[i] == sid) {
            // 見つかった場合、adViewIDを取得する
            adViewID = adlist.adViewIdList[i];
            break;
        }
    }
    
    //adViewIDが0ではない　＝　前に表示したことがある。
    if (adViewID == 0) {
        // 広告の表示位置を指定して表示
        IMobileCocos2dxModule::registerSpotInline(pid, mid, sid);
        adViewID = IMobileCocos2dxModule::show(sid, adtype, IMobileCocos2dxModule::AdAlignPosition::CENTER, IMobileCocos2dxModule::AdValignPosition::MIDDLE, false);
        
        //リストへ格納。
        adlist.adViewIdList[adlist.adCount] = adViewID;
        adlist.sidList[adlist.adCount] = sid;
        adlist.adCount++;
    }else{
        // 前回表示を行った際に発行されているadViewIDが存在した場合、表示をおこないます。
        IMobileCocos2dxModule::setVisibility(adViewID, true);
    }
    
    return scene;
}
bool Inline::init()
{
    // 画面サイズの取得
    Size winSize = Director::getInstance()->getVisibleSize();
    
    // 背景色の追加
    auto backcolor = LayerColor::create(Color4B(49,77,121,255), winSize.width, winSize.height);
    addChild(backcolor);
    
    // 戻るボタンのサイズ、配置場所の取得
    Size backBtnSize;
    Point backBtnPoint;
    // 戻るボタン
    auto backBtnLabel = Label::createWithSystemFont("戻る", "Arial", 20);
    backBtnLabel->setColor(Color3B::BLACK);
    backBtnSize = backBtnLabel->getContentSize();
    auto backBtnItem = MenuItemLabel::create(backBtnLabel, CC_CALLBACK_0(Inline::pushBackBtn, this));
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
void Inline::pushBackBtn(){
    // 表示を行った時のIDを使用し、広告を非表示にします。
    IMobileCocos2dxModule::setVisibility(adViewID, false);
    
    Director::getInstance()->popScene();
}
