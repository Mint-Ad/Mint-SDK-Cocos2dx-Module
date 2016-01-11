//
//  IMobileCocos2dxModule.mm
//
//  Copyright (c) 2014 i-mobile. All rights reserved.
//

#include "IMobileCocos2dxModule.h"
#include "AppController.h"

#import "RootViewController.h"
#import "EAGLView.h"

#import "ImobileSdkAds/ImobileSdkAds.h"

// スポットデータ
@interface IMobileCocos2dxModuleSpotData  : NSObject
    // パブリッシャーID
    @property (nonatomic,copy) NSString *publisherId;
    // メディアID
    @property (nonatomic,copy) NSString *mediaId;
@end

@implementation IMobileCocos2dxModuleSpotData
@end


#pragma mark - 内部メソッド(プロトタイプ宣言)
//  char* から NSString に変換します
static NSString* charToNSString(const char *value);
// 広告の表示領域を返します(表示位置パラメータ指定)
static CGRect getAdRect(IMobileCocos2dxModule::AdAlignPosition adAlignPosition, IMobileCocos2dxModule::AdValignPosition adValignPosition, IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModuleIconParams iconParams, bool sizeAdjust);
// 広告サイズを取得します
static CGSize getAdSize(IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModuleIconParams iconParams, bool sizeAdjust);
// サイズ調整後の広告サイズを取得します
static CGSize getAdjustedAdSize(CGSize originalSize, bool sizeAdjust, IMobileCocos2dxModule::AdType adType);
// 登録オブザーバ名の取得
static const char* getObserverEntryName(IMobileCocos2dxModule::AdNotficationType adNotficationType, NSString *spotId);
// 通常の通知を処理します
static void normalNotification(IMobileCocos2dxModule::AdNotficationType adNotficationType, NSString *spotId);
// 異常系の通知を処理します
static void abnormalNotification(IMobileCocos2dxModule::AdNotficationType adNotficationType, NSString *spotId, NSString *reason);
// showメソッドの実装
static int showImpl(const char* spotId, IMobileCocos2dxModule::AdType adType, float x, float y, IMobileCocos2dxModuleIconParams iconParams, bool sizeAdjust);

#pragma mark - 広告状態通知デリゲート
// 広告状態取得用のデリゲート
@interface IMobileCocos2dxModuleDelegate : NSObject<IMobileSdkAdsDelegate>
@end

@implementation IMobileCocos2dxModuleDelegate
//広告の表示が準備完了した際に呼ばれます
- (void)imobileSdkAdsSpot:(NSString *)spotId didReadyWithValue:(ImobileSdkAdsReadyResult)value {
    normalNotification(IMobileCocos2dxModule::DID_READY, spotId);
}

//広告の表示要求があった際に、準備が完了していない場合に呼ばれます
- (void)imobileSdkAdsSpotIsNotReady:(NSString *)spotId {
    normalNotification(IMobileCocos2dxModule::DID_READY, spotId);
}

//広告クリックした際に呼ばれます
- (void)imobileSdkAdsSpotDidClick:(NSString *)spotId {
    normalNotification(IMobileCocos2dxModule::DID_CLICK, spotId);
}
//広告を閉じた際に呼ばれます(広告の表示がスキップされた場合も呼ばれます)
- (void)imobileSdkAdsSpotDidClose:(NSString *)spotId {
    normalNotification(IMobileCocos2dxModule::DID_CLOSE, spotId);
}

//広告が表示された際に呼ばれます
- (void)imobileSdkAdsSpotDidShow:(NSString *)spotId {
    normalNotification(IMobileCocos2dxModule::DID_SHOW, spotId);
}


