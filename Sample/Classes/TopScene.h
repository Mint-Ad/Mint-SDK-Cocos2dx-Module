#ifndef __TOP_SCENE_H__
#define __TOP_SCENE_H__

#include <iostream>
#include "cocos2d.h"
#include <extensions/cocos-ext.h>

USING_NS_CC;

using namespace extension;

class TopScene : public cocos2d::Layer,public TableViewDelegate,public TableViewDataSource
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    //TableViewDataSourceの抽象メソッド
    virtual Size cellSizeForTable(TableView* table);
    virtual TableViewCell* tableCellAtIndex(TableView* table,ssize_t idx);
    virtual ssize_t numberOfCellsInTableView(TableView* table);
    virtual void tableCellTouched(TableView* table,TableViewCell* cell);
    virtual void scrollViewDidScroll(ScrollView* view){};
    virtual void scrollViewDidZoom(ScrollView* view){};
    
    //SP広告ボタンメソッド
    void pushspbtn();
    //タブレット広告ボタンメソッド
    void pushtabbtn();

    // implement the "static create()" method manually
    CREATE_FUNC(TopScene);
};

#endif // __TOP_SCENE_H__
