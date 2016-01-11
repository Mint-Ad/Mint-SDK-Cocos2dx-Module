#include "TopScene.h"

// 広告を表示するためのIDリスト
#include "ImobileSpotInfo.h"

// インライン広告
#include "InlineScene.h"

// アイコン広告
#include "IconScene.h"

// 全画面広告
#include "FullScreenScene.h"

// IMobileCocos2dxModule.hの読み込み
#include "IMobileCocos2dxModule.h"

// iOSのセルの高さ
#define CELL_HEIGHT_IOS 35
// アンドロイドのセルの高さ
#define CELL_HEIGHT_ANDROID 55

USING_NS_CC;

// 画面のサイズ
Size winSize;

// SP広告リスト
const char *spAdList[7];
// タブレット広告リスト
const char *tabAdList[9];

//cellの数
int cellNumber;

//SP広告ボタンの背景（押した時に背景色を変えるため）
Sprite* spBtnBack;
//タブレット広告ボタンの背景（御した時に背景色を変えるため)
Sprite* tabBtnBack;

//広告リストを表示するためのTableView
TableView* adTableView;

//SP広告とタブレット広告の表示を仕分けるフラグ
//true : SP広告
//false : タブレット広告
bool spTabFlg = true;

Scene* TopScene::createScene()
{
	auto scene = Scene::create();
    auto layer = TopScene::create();
    scene->addChild(layer);
    return scene;
}

