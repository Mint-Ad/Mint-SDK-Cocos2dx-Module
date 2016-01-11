//
//  Module.java
//
//  Copyright (c) 2014 i-mobile. All rights reserved.
//

package jp.co.imobile.sdkads.android.cocos2dx;

import jp.co.imobile.sdkads.android.AdOrientation;
import jp.co.imobile.sdkads.android.FailNotificationReason;
import jp.co.imobile.sdkads.android.ImobileIconParams;
import jp.co.imobile.sdkads.android.ImobileSdkAd;
import jp.co.imobile.sdkads.android.ImobileSdkAdListener;

import org.cocos2dx.lib.Cocos2dxActivity;

import android.app.Activity;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.RelativeLayout;

public class Module {
	/* [privete] カレントActivityを取得します */
	private static Activity getCurrentActivity() {
		return (Activity)Cocos2dxActivity.getContext();
	}

	/* [privete] 指定されたスポットIDに対応するImobileSdkAdListenerを生成します */
	// C++側の受け取り用メソッドの定義
	public static native void onAdReadyCompleted(String spotId);
	public static native void onAdShowCompleted(String spotId);
	public static native void onAdCliclkCompleted(String spotId);
	public static native void onAdCloseCompleted(String spotId);
	public static native void onFailed(String spotId, String reason);

	private static ImobileSdkAdListener getSpotListener(final String spotId) {
		return new ImobileSdkAdListener() {
			//広告の表示準備が完了した際に呼び出されます
			@Override
			public void onAdReadyCompleted() {
				Module.onAdReadyCompleted(spotId);
			}
			//広告が表示された際に呼び出されます
			@Override
			public void onAdShowCompleted() {
				Module.onAdShowCompleted(spotId);
			}
			//広告がクリックされた際に呼び出されます
			@Override
			public void onAdCliclkCompleted() {
				Module.onAdCliclkCompleted(spotId);
			}
			//広告が閉じられた際に呼び出されます
			@Override
			public void onAdCloseCompleted() {
				final Activity activity = getCurrentActivity();
				activity.runOnUiThread(new Runnable(){
					public void run(){
						Module.onAdCloseCompleted(spotId);
					}
				});
			}
			//広告の取得に失敗した際に呼び出されます。(reasonは、失敗理由が設定されます)
			@Override
			public void onFailed(FailNotificationReason reason) {
				Module.onFailed(spotId, reason.toString());
			}
		};
	}

	/* [privete] 解像度によるピクセル計算(dpi to px) */
	private static int convertDpiToPixel(Activity activity,int dpiValue){
		return (int)(dpiValue * getDeviceDensity() + 0.5f);
	}
		
	/* 全画面広告スポットを登録します */
	public static void registerSpotFullScreen(final String publisherId, final String mediaId, final String spotId) {
		final Activity activity = getCurrentActivity();
		activity.runOnUiThread(new Runnable() {
			public void run() {
				// スポットの登録
				ImobileSdkAd.registerSpotFullScreen(activity, publisherId, mediaId, spotId);
				// スポットリスナーの設定
				ImobileSdkAd.setImobileSdkAdListener(spotId, getSpotListener(spotId));
				// 広告の取得開始
				ImobileSdkAd.start(spotId);
			}
		});
	}
	
	/* 全画面広告を表示します */
	public static void showAd(final String spotId) {
		final Activity activity = getCurrentActivity();
		activity.runOnUiThread(new Runnable() {
			public void run() {
				// 広告の表示
				ImobileSdkAd.showAd(activity, spotId);
			}
		});
	}
	