//広告の取得を失敗した際に呼ばれます
- (void)imobileSdkAdsSpot:(NSString *)spotId didFailWithValue:(ImobileSdkAdsFailResult)value {
    NSString *reason;
    switch (value) {
        case IMOBILESDKADS_ERROR_PARAM:
            reason = @"PARAM";
            break;
        case IMOBILESDKADS_ERROR_AUTHORITY:
            reason = @"AUTHORITY";
            break;
        case IMOBILESDKADS_ERROR_RESPONSE:
            reason = @"RESPONSE";
            break;
        case IMOBILESDKADS_ERROR_NETWORK_NOT_READY:
            reason = @"NETWORK_NOT_READY";
            break;
        case IMOBILESDKADS_ERROR_NETWORK:
            reason = @"NETWORK";
            break;
        case IMOBILESDKADS_ERROR_UNKNOWN:
            reason = @"UNKNOWN";
            break;
        case IMOBILESDKADS_ERROR_AD_NOT_READY:
            reason = @"AD_NOT_READY";
            break;
        case IMOBILESDKADS_ERROR_NOT_FOUND:
            reason = @"NOT_DELIVERY_AD";
            break;
        case IMOBILESDKADS_ERROR_SHOW_TIMEOUT:
            reason = @"TIMEOUT";
            break;
        default:
            reason = @"UNKNOWN";
            break;
    }
    
    abnormalNotification(IMobileCocos2dxModule::DID_FAIL, spotId, reason);
}

@end

#pragma mark - クラス変数
// adViewId管理用
static int adViewIdCounter = 100000;
static NSMutableDictionary *adViewIdDictionary = [NSMutableDictionary dictionary];
// スポット情報管理(インラインの場合のみ使用)
static NSMutableDictionary *spots = [NSMutableDictionary dictionary];

IMobileCocos2dxModule::IMobileCocos2dxModule(){
}

#pragma mark - 公開メソッド
// スポット登録(全画面)を行います
void IMobileCocos2dxModule::registerSpotFullScreen(const char *publisherId, const char *mediaId, const char *spotId) {
    // スポットの登録
    [ImobileSdkAds registerWithPublisherID:charToNSString(publisherId) MediaID:charToNSString(mediaId) SpotID:charToNSString(spotId)];
    // デリゲート設定
    [ImobileSdkAds setSpotDelegate:charToNSString(spotId) delegate:(id<IMobileSdkAdsDelegate>)[[IMobileCocos2dxModuleDelegate alloc] init]];
    // 広告取得開始
    [ImobileSdkAds startBySpotID:charToNSString(spotId)];
}

// スポット登録(インライン)を行います
void IMobileCocos2dxModule::registerSpotInline(const char *publisherId, const char *mediaId, const char *spotId) {
    IMobileCocos2dxModuleSpotData *spotData = [[IMobileCocos2dxModuleSpotData alloc] init];
    spotData.publisherId = charToNSString(publisherId);
    spotData.mediaId = charToNSString(mediaId);
    
    [spots setObject:spotData forKey:charToNSString(spotId)];
}

// 全画面広告表示を行います
void IMobileCocos2dxModule::show(const char *spotId) {
    [ImobileSdkAds showBySpotID:charToNSString(spotId)];
}

// インライン広告表示(座標指定)を行います
int IMobileCocos2dxModule::show(const char *spotId, IMobileCocos2dxModule::AdType adType, float x, float y) {
    IMobileCocos2dxModuleIconParams iconParams;
    return showImpl(spotId, adType, x, y, iconParams, false);
}
//　インライン広告表示（座標指定、表示サイズ自動調節）を行います
int IMobileCocos2dxModule::show(const char* spotId, IMobileCocos2dxModule::AdType adType, float x, float y , bool sizeAdjust){
    IMobileCocos2dxModuleIconParams iconParams;
    return showImpl(spotId, adType, x, y, iconParams , sizeAdjust);
}

// インライン広告表示(表示位置パラメータ指定)を行います
int IMobileCocos2dxModule::show(const char *spotId, IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModule::AdAlignPosition adAlignPosition, IMobileCocos2dxModule::AdValignPosition adValignPosition) {
    IMobileCocos2dxModuleIconParams iconParams;
    // 表示位置パラメータを座標に変換してから呼び出し
    CGRect adRect = getAdRect(adAlignPosition, adValignPosition, adType, iconParams, false);
    return showImpl(spotId, adType, adRect.origin.x, adRect.origin.y, iconParams, false);
}