bool TopScene::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
    // SPの広告リスト
    if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) {
        cellNumber = 7;
        spAdList[0] = "バナー (320 × 50)";
        spAdList[1] = "ビックバナー (320 × 100)";
        spAdList[2] = "アイコン (57 × 57)";
        spAdList[3] = "Mレクタングル (300 × 250)";
        spAdList[4] = "インタースティシャル (全画面)";
        spAdList[5] = "ウォール (全画面)";
        spAdList[6] = "テキストポップアップ (全画面)";
    }else if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS){
        cellNumber = 5;
        spAdList[0] = "バナー (320 × 50)";
        spAdList[1] = "ビックバナー (320 × 100)";
        spAdList[2] = "Mレクタングル (300 × 250)";
        spAdList[3] = "インタースティシャル (全画面)";
        spAdList[4] = "テキストポップアップ (全画面)";
    }
    
    // タブレットの広告リスト
    tabAdList[0] = "タブレットバナー(468 × 60)";
    tabAdList[1] = "タブレットビックバナー(768 × 90)";
    tabAdList[2] = "スカイスクレイパー(120 × 600)";
    tabAdList[3] = "ワイドスカイスクレイパー(160 × 600)";
    tabAdList[4] = "スクエア(小)(200 × 200)";
    tabAdList[5] = "スクエア(250 × 250)";
    tabAdList[6] = "レクタングル(300 × 250)";
    tabAdList[7] = "レクタングル(大)(336 × 280)";
    tabAdList[8] = "ハーフページ(300 × 600)";
    
    // 画面サイズの取得
    winSize = Director::getInstance()->getVisibleSize();
    
    // 背景色の追加
    auto backgroundcolor = LayerColor::create(Color4B::WHITE, winSize.width, winSize.height);
    addChild(backgroundcolor);
    
    // cocosロゴの追加
    auto cocosLogo = Sprite::create("cocos2dxlogo.png");
    cocosLogo->setPosition(280,450);
    cocosLogo->setScale(0.15, 0.15);
    addChild(cocosLogo);
    
    // imobileロゴの追加
    auto imobileLogo = Sprite::create("imobilelogo.png");
    imobileLogo->setPosition(110,450);
    imobileLogo->setScale(0.15, 0.15);
    addChild(imobileLogo);
    
    // IOSとアンドロイドでリストの高さを変更する
    if(CC_TARGET_PLATFORM == CC_PLATFORM_IOS){
        adTableView = TableView::create(this, Size(winSize.width, sizeof(tabAdList) / sizeof(tabAdList[0]) * CELL_HEIGHT_IOS) );
        adTableView->setPosition(0,100);
    }else{
        adTableView = TableView::create(this, Size(winSize.width, sizeof(spAdList) / sizeof(spAdList[0]) * CELL_HEIGHT_ANDROID) );
        adTableView->setPosition(0,50);
    }
    adTableView->setDirection(TableView::Direction::VERTICAL);
    // 表示順序上からしたへ
    adTableView->setVerticalFillOrder(TableView::VerticalFillOrder::TOP_DOWN);
    adTableView->setDelegate(this);
    adTableView->setBounceable(false);
    addChild(adTableView);
    adTableView->reloadData();
    
    if(CC_TARGET_PLATFORM == CC_PLATFORM_IOS){
		// SP広告ボタン
		Size spBtnSize;
		Point spBtnPoint;
		auto spLabel = Label::createWithSystemFont("SP広告", "Arial", 20);
		spLabel->setColor(Color3B::BLACK);
		spBtnSize = spLabel->getContentSize();
		auto spBtnItem = MenuItemLabel::create(spLabel, CC_CALLBACK_0(TopScene::pushspbtn, this));
		Menu* spMenu = Menu::create(spBtnItem, NULL);
		spMenu->setPosition(70,50);
		spBtnPoint = spMenu->getPosition();
		// SP広告ボタンの背景
		spBtnBack = Sprite::create();
		spBtnBack->setPosition(spBtnPoint);
		spBtnBack->setTextureRect(Rect(0, 0, spBtnSize.width+60, spBtnSize.height+20));
		spBtnBack->setColor(Color3B(59,164,153));
		addChild(spBtnBack);
		addChild(spMenu);

		// タブレット広告ボタン
		Size tabBtnSize;
		Point tabBtnPoint;
		auto tabLabel = Label::createWithSystemFont("タブレット広告", "Arial", 20);
		tabLabel->setColor(Color3B::BLACK);
		tabBtnSize = tabLabel->getContentSize();
		auto tabBtnItem = MenuItemLabel::create(tabLabel, CC_CALLBACK_0(TopScene::pushtabbtn, this));
		Menu* tabMenu = Menu::create(tabBtnItem, NULL);
		tabMenu->setPosition(230,50);
		tabBtnPoint = tabMenu->getPosition();
		// タブレット広告ボタンの背景
		tabBtnBack = Sprite::create();
		tabBtnBack->setPosition(tabBtnPoint);
		tabBtnBack->setTextureRect(Rect(0, 0, tabBtnSize.width+20, tabBtnSize.height+20));
		tabBtnBack->setColor(Color3B(255,255,255));
		addChild(tabBtnBack);
		addChild(tabMenu);
    }
    return true;
}
// セルの大きさを設定する
Size TopScene::cellSizeForTable(TableView *table){
    //アンドロイドとiOSで高さの変更
    if(CC_TARGET_PLATFORM == CC_PLATFORM_IOS){
    	return Size(winSize.width, CELL_HEIGHT_IOS);
    }else{
    	return Size(winSize.width, CELL_HEIGHT_ANDROID);
    }
}

// cellの作成
TableViewCell* TopScene::tableCellAtIndex(TableView *table, ssize_t idx){
    TableViewCell *cell = table->dequeueCell();
    cell = new TableViewCell();
    cell->autorelease();
    
    // ボーダーライン
    Sprite* line = Sprite::create();
    line->setAnchorPoint(Point(0, 0));
    line->setTextureRect(Rect(0, 0, winSize.width, 1));
    line->setColor(Color3B(0,0,0));
    cell->addChild(line);
    
    // タイトルの文字列
    const char *title;
    if (spTabFlg) {
        title = spAdList[idx];
    }else{
        title = tabAdList[idx];
    }
    
    // タイトルラベル
    auto *adLabel = LabelTTF::create(title, "Arial", 16);
    adLabel->setAnchorPoint(Point(0, 0));
    adLabel->setPosition(Point(10, 10));
    adLabel->setColor(Color3B(0,0,0));
    cell->addChild(adLabel);
    
    // 矢印
    auto *arrayLabel = LabelTTF::create(">", "Arial", 16);
    arrayLabel->setAnchorPoint(Point(0, 0));
    arrayLabel->setPosition(Point(300, 10));
    arrayLabel->setColor(Color3B(0,0,0));
    cell->addChild(arrayLabel);
    
    return cell;
}