	/* (開発Temp)広告の表示(インライン) [注意]インラインの場合は、スポットの登録、広告の取得、広告の表示をこのメソッドのみで実行します */
	public static void showAd(final int adViewId, final String publisherId, final String mediaId, final String spotId, final int left, final int top) {
		final Activity activity = getCurrentActivity();
		activity.runOnUiThread(new Runnable() {
			public void run() {
				// スポットの登録
				ImobileSdkAd.registerSpotInline(activity, publisherId, mediaId, spotId);
				// スポットリスナーの設定
				ImobileSdkAd.setImobileSdkAdListener(spotId, getSpotListener(spotId));
				// 広告の取得開始
				ImobileSdkAd.start(spotId);
				// 広告の表示
				// アイコンパラメータオブジェクトの作成
				// 広告表示用のViewを作成
				ViewGroup adView = (ViewGroup)activity.findViewById(adViewId);
				if (adView != null) {
					// 既にViewが存在している場合は、何もしない
					return;
				}
				adView = new RelativeLayout(activity);
				adView.setId(adViewId);
				adView.setPadding(convertDpiToPixel(activity, left), convertDpiToPixel(activity, top), 0, 0);
				activity.addContentView(adView, new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT,LayoutParams.WRAP_CONTENT));
				// 広告表示処理呼び出し
				ImobileSdkAd.showAd(activity, spotId, adView);
			}
		});
	}

	/* 広告の表示(インライン) [注意]インラインの場合は、スポットの登録、広告の取得、広告の表示をこのメソッドのみで実行します */
	public static void show(final String publisherId, final String mediaId, final String spotId, final int adViewid, final int left, final int top,
			final int iconNumber,final int iconViewLayoutWidth, final int iconSize, final boolean iconTitleEnable, final int iconTitleFontSize, final String iconTitleFontColor, final int iconTitleOffset, 
			final boolean iconTitleShadowEnable, final String iconTitleShadowColor, final int iconTitleShadowDx, final int iconTitleShadowDy, final boolean sizeAdjust, final boolean isIcon){
		final Activity activity = getCurrentActivity();
		activity.runOnUiThread(new Runnable(){
			public void run(){
				ViewGroup adView = (ViewGroup)activity.findViewById(adViewid);
				if (adView != null) {
					return;
				}
				// スポットの登録
				ImobileSdkAd.registerSpotInline(activity, publisherId, mediaId, spotId);
				// スポットリスナーの設定
				ImobileSdkAd.setImobileSdkAdListener(spotId, getSpotListener(spotId));

				// 広告の取得を開始
				ImobileSdkAd.start(spotId);

				// 広告を表示するためのレイアウトを作成
				adView = new RelativeLayout(activity);
				adView.setId(adViewid);
				adView.setPadding(convertDpiToPixel(activity, left), convertDpiToPixel(activity, top), 0, 0);
				activity.addContentView(adView, new RelativeLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));

				// 表示パラメータの設定
				ImobileIconParams params = new ImobileIconParams();

				// アイコン数
				if (iconNumber != 0) {
					params.setIconNumber(iconNumber);
				}
				// アイコン広告表示レイアウトサイズ
				if (iconViewLayoutWidth > 0) {
					params.setIconViewLayoutWidth(iconViewLayoutWidth);
				}
			    // アイコン広告サイズ
				if (iconSize > 0) {
					params.setIconSize(iconSize);
				}
				// アイコンタイトル有無
				params.setIconTitleEnable(iconTitleEnable);
			    // アイコンタイトルフォントサイズ
				if (iconTitleFontSize > 0) {
					params.setIconTitleFontSize(iconTitleFontSize);
				}
				// アイコンタイトルフォントカラー
				if (!iconTitleShadowColor.equals("#FFFFFF")) {
					params.setIconTitleFontColor(iconTitleFontColor);
				}
			    // アイコン広告画像とタイトルの間隔
				if (iconTitleOffset > 0) {
					params.setIconTitleOffset(iconTitleOffset);
				}
				// アイコン広告タイトルの影付/影
				params.setIconTitleShadowEnable(iconTitleShadowEnable);
				// アイコン広告タイトルの影の色
				if (!iconTitleShadowColor.equals("#000000")) {
					params.setIconTitleShadowColor(iconTitleShadowColor);
				}
				// タイトル文字影付き 影の位置 X
				if (iconTitleShadowDx != 0) {
					params.setIconTitleShadowDx(iconTitleShadowDx);
				}
				// タイトル文字影付き 影の位置 Y
				if (iconTitleShadowDy != 0) {
					params.setIconTitleShadowDy(iconTitleShadowDy);
				}

				// 広告の表示
				if (isIcon) {
					ImobileSdkAd.showAd(activity, spotId, adView, params);
				} else {
					ImobileSdkAd.showAd(activity, spotId, adView, sizeAdjust);
				}
			}
		});
	}

	/* 広告の表示・非表示を設定します */
	public static void setVisibility(final int adViewId, final boolean visible){
		final Activity activity = getCurrentActivity();
		activity.runOnUiThread(new Runnable(){
			public void run(){
				ViewGroup adView = (ViewGroup)activity.findViewById(adViewId);
				if(adView != null){
					adView.setVisibility( visible ? View.VISIBLE : View.GONE);
				}
			}
		});
	}
	
	/* 指定されたスポットの広告の取得を停止します */
	public static void stop(final String spotId) {
		final Activity activity = getCurrentActivity();
		activity.runOnUiThread(new Runnable() {
			public void run() {
				ImobileSdkAd.stop(spotId);
			}
		});
	}

	/* 全ての登録済みスポットの広告の取得を開始します */
	public static void startAll() {
		final Activity activity = getCurrentActivity();
		activity.runOnUiThread(new Runnable() {
			public void run() {
				ImobileSdkAd.startAll();
			}
		});
	}
	
	/* 全ての登録済みスポットの広告の取得を停止します */
	public static void stopAll() {
		final Activity activity = getCurrentActivity();
		activity.runOnUiThread(new Runnable() {
			public void run() {
				ImobileSdkAd.stopAll();
			}
		});
	}
	
	/* テストモードの設定を行います */
	public static void setTestMode(final boolean testFlg){
		final Activity activity = getCurrentActivity();
		activity.runOnUiThread(new Runnable() {
			public void run() {
				ImobileSdkAd.setTestMode(testFlg);
			}
		});
	}
	/* 広告の表示・非表示の向きを設定します */
	public static void setAdOrientation(final int adOrientationNumber) {
		final Activity activity = getCurrentActivity();
		activity.runOnUiThread(new Runnable(){
			public void run(){
				switch (adOrientationNumber) {
					case 0:
						ImobileSdkAd.setAdOrientation(AdOrientation.AUTO);
						break;
					case 1:
						ImobileSdkAd.setAdOrientation(AdOrientation.PORTRAIT);
						break;
					case 2:
						ImobileSdkAd.setAdOrientation(AdOrientation.LANDSCAPE);
						break;
				}
			}
		});
	}

	/* DeviceのDensity値を取得します */
	public static float getDeviceDensity() {
		DisplayMetrics metrics = new DisplayMetrics();
		getCurrentActivity().getWindowManager().getDefaultDisplay().getMetrics(metrics);
		return metrics.density;
	}
}