// インライン広告表示(表示位置パラメータ指定、表示サイズ自動調節)を行います
int IMobileCocos2dxModule::show(const char* spotId, IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModule::AdAlignPosition adAlignPosition, IMobileCocos2dxModule::AdValignPosition adValignPosition, bool sizeAdjust){
    IMobileCocos2dxModuleIconParams iconParams;
    // 表示位置パラメータを座標に変換してから呼び出し
    CGRect adRect = getAdRect(adAlignPosition, adValignPosition, adType, iconParams, sizeAdjust);
    return showImpl(spotId, adType, adRect.origin.x, adRect.origin.y, iconParams, sizeAdjust);
}

// インライン広告表示(表示位置パラメータ指定、アイコン表示パラメータ指定)を行います
int IMobileCocos2dxModule::show(const char *spotId, IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModule::AdAlignPosition adAlignPosition, IMobileCocos2dxModule::AdValignPosition adValignPosition, IMobileCocos2dxModuleIconParams iconParams) {
    // 表示位置パラメータを座標に変換してから呼び出し
    CGRect adRect = getAdRect(adAlignPosition, adValignPosition, adType, iconParams, false);
    return showImpl(spotId, adType, adRect.origin.x, adRect.origin.y, iconParams, false);
}

// インライン広告表示(座標指定、アイコン表示パラメータ指定)を行います
int IMobileCocos2dxModule::show(const char *spotId, IMobileCocos2dxModule::AdType adType, float x, float y, IMobileCocos2dxModuleIconParams iconParams) {
    return showImpl(spotId, adType, x, y, iconParams,false);
}

// showメソッドの実装
static int showImpl(const char *spotId, IMobileCocos2dxModule::AdType adType, float x, float y, IMobileCocos2dxModuleIconParams iconParams, bool sizeAdjust) {
    
    CGSize adSize = getAdSize(adType,iconParams,sizeAdjust);
    UIView *adContainerView = [[UIView alloc] initWithFrame:CGRectMake(x, y, adSize.width, adSize.height)];
    UIWindow *mainWindow = [[UIApplication sharedApplication].windows objectAtIndex:0];
    [mainWindow.rootViewController.view addSubview:adContainerView];

    
    // スポット情報の取得
    NSString *spotIdString = charToNSString(spotId);
    IMobileCocos2dxModuleSpotData *spotData = [spots objectForKey:spotIdString];
    // スポットの登録
    [ImobileSdkAds registerWithPublisherID:spotData.publisherId MediaID:spotData.mediaId SpotID:spotIdString];
    // デリゲート設定
    [ImobileSdkAds setSpotDelegate:charToNSString(spotId) delegate:(id<IMobileSdkAdsDelegate>)[[IMobileCocos2dxModuleDelegate alloc] init]];
    // 広告取得開始
    [ImobileSdkAds startBySpotID:spotIdString];
    
    // バナー広告表示
    [ImobileSdkAds showBySpotID:spotIdString View:adContainerView SizeAdjust:sizeAdjust];
    
    adViewIdCounter++;
    [adViewIdDictionary setObject:adContainerView forKey:[NSString stringWithFormat:@"%d", adViewIdCounter]];
    
    return adViewIdCounter;
}
// 広告Viewの表示／非表示を指定します
void IMobileCocos2dxModule::setVisibility(const int adViewId, bool visible) {
    UIView *adContainerView = [adViewIdDictionary objectForKey:[NSString stringWithFormat:@"%d", adViewId]];
    adContainerView.hidden = !visible;
}

// 広告状態通知用のオブザーバを登録します
void IMobileCocos2dxModule::addObserver(cocos2d::CCObject *target, IMobileCocos2dxModule::AdNotficationType adNotficationType, const char *spotId, cocos2d::SEL_CallFuncO selector) {
    cocos2d::CCNotificationCenter::sharedNotificationCenter()->addObserver(target, selector, getObserverEntryName(adNotficationType, charToNSString(spotId)), NULL);
}

