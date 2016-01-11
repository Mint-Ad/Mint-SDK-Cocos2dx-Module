//
//  IMobileCocos2dxModule.cpp
//
//  Copyright (c) 2014 i-mobile. All rights reserved.
//

#include "IMobileCocos2dxModule.h"
#include <jni.h>
#include "platform/android/jni/JniHelper.h"

#define JAVA_MODULE_CLASS_NAME "jp/co/imobile/sdkads/android/cocos2dx/Module"

USING_NS_CC;

// スポットデータ
class IMobileCocos2dxModuleSpotData : public CCObject {
public:
	cocos2d::CCString *publisherId;
	cocos2d::CCString *mediaId;

	IMobileCocos2dxModuleSpotData(){
		publisherId = new cocos2d::CCString();
		mediaId = new cocos2d::CCString();
	}

	~IMobileCocos2dxModuleSpotData(){
		publisherId->release();
		mediaId->release();
	}
};

/* 変数宣言 */
// AdViewIdカウンタ
static int viewIdCounter = 100000;
// スポット情報管理(インラインの場合のみ使用)
static cocos2d::Dictionary *spots = new cocos2d::Dictionary;
// DeviceDensity (この値を直接扱わず、getDeviceDensityで取得すること)
static float deviceDensity = 0;

/* プロトタイプ宣言 */
// 広告の表示領域を返します(表示位置パラメータ指定)
static cocos2d::Rect getAdRect(IMobileCocos2dxModule::AdAlignPosition adAlignPosition, IMobileCocos2dxModule::AdValignPosition adValignPosition, IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModuleIconParams iconParams, bool sizeAdjust);
// 広告サイズを取得します
static cocos2d::Size getAdSize (IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModuleIconParams iconParams,bool sizeAdjust);
// サイズ調整後の広告サイズを取得します
static cocos2d::Size getAdjustedAdSize(cocos2d::Size originalSize, bool sizeAdjust, IMobileCocos2dxModule::AdType adType);
// showメソッドの実装
static int showImpl(const char* spotId, IMobileCocos2dxModule::AdType adType, float x, float y, IMobileCocos2dxModuleIconParams iconParams, bool sizeAdjust);

static const char* getObserverEntryName(IMobileCocos2dxModule::AdNotficationType adNotficationType, const char *spotId);

// コンストラクタ隠蔽
IMobileCocos2dxModule::IMobileCocos2dxModule() {}