// セル数
ssize_t TopScene::numberOfCellsInTableView(TableView *table){
    if (spTabFlg) {
        return cellNumber;
    }else{
        return sizeof(tabAdList) / sizeof(tabAdList[0]);
    }
}
// SPボタン
void TopScene::pushspbtn(){
    if (!spTabFlg) {
        //フラグの変更
        spTabFlg = true;
        //背景色の入れ替え
        spBtnBack->setColor(Color3B(59,164,153));
        tabBtnBack->setColor(Color3B(255,255,255));
        //広告リストを再読み込み
        adTableView->reloadData();
    }
}
//タブレットボタン
void TopScene::pushtabbtn(){
    if (spTabFlg ) {
        //フラグの変更
        spTabFlg = false;
        //背景色の入れ替え
        spBtnBack->setColor(Color3B(255,255,255));
        tabBtnBack->setColor(Color3B(59,164,153));
        //広告リストを再読み込み
        adTableView->reloadData();
    }
}
// セルがタッチされた時のcallback
void TopScene::tableCellTouched(TableView* table, TableViewCell* cell){
    if (spTabFlg) {
        // SP広告
        if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) {
            switch (cell->getIdx()) {
                case 0:
                    Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::BANNER, banner_pid, banner_mid, banner_sid));
                    break;
                case 1:
                    Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::BIG_BANNER, bigbanner_pid, bigbanner_mid, bigbanner_sid));
                    break;
                case 2:
                    Director::getInstance()->pushScene(Icon::createScene(icon_pid, icon_mid, icon_sid));
                    break;
                case 3:
                    Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::MEDIUM_RECTANGLE, m_rectangle_pid, m_rectangle_mid, m_rectangle_sid));
                    break;
                case 4:
                    Director::getInstance()->pushScene(FullScreen::createScene(inter_pid, inter_mid, inter_sid));
                    break;
                case 5:
                    Director::getInstance()->pushScene(FullScreen::createScene(wall_pid, wall_mid, wall_sid));
                    break;
                case 6:
                    Director::getInstance()->pushScene(FullScreen::createScene(textpopup_pid, textpopup_mid, textpopup_sid));
                    break;
                default:
                    break;
            }
        }else if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS){
            switch (cell->getIdx()) {
                case 0:
                    Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::BANNER, banner_pid, banner_mid, banner_sid));
                    break;
                case 1:
                    Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::BIG_BANNER, bigbanner_pid, bigbanner_mid, bigbanner_sid));
                    break;
                case 2:
                    Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::MEDIUM_RECTANGLE, m_rectangle_pid, m_rectangle_mid, m_rectangle_sid));
                    break;
                case 3:
                    Director::getInstance()->pushScene(FullScreen::createScene(inter_pid, inter_mid, inter_sid));
                    break;
                case 4:
                    Director::getInstance()->pushScene(FullScreen::createScene(textpopup_pid, textpopup_mid, textpopup_sid));
                    break;
                default:
                    break;
            }
        }
    }else{
        // タブレット広告
        switch (cell->getIdx()) {
            case 0:
                Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::TABLET_BANNER, tabbanner_pid, tabbanner_mid, tabbanner_sid));
                break;
            case 1:
                Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::TABLET_BIG_BANNER, tabbigbanner_pid, tabbigbanner_mid, tabbigbanner_sid));
                break;
            case 2:
                Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::SKYSCRAPER, skyscraper_pid, skyscraper_mid, skyscraper_sid));
                break;
            case 3:
                Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::WIDE_SKYSCRAPER, wideskyscraper_pid, wideskyscraper_mid, wideskyscraper_sid));
                break;
            case 4:
                Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::SMALL_SQUARE, smallsquare_pid, smallsquare_mid, smallsquare_sid));
                break;
            case 5:
                Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::SQUARE, square_pid, square_mid, square_sid));
                break;
            case 6:
                Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::MEDIUM_RECTANGLE, m_rectangle_pid, m_rectangle_mid, m_rectangle_sid));
                break;
            case 7:
                Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::BIG_RECTANGLE, big_rectangle_pid, big_rectangle_mid, big_rectangle_sid));
                break;
            case 8:
                Director::getInstance()->pushScene(Inline::createScene(IMobileCocos2dxModule::HALFPAGE, halfpage_pid, halfpage_mid, halfpage_sid));
                break;
            default:
                break;
        }
    }
}