// 広告状態通知用のオブザーバを削除します
void IMobileCocos2dxModule::removeObserver(cocos2d::CCObject *target, IMobileCocos2dxModule::AdNotficationType adNotficationType, const char *spotId) {
    cocos2d::CCNotificationCenter::sharedNotificationCenter()->removeObserver(target, getObserverEntryName(adNotficationType, charToNSString(spotId)));
}
// 端末の物理解像度の取得を行います
cocos2d::CCSize IMobileCocos2dxModule::getDisplaySize(){
    EAGLView *eaglView = [EAGLView sharedEGLView];
    cocos2d::CCSize screenSize = cocos2d::CCSize(eaglView.bounds.size.width,eaglView.bounds.size.height);
    
    return screenSize;
}

// 広告が表示される向きの設定を設定します
// 初期値：AUTO
void IMobileCocos2dxModule::setAdOrientation(IMobileCocos2dxModule::AdOrientation orientation) {
    switch (orientation) {
        case IMobileCocos2dxModule::AUTO:
            [ImobileSdkAds setAdOrientation:IMOBILESDKADS_AD_ORIENTATION_AUTO];
            break;
        case IMobileCocos2dxModule::PORTRAIT:
            [ImobileSdkAds setAdOrientation:IMOBILESDKADS_AD_ORIENTATION_PORTRAIT];
            break;
        case IMobileCocos2dxModule::LANDSCAPE:
            [ImobileSdkAds setAdOrientation:IMOBILESDKADS_AD_ORIENTATION_LANDSCAPE];
            break;
        default:
            [ImobileSdkAds setAdOrientation:IMOBILESDKADS_AD_ORIENTATION_AUTO];
            break;
    }
}

// オフスクリーンウインドウのウインドウレベルを設定します
void IMobileCocos2dxModule::setOffscreenWindowLevel(AdWindowLevel windowLevel) {
    switch (windowLevel) {
        case IMobileCocos2dxModule::NORMAL:
            //[ImobileSdkAds setOffscreenWindowLevel:UIWindowLevelNormal];
            break;
        case IMobileCocos2dxModule::ALERT:
            //[ImobileSdkAds setOffscreenWindowLevel:UIWindowLevelAlert];
            break;
        case IMobileCocos2dxModule::STATUS_BAR:
            //[ImobileSdkAds setOffscreenWindowLevel:UIWindowLevelStatusBar];
            break;
        default:
            //[ImobileSdkAds setOffscreenWindowLevel:UIWindowLevelAlert];
            break;
    }
}

// 広告スポットの取得を停止します
void IMobileCocos2dxModule::stop(const char *spotId) {
    [ImobileSdkAds stopBySpotID:charToNSString(spotId)];
}

// 全ての広告スポットの取得を停止します
void IMobileCocos2dxModule::stopAll() {
    [ImobileSdkAds stop];
}

// 全ての広告スポットの取得を開始します
void IMobileCocos2dxModule::startAll() {
    [ImobileSdkAds start];
}

// テストモードの設定をします
void IMobileCocos2dxModule::setTestMode(bool testFlg){
    [ImobileSdkAds setTestMode:testFlg];
}

#pragma mark - 内部メソッド(実装)
//  const char * から NSString に変換する
static NSString* charToNSString(const char *value){
    return [NSString stringWithCString:value encoding:NSUTF8StringEncoding];
}