/* 公開メソッド */
// スポット登録(全画面)を行い、広告の取得を開始します
void IMobileCocos2dxModule::registerSpotFullScreen(const char *publisherId, const char *mediaId, const char *spotId) {
	// スポットの登録
	cocos2d::JniMethodInfo jniMethodInfo;
	if (cocos2d::JniHelper::getStaticMethodInfo(jniMethodInfo, JAVA_MODULE_CLASS_NAME, "registerSpotFullScreen", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V")) {
		jstring stringPublisherId = jniMethodInfo.env->NewStringUTF(publisherId);
		jstring stringMediaId = jniMethodInfo.env->NewStringUTF(mediaId);
		jstring stringSpotId = jniMethodInfo.env->NewStringUTF(spotId);
		jniMethodInfo.env->CallStaticVoidMethod(jniMethodInfo.classID, jniMethodInfo.methodID, stringPublisherId, stringMediaId, stringSpotId);
		jniMethodInfo.env->DeleteLocalRef(stringPublisherId);
		jniMethodInfo.env->DeleteLocalRef(stringMediaId);
		jniMethodInfo.env->DeleteLocalRef(stringSpotId);
		jniMethodInfo.env->DeleteLocalRef(jniMethodInfo.classID);
	}
}

// スポット登録(インライン)を行います
// インラインの場合は、スポットを管理用Dictionaryに登録し、Showのタイミングでスポット登録、広告開始、広告表示を行う
void IMobileCocos2dxModule::registerSpotInline(const char *publisherId, const char *mediaId, const char *spotId) {
	// スポット情報の作成
	IMobileCocos2dxModuleSpotData *spotData = new IMobileCocos2dxModuleSpotData();
	spotData->publisherId->initWithFormat("%s", publisherId);
	spotData->mediaId->initWithFormat("%s", mediaId);
	// スポット情報の削除
    spots->removeObjectForKey((intptr_t)spotId);
	// スポット情報の登録
	spots->setObject(spotData, (intptr_t)spotId);
}

// 全画面広告を表示します
void IMobileCocos2dxModule::show(const char* spotId) {
	cocos2d::JniMethodInfo jniMethodInfo;
	if (cocos2d::JniHelper::getStaticMethodInfo(jniMethodInfo, JAVA_MODULE_CLASS_NAME, "showAd", "(Ljava/lang/String;)V")) {
		jstring stringSpotID = jniMethodInfo.env->NewStringUTF(spotId);
		jniMethodInfo.env->CallStaticVoidMethod(jniMethodInfo.classID, jniMethodInfo.methodID, stringSpotID);
		jniMethodInfo.env->DeleteLocalRef(stringSpotID);
		jniMethodInfo.env->DeleteLocalRef(jniMethodInfo.classID);
	}
}

//インライン広告を表示します
// インライン広告表示します(表示位置パラメータ指定)
int IMobileCocos2dxModule::show(const char* spotId, IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModule::AdAlignPosition adAlignPosition, IMobileCocos2dxModule::AdValignPosition adValignPosition) {
	IMobileCocos2dxModuleIconParams iconParams;
    // 表示位置パラメータを座標に変換してから呼び出し
    cocos2d::Rect adRect = getAdRect(adAlignPosition, adValignPosition, adType, iconParams, false);
    return showImpl(spotId, adType, adRect.origin.x ,adRect.origin.y, iconParams, false);
}

// インライン広告を表示します(表示位置パラメータ指定、表示サイズ自動調節)
int IMobileCocos2dxModule::show(const char* spotId, IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModule::AdAlignPosition adAlignPosition, IMobileCocos2dxModule::AdValignPosition adValignPosition, bool sizeAdjust){
	IMobileCocos2dxModuleIconParams iconParams;
    // 表示位置パラメータを座標に変換してから呼び出し
    cocos2d::Rect adRect = getAdRect(adAlignPosition, adValignPosition, adType, iconParams, sizeAdjust);
    return showImpl(spotId, adType, adRect.origin.x ,adRect.origin.y, iconParams, sizeAdjust);
}

// インライン広告表示します(表示位置パラメータ指定、アイコン表示パラメータ指定)
int IMobileCocos2dxModule::show(const char* spotId, IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModule::AdAlignPosition adAlignPosition, IMobileCocos2dxModule::AdValignPosition adValignPosition, IMobileCocos2dxModuleIconParams iconParams) {
	// アイコンの個数を7個数以上の場合は、アイコンの個数を6個にする。
    if(iconParams.iconNumber >= 7){
    	iconParams.iconNumber = 6;
    }
    // 表示位置パラメータを座標に変換してから呼び出し
    cocos2d::Rect adRect = getAdRect(adAlignPosition, adValignPosition, adType, iconParams, false);
    return showImpl(spotId, adType, adRect.origin.x ,adRect.origin.y, iconParams, false);
}

// インライン広告表示します(座標指定)
int IMobileCocos2dxModule::show(const char* spotId, IMobileCocos2dxModule::AdType adType, float x, float y) {
    IMobileCocos2dxModuleIconParams iconParams;
    return showImpl(spotId, adType, x, y, iconParams, false);
}

// インライン広告を表示します(座標指定、表示サイズ自動調節）
int IMobileCocos2dxModule::show(const char* spotId, IMobileCocos2dxModule::AdType adType, float x, float y, bool sizeAdjust){
    IMobileCocos2dxModuleIconParams iconParams;
    return showImpl(spotId, adType, x, y, iconParams, sizeAdjust);
}

// インライン広告を表示します(座標指定、アイコン表示パラメータ指定)
int IMobileCocos2dxModule::show(const char* spotId, IMobileCocos2dxModule::AdType adType, float x, float y, IMobileCocos2dxModuleIconParams iconParams){
	// アイコンの個数を7個数以上の場合は、アイコンの個数を6個にする。
    if(iconParams.iconNumber >= 7){
    	iconParams.iconNumber = 6;
    }
    return showImpl(spotId, adType, x, y, iconParams, false);
}

// showメソッドの実装
static int showImpl(const char* spotId, IMobileCocos2dxModule::AdType adType, float x, float y, IMobileCocos2dxModuleIconParams iconParams, bool sizeAdjust) {
	// インスタンス生成前(registerを呼ぶ前、releaseを呼んだあとにregisterする前に呼んだ場合は、spotsがnullのため処理ができない。)
	if (nullptr == spots) {
    	return -1;
	}

	// インライン広告表示処理(Javaのメソッド)呼び出し
	cocos2d::JniMethodInfo jniMethodInfo;
	int adViewId = viewIdCounter ++;
	if (cocos2d::JniHelper::getStaticMethodInfo(jniMethodInfo, JAVA_MODULE_CLASS_NAME, "show", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIIIIIZILjava/lang/String;IZLjava/lang/String;IIZZ)V")) {
		// スポット情報を取得し、パートナーID と メディアIDを取得する
		IMobileCocos2dxModuleSpotData *spotData = (IMobileCocos2dxModuleSpotData *)spots->objectForKey((intptr_t)spotId);
	    if (nullptr == spotData) {
	    	return -1;
	    }
		// Java に渡す文字列引数を生成
	    jstring stringPublisherID = jniMethodInfo.env->NewStringUTF(spotData->publisherId->getCString());
	    jstring stringMediaID = jniMethodInfo.env->NewStringUTF(spotData->mediaId->getCString());
		jstring stringSpotID = jniMethodInfo.env->NewStringUTF(spotId);
		jstring stringIconTitleFontColor = jniMethodInfo.env->NewStringUTF(iconParams.iconTitleFontColor);
		jstring stringIconTitleShadowColor = jniMethodInfo.env->NewStringUTF(iconParams.iconTitleShadowColor);
		// アイコンか判定する
		bool isIcon = (adType == IMobileCocos2dxModule::ICON)? true : false;
		// Java のメソッドの呼び出し
		jniMethodInfo.env->CallStaticVoidMethod(jniMethodInfo.classID, jniMethodInfo.methodID, stringPublisherID, stringMediaID, stringSpotID, adViewId, (int)x, (int)y,
				iconParams.iconNumber, iconParams.iconViewLayoutWidth, iconParams.iconSize, iconParams.iconTitleEnable, iconParams.iconTitleFontSize, stringIconTitleFontColor, iconParams.iconTitleOffset,
				iconParams.iconTitleShadowEnable, stringIconTitleShadowColor, iconParams.iconTitleShadowDx, iconParams.iconTitleShadowDy,sizeAdjust,isIcon);
		// Java のメソッドの呼び出しに使った変数を破棄

		jniMethodInfo.env->DeleteLocalRef(stringPublisherID);
		jniMethodInfo.env->DeleteLocalRef(stringMediaID);
		jniMethodInfo.env->DeleteLocalRef(stringSpotID);
		jniMethodInfo.env->DeleteLocalRef(stringIconTitleFontColor);
		jniMethodInfo.env->DeleteLocalRef(stringIconTitleShadowColor);
		jniMethodInfo.env->DeleteLocalRef(jniMethodInfo.classID);
	}
	return adViewId;
}

// 広告Viewの表示／非表示を切り替えます
void IMobileCocos2dxModule::setVisibility(const int adViewId, const bool visible) {
	cocos2d::JniMethodInfo jniMethodInfo;
	if (cocos2d::JniHelper::getStaticMethodInfo(jniMethodInfo, JAVA_MODULE_CLASS_NAME, "setVisibility", "(IZ)V")) {
		jniMethodInfo.env->CallStaticVoidMethod(jniMethodInfo.classID, jniMethodInfo.methodID, adViewId, visible);
		jniMethodInfo.env->DeleteLocalRef(jniMethodInfo.classID);
	}
}

// オフスクリーンウインドウのウインドウレベルを設定します(iOS Only)
void IMobileCocos2dxModule::setOffscreenWindowLevel(IMobileCocos2dxModule::AdWindowLevel windowLevel) {
	//何もしない
}

// 広告が表示される向きを指定します
// 初期値：AUTO
void IMobileCocos2dxModule::setAdOrientation(IMobileCocos2dxModule::AdOrientation orientation) {
	//何もしない(Androidの場合は、Manifestにて設定)
	cocos2d::JniMethodInfo jniMethodInfo;
	if (cocos2d::JniHelper::getStaticMethodInfo(jniMethodInfo, JAVA_MODULE_CLASS_NAME, "setAdOrientation", "(I)V")) {
		jniMethodInfo.env->CallStaticVoidMethod(jniMethodInfo.classID, jniMethodInfo.methodID, orientation);
		jniMethodInfo.env->DeleteLocalRef(jniMethodInfo.classID);
	}
}

// 広告スポットの取得を停止します
void IMobileCocos2dxModule::stop(const char *spotId) {
	cocos2d::JniMethodInfo jniMethodInfo;
	if (cocos2d::JniHelper::getStaticMethodInfo(jniMethodInfo, JAVA_MODULE_CLASS_NAME, "stop", "(Ljava/lang/String;)V")) {
		jstring stringSpotID = jniMethodInfo.env->NewStringUTF(spotId);
		jniMethodInfo.env->CallStaticVoidMethod(jniMethodInfo.classID, jniMethodInfo.methodID, stringSpotID);
		jniMethodInfo.env->DeleteLocalRef(stringSpotID);
		jniMethodInfo.env->DeleteLocalRef(jniMethodInfo.classID);
	}
}

// 全ての広告スポットの取得を停止します
void IMobileCocos2dxModule::stopAll() {
	cocos2d::JniMethodInfo jniMethodInfo;
	if (cocos2d::JniHelper::getStaticMethodInfo(jniMethodInfo, JAVA_MODULE_CLASS_NAME, "stopAll", "()V")) {
		jniMethodInfo.env->CallStaticVoidMethod(jniMethodInfo.classID, jniMethodInfo.methodID);
		jniMethodInfo.env->DeleteLocalRef(jniMethodInfo.classID);
	}
}

// 全ての広告スポットの取得を開始します
void IMobileCocos2dxModule::startAll() {
	cocos2d::JniMethodInfo jniMethodInfo;
	if (cocos2d::JniHelper::getStaticMethodInfo(jniMethodInfo, JAVA_MODULE_CLASS_NAME, "startAll", "()V")) {
		jniMethodInfo.env->CallStaticVoidMethod(jniMethodInfo.classID, jniMethodInfo.methodID);
		jniMethodInfo.env->DeleteLocalRef(jniMethodInfo.classID);
	}
}
//テストモードの設定をします
void IMobileCocos2dxModule::setTestMode(bool testFlag){
	cocos2d::JniMethodInfo jniMethodInfo;
	if (cocos2d::JniHelper::getStaticMethodInfo(jniMethodInfo, JAVA_MODULE_CLASS_NAME, "setTestMode", "(Z)V")) {
		jniMethodInfo.env->CallStaticVoidMethod(jniMethodInfo.classID, jniMethodInfo.methodID,testFlag);
		jniMethodInfo.env->DeleteLocalRef(jniMethodInfo.classID);
	}
}

/* プライペートメソッド */
// デバイスのDensity値を取得します
static float getDeviceDensity() {
	if (deviceDensity == 0) {
		cocos2d::JniMethodInfo jniMethodInfo;
		if (cocos2d::JniHelper::getStaticMethodInfo(jniMethodInfo, JAVA_MODULE_CLASS_NAME, "getDeviceDensity", "()F")) {
			deviceDensity = jniMethodInfo.env->CallStaticFloatMethod(jniMethodInfo.classID, jniMethodInfo.methodID);
			jniMethodInfo.env->DeleteLocalRef(jniMethodInfo.classID);
		}
	}
	return deviceDensity;
}

// 広告の表示領域を返します(表示位置パラメータ指定版)
static cocos2d::Rect getAdRect(IMobileCocos2dxModule::AdAlignPosition adAlignPosition, IMobileCocos2dxModule::AdValignPosition adValignPosition, IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModuleIconParams iconParams, bool sizeAdjust){

    // デバイスの物理サイズを取得
    cocos2d::Size frameSize = cocos2d::Director::getInstance()->getOpenGLView()->getFrameSize();
    // 広告サイズ(論理サイズ)を取得
    cocos2d::Size adSize = getAdSize(adType, iconParams, sizeAdjust);

    // X座標の取得
    int x = 0;
    int screenWidth = trunc(frameSize.width / getDeviceDensity());
    switch (adAlignPosition) {
        case IMobileCocos2dxModule::AdAlignPosition::LEFT:
            x = 0;
            break;
        case IMobileCocos2dxModule::AdAlignPosition::CENTER:
            x = screenWidth - adSize.width;
            if(x != 0) {
                x = x / 2;
            }
            break;
        case IMobileCocos2dxModule::AdAlignPosition::RIGHT:
            x = screenWidth - adSize.width;
            break;
    }
    // Y座標の取得
    int y = 0;
    int screenHeight = trunc(frameSize.height / getDeviceDensity());
    switch (adValignPosition) {
        case IMobileCocos2dxModule::AdValignPosition::TOP:
            y = 0;
            break;
        case IMobileCocos2dxModule::AdValignPosition::MIDDLE:
            y = (screenHeight / 2) - (adSize.height / 2);
            break;
        case IMobileCocos2dxModule::AdValignPosition::BOTTOM:
            y = screenHeight - adSize.height;
            break;
    }

    return cocos2d::Rect(x, y, adSize.width, adSize.height);
}

// 広告サイズを取得します(返すサイズは論理値)
static cocos2d::Size getAdSize(IMobileCocos2dxModule::AdType adType, IMobileCocos2dxModuleIconParams iconParams, bool sizeAdjust) {

	cocos2d::Size adSize;
	switch (adType) {
        case IMobileCocos2dxModule::AdType::BANNER:
        	adSize = getAdjustedAdSize(cocos2d::Size(320,50), sizeAdjust, adType);
            break;
        case IMobileCocos2dxModule::AdType::BIG_BANNER:
        	adSize = getAdjustedAdSize(cocos2d::Size(320,100), sizeAdjust, adType);
            break;
		case IMobileCocos2dxModule::AdType::TABLET_BANNER:
			adSize = cocos2d::Size(468,60);
			break;
		case IMobileCocos2dxModule::AdType::TABLET_BIG_BANNER:
			adSize = cocos2d::Size(728,90);
			break;
        case IMobileCocos2dxModule::AdType::MEDIUM_RECTANGLE:
        	adSize = getAdjustedAdSize(cocos2d::Size(300,250), sizeAdjust, adType);
            break;
		case IMobileCocos2dxModule::AdType::BIG_RECTANGLE:
			adSize = cocos2d::Size(336,280);
			break;
		case IMobileCocos2dxModule::AdType::SKYSCRAPER:
			adSize = cocos2d::Size(120,600);
			break;
		case IMobileCocos2dxModule::AdType::WIDE_SKYSCRAPER:
			adSize = cocos2d::Size(160,600);
			break;
		case IMobileCocos2dxModule::AdType::SQUARE:
			adSize = cocos2d::Size(250,250);
			break;
		case IMobileCocos2dxModule::AdType::SMALL_SQUARE:
			adSize = cocos2d::Size(200,200);
			break;
		case IMobileCocos2dxModule::AdType::HALFPAGE:
			adSize = cocos2d::Size(300,600);
			break;
        case IMobileCocos2dxModule::AdType::ICON:
            int iconDefaultWidth = 57;          // アイコン一つあたりのデフォルトサイズ
            int iconMinimumWidth = 47;          // アイコン一つあたりの最小サイズ
            int iconDefaultMargin = 18;         // デフォルトアイコン間隔
            int iconMinimumMargin = 4;          // 最小アイコン間隔
            int iconAdTitleReserveSpace = 3;    // タイトル表示エリアの予備マージン

            // アイコン広告の表示サイズをアイコンパラメータから計算して求める
            int iconAdWidth = 0;
            int iconAdHeight = 0;
            // デバイスの物理サイズを取得
            cocos2d::Size frameSize = cocos2d::Director::getInstance()->getOpenGLView()->getFrameSize();
            // 幅を計算する
            int minWidth = trunc((frameSize.width > frameSize.height ? frameSize.height : frameSize.width) / getDeviceDensity());
            if (iconParams.iconViewLayoutWidth > 0) {
                 iconAdWidth = iconParams.iconViewLayoutWidth;
            } else {

                if (iconParams.iconNumber < 4) {
                    int iconWidth = (iconParams.iconSize > 0) ? fmax (iconParams.iconSize, iconMinimumWidth) : iconDefaultWidth;
                    iconAdWidth =  (iconWidth + iconDefaultMargin) * iconParams.iconNumber;
                } else {
                    iconAdWidth =  minWidth;
                }
            }
            // 高さを計算する
            // アイコン一つあたりの幅を計算
            int iconImageWidth = 0;
            if (iconParams.iconSize > 0) {
                iconImageWidth  = fmax(iconParams.iconSize, iconMinimumWidth);
            } else {
                if (iconAdWidth >= (iconDefaultWidth + iconDefaultMargin) * iconParams.iconNumber) {
                    iconImageWidth = iconDefaultWidth;
                } else {
                    iconImageWidth = iconMinimumWidth;
                }
            }
            // アイコンの表示間隔を取得(アイコン広告の表示サイス - アイコン一つあたりの幅 * アイコンの数) / アイコンの数 の結果と最低マージンとの比較で大きい方をマージンとして設定
            int iconImageMargin = fmax((int)ceil((double)(iconAdWidth - iconImageWidth * iconParams.iconNumber) / iconParams.iconNumber) , iconMinimumMargin);

            // 高さの取得
            if (!iconParams.iconTitleEnable) {
                iconAdHeight = iconImageWidth;
            } else {
                int iconTitleOffset = (iconParams.iconTitleOffset > 0) ? iconParams.iconTitleOffset : 4;
                int iconTitleFontSize = (iconParams.iconTitleFontSize > 0) ? fmax(iconParams.iconTitleFontSize, 8) : 10;
                int iconTitleShadowDy = (iconParams.iconTitleShadowEnable) ? iconParams.iconTitleShadowDy : 0;

                // タイトルが一行に収まるか計算
                if ((iconImageWidth + iconImageMargin) >= (iconDefaultWidth + iconDefaultMargin / 2)) {
                    iconAdHeight = iconImageWidth + iconTitleOffset + iconTitleFontSize + iconTitleShadowDy + iconAdTitleReserveSpace;
                } else {
                    iconAdHeight = iconImageWidth + iconTitleOffset + iconTitleFontSize * 2 + iconTitleShadowDy + iconAdTitleReserveSpace;
                }
            }
            adSize = cocos2d::Size(iconAdWidth, iconAdHeight);
            break;
    }
	return adSize;
}
static cocos2d::Size getAdjustedAdSize(cocos2d::Size originalSize, bool sizeAdjust, IMobileCocos2dxModule::AdType adType){
    if (!sizeAdjust) {
        return originalSize;
    }
    // デバイスの物理サイズを取得
    cocos2d::Size frameSize = cocos2d::Director::getInstance()->getOpenGLView()->getFrameSize();

    int screenWidth = trunc(frameSize.width / getDeviceDensity());
    int screenHeight = trunc(frameSize.height / getDeviceDensity());

    // スクリーンの小さい方を横幅として取得を行う
    screenWidth = (screenWidth < screenHeight) ? screenWidth : screenHeight;

    int adjustedWidth = (adType == IMobileCocos2dxModule::AdType::MEDIUM_RECTANGLE) ? screenWidth - 20 : screenWidth;
    int adjustedHeight = (int)round(originalSize.height * (double)adjustedWidth / (double)originalSize.width);

    return cocos2d::Size(adjustedWidth, adjustedHeight);
}

/* 通知処理(Javaからの通知受け取り用) */
// [公開メソッド]広告状態通知用のオブザーバを登録します
void IMobileCocos2dxModule::addObserver(cocos2d::Ref *target, IMobileCocos2dxModule::AdNotficationType adNotficationType, const char *spotId, cocos2d::SEL_CallFuncO selector) {
	cocos2d::CCNotificationCenter::getInstance()->addObserver(target, selector, getObserverEntryName(adNotficationType, spotId), NULL);
}

// [公開メソッド]広告状態通知用のオブザーバを削除します
void IMobileCocos2dxModule::removeObserver(cocos2d::Ref *target, IMobileCocos2dxModule::AdNotficationType adNotficationType, const char *spotId) {
	cocos2d::CCNotificationCenter::getInstance()->removeObserver(target, getObserverEntryName(adNotficationType, spotId));
}

// [公開メソッド]端末の物理解像度の取得を行います
cocos2d::Size IMobileCocos2dxModule::getDisplaySize(){
    // デバイスの物理サイズを取得
    cocos2d::Size frameSize = cocos2d::Director::getInstance()->getOpenGLView()->getFrameSize();
    int screenWidth = trunc(frameSize.width / getDeviceDensity());
    int screenHeight = trunc(frameSize.height / getDeviceDensity());

    return cocos2d::Size(screenWidth,screenHeight);
}

// SpotIDと通知の種類(AdNotficationType)からNotificationに登録されているメソッドを探し出し呼び出します
// 通常の通知を処理します
void static normalNotification(IMobileCocos2dxModule::AdNotficationType adNotficationType, const char* spotId) {
	cocos2d::CCNotificationCenter::getInstance()->postNotification(getObserverEntryName(adNotficationType, spotId) ,NULL);
}

// 異常系の通知を処理します
void static abnormalNotification(IMobileCocos2dxModule::AdNotficationType adNotficationType, const char* spotId, const char* reason) {
    cocos2d::String value = cocos2d::String(reason);
	cocos2d::CCNotificationCenter::getInstance()->postNotification(getObserverEntryName(adNotficationType, spotId), &value);
	if ( strcmp("AD_NOT_READY", reason) == 0 ) {
		cocos2d::CCNotificationCenter::getInstance()->postNotification(getObserverEntryName(IMobileCocos2dxModule::AdNotficationType::DID_CLOSE, spotId) ,NULL);
	}
}

// 登録オブザーバ名の取得
static const char* getObserverEntryName(IMobileCocos2dxModule::AdNotficationType adNotficationType, const char *spotId) {
	// オブザーバ登録名の作成
	cocos2d::String *observerEntryString;
	switch (adNotficationType) {
		case IMobileCocos2dxModule::AdNotficationType::DID_READY:
			observerEntryString = cocos2d::String::createWithFormat("IMOBILE_SDK_ADS_SPOT_DID_READY_%s", spotId);
			break;
		case IMobileCocos2dxModule::AdNotficationType::DID_FAIL:
			observerEntryString = cocos2d::String::createWithFormat("IMOBILE_SDK_ADS_SPOT_DID_FAIL_%s", spotId);
			break;
		case IMobileCocos2dxModule::AdNotficationType::DID_SHOW:
			observerEntryString = cocos2d::String::createWithFormat("IMOBILE_SDK_ADS_SPOT_DID_SHOW_%s", spotId);
			break;
		case IMobileCocos2dxModule::AdNotficationType::DID_CLICK:
			observerEntryString = cocos2d::String::createWithFormat("IMOBILE_SDK_ADS_SPOT_DID_CLICK_%s", spotId);
			break;
		case IMobileCocos2dxModule::AdNotficationType::DID_CLOSE:
			observerEntryString = cocos2d::String::createWithFormat("IMOBILE_SDK_ADS_SPOT_DID_CLOSE_%s", spotId);
			break;
	}
	return observerEntryString->getCString();
}

// Javaから呼び出される通知用のメソッド(Java側で [public static native void xx] で宣言されているメソッド )
extern "C"
{
	//広告の表示準備が完了した際に呼び出されます
	JNIEXPORT void JNICALL Java_jp_co_imobile_sdkads_android_cocos2dx_Module_onAdReadyCompleted(JNIEnv* env, jobject thiz, jstring spotId) {
		normalNotification(IMobileCocos2dxModule::AdNotficationType::DID_READY, env->GetStringUTFChars(spotId, 0));
	}
	//広告が表示された際に呼び出されます
	JNIEXPORT void JNICALL Java_jp_co_imobile_sdkads_android_cocos2dx_Module_onAdShowCompleted(JNIEnv* env, jobject thiz, jstring spotId) {
		normalNotification(IMobileCocos2dxModule::AdNotficationType::DID_SHOW, env->GetStringUTFChars(spotId, 0));
	}
	//広告がクリックされた際に呼び出されます
	JNIEXPORT void JNICALL Java_jp_co_imobile_sdkads_android_cocos2dx_Module_onAdCliclkCompleted(JNIEnv* env, jobject thiz, jstring spotId) {
		normalNotification(IMobileCocos2dxModule::AdNotficationType::DID_CLICK, env->GetStringUTFChars(spotId, 0));
	}
	//広告が閉じられた際に呼び出されます
	JNIEXPORT void JNICALL Java_jp_co_imobile_sdkads_android_cocos2dx_Module_onAdCloseCompleted(JNIEnv* env, jobject thiz, jstring spotId) {
		normalNotification(IMobileCocos2dxModule::AdNotficationType::DID_CLOSE, env->GetStringUTFChars(spotId, 0));
	}
	//広告の取得に失敗した際に呼び出されます。(reasonは、失敗理由が設定されます)
	JNIEXPORT void JNICALL Java_jp_co_imobile_sdkads_android_cocos2dx_Module_onFailed(JNIEnv* env, jobject thiz, jstring spotId, jstring reason) {
		abnormalNotification(IMobileCocos2dxModule::AdNotficationType::DID_FAIL, env->GetStringUTFChars(spotId, 0), env->GetStringUTFChars(reason, 0));
	}
}