// 広告の表示領域を返します(表示位置パラメータ指定)
static CGRect getAdRect(IMobileCocos2dxModule::AdAlignPosition adAlignPosition, IMobileCocos2dxModule::AdValignPosition adValignPosition, IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModuleIconParams iconParams, bool sizeAdjust){
    
    // デバイスの物理サイズを取得
    //    cocos2d::Size screenSize = cocos2d::Director::getInstance()->getWinSize();
    // デバイスの論理サイズを取得
    EAGLView *eaglView = [EAGLView sharedEGLView];
    CGSize screenSize = eaglView.bounds.size;
    
    int x = 0;
    int y = 0;
    
    // 広告サイズを取得
    CGSize adSize = getAdSize(adType, iconParams, sizeAdjust);
    // X座標の取得
    int screenWidth = screenSize.width;
    switch (adAlignPosition) {
        case IMobileCocos2dxModule::LEFT:
            x = 0;
            break;
        case IMobileCocos2dxModule::CENTER:
            x = screenWidth - adSize.width;
            if (x != 0) {
                x = x / 2;
            }
            break;
        case IMobileCocos2dxModule::RIGHT:
            x = screenWidth - adSize.width;
            break;
    }
    // Y座標の取得
    int screenHeight = screenSize.height;
    switch (adValignPosition) {
        case IMobileCocos2dxModule::TOP:
            y = 0;
            break;
        case IMobileCocos2dxModule::MIDDLE:
            y = (screenHeight / 2) - (adSize.height / 2);
            break;
        case IMobileCocos2dxModule::BOTTOM:
            y = screenHeight - adSize.height;
            break;
    }
    return CGRectMake(x, y, adSize.width, adSize.height);
}

// 広告サイズを取得
static CGSize getAdSize(IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModuleIconParams iconParams,bool sizeAdjust) {
    
    CGSize adSize;
    switch (adType) {
        case IMobileCocos2dxModule::BANNER:
            adSize = getAdjustedAdSize(CGSizeMake(320, 50), sizeAdjust, IMobileCocos2dxModule::BANNER);
            break;
        case IMobileCocos2dxModule::BIG_BANNER:
            adSize = getAdjustedAdSize(CGSizeMake(320, 100), sizeAdjust, IMobileCocos2dxModule::BIG_BANNER);
            break;
        case IMobileCocos2dxModule::TABLET_BANNER:
            adSize = CGSizeMake(468,60);
            break;
        case IMobileCocos2dxModule::TABLET_BIG_BANNER:
            adSize = CGSizeMake(728,90);
            break;
        case IMobileCocos2dxModule::MEDIUM_RECTANGLE:
            adSize = getAdjustedAdSize(CGSizeMake(300, 250), sizeAdjust, IMobileCocos2dxModule::MEDIUM_RECTANGLE);
            break;
        case IMobileCocos2dxModule::BIG_RECTANGLE:
            adSize = CGSizeMake(336,280);
            break;
        case IMobileCocos2dxModule::SKYSCRAPER:
            adSize = CGSizeMake(120,600);
            break;
        case IMobileCocos2dxModule::WIDE_SKYSCRAPER:
            adSize = CGSizeMake(160,600);
            break;
        case IMobileCocos2dxModule::SQUARE:
            adSize = CGSizeMake(250,250);
            break;
        case IMobileCocos2dxModule::SMALL_SQUARE:
            adSize = CGSizeMake(200,200);
            break;
        case IMobileCocos2dxModule::HALFPAGE:
            adSize = CGSizeMake(300,600);
            break;
        case IMobileCocos2dxModule::ICON:
            int iconMinimumWidth = 47;          // アイコン一つあたりの最小サイズ
            int iconAdTitleReserveSpace = 3;    // タイトル表示エリアの予備マージン
            
            // アイコン広告の表示サイズをアイコンパラメータから計算して求める
            int iconAdWidth = 0;
            int iconAdHeight = 0;
            // デバイスの論理サイズを取得
            EAGLView *eaglView = [EAGLView sharedEGLView];
            CGSize screenSize = eaglView.bounds.size;
            // 幅を計算する
            int minWidth = screenSize.width > screenSize.height ? screenSize.height : screenSize.width;
            if (iconParams.iconViewLayoutWidth > 0) {
                iconAdWidth = iconParams.iconViewLayoutWidth;
            } else {
                iconAdWidth =  minWidth;
            }
            // 高さを計算する
            // アイコン一つあたりの幅を計算
            int iconImageWidth = 0;
            if (iconParams.iconSize > 0) {
                iconImageWidth  = fmax(iconParams.iconSize, iconMinimumWidth);
            } else {
                iconImageWidth = iconMinimumWidth;
            }
            
            // 高さの取得
            if (!iconParams.iconTitleEnable) {
                iconAdHeight = iconImageWidth;
            } else {
                int iconTitleOffset = (iconParams.iconTitleOffset > 0) ? iconParams.iconTitleOffset : 4;
                int iconTitleFontSize = (iconParams.iconTitleFontSize > 0) ? fmax(iconParams.iconTitleFontSize, 8) : 10;
                int iconTitleShadowDy = (iconParams.iconTitleShadowEnable) ? iconParams.iconTitleShadowDy : 0;
                iconAdHeight = iconImageWidth + iconTitleOffset + iconTitleFontSize * 2 + iconTitleShadowDy + iconAdTitleReserveSpace;
            }
            adSize = CGSizeMake(iconAdWidth, iconAdHeight);
            break;
    }
    return adSize;
}
// サイズ調整後の広告サイズを取得します
static CGSize getAdjustedAdSize(CGSize originalSize, bool sizeAdjust, IMobileCocos2dxModule::AdType adType)
{
    if (!sizeAdjust) {
        return originalSize;
    }
    
    int screenWidth = 0;
    int screenHeight = 0;
    
    EAGLView *eaglView = [EAGLView sharedEGLView];
    
    screenWidth = eaglView.bounds.size.width;
    screenHeight = eaglView.bounds.size.height;
    
    // スクリーンの小さい方を横幅として取得を行う
    screenWidth = (screenWidth < screenHeight) ? screenWidth : screenHeight;
    
    int adjustedWidth = (adType == IMobileCocos2dxModule::MEDIUM_RECTANGLE) ? screenWidth - 20 : screenWidth;
    int adjustedHeight = (int)round(originalSize.height * (double)adjustedWidth / (double)originalSize.width);
    
    return CGSizeMake(adjustedWidth, adjustedHeight);
}

// 登録オブザーバ名の取得
static const char* getObserverEntryName(IMobileCocos2dxModule::AdNotficationType adNotficationType, NSString *spotId) {
    
    // オブザーバ登録名の作成
    NSString *observerEntryString;
    switch (adNotficationType) {
        case IMobileCocos2dxModule::DID_READY:
            observerEntryString = [NSString stringWithFormat:@"IMOBILE_SDK_ADS_SPOT_DID_READY_%@", spotId];
            break;
        case IMobileCocos2dxModule::DID_FAIL:
            observerEntryString = [NSString stringWithFormat:@"IMOBILE_SDK_ADS_SPOT_DID_FAIL_%@", spotId];
            break;
        case IMobileCocos2dxModule::DID_SHOW:
            observerEntryString = [NSString stringWithFormat:@"IMOBILE_SDK_ADS_SPOT_DID_SHOW_%@", spotId];
            break;
        case IMobileCocos2dxModule::DID_CLICK:
            observerEntryString = [NSString stringWithFormat:@"IMOBILE_SDK_ADS_SPOT_DID_CLICK_%@", spotId];
            break;
        case IMobileCocos2dxModule::DID_CLOSE:
            observerEntryString = [NSString stringWithFormat:@"IMOBILE_SDK_ADS_SPOT_DID_CLOSE_%@", spotId];
            break;
    }
    return [observerEntryString UTF8String];
}


// 通常の通知を処理します
static void normalNotification(IMobileCocos2dxModule::AdNotficationType adNotficationType, NSString *spotId) {
    cocos2d::CCNotificationCenter::sharedNotificationCenter()->postNotification(getObserverEntryName(adNotficationType, spotId), NULL);
}

// 異常系の通知を処理します
static void abnormalNotification(IMobileCocos2dxModule::AdNotficationType adNotficationType, NSString *spotId,  NSString *reason) {
    cocos2d::CCString value = cocos2d::CCString([reason UTF8String]);
    cocos2d::CCNotificationCenter::sharedNotificationCenter()->postNotification(getObserverEntryName(adNotficationType, spotId) , &value);
}

