/** Copyright (c) 2019 The Brave Authors. All rights reserved.
  * This Source Code Form is subject to the terms of the Mozilla Public
  * License, v. 2.0. If a copy of the MPL was not distributed with this file,
  * You can obtain one at http://mozilla.org/MPL/2.0/.
  */
package org.chromium.chrome.browser;

import android.net.Uri;
import android.text.TextUtils;
import java.util.Locale;

import org.chromium.chrome.browser.BraveRewardsNativeWorker;
import org.chromium.chrome.browser.BraveRewardsObserver;
import org.chromium.chrome.browser.externalnav.BraveExternalNavigationHandler;
import org.chromium.chrome.browser.externalnav.ExternalNavigationParams;

import org.json.JSONException;
import org.json.JSONObject;

//used from org.chromium.chrome.browser.externalnav
public class BraveUphold implements BraveRewardsObserver {
    public static final String WALLET_UPHOLD = "uphold";
    public static final String UPHOLD_REDIRECT_URL = "rewards://uphold";
    public static final String UPHOLD_REDIRECT_URL_KEY = "redirect_url";
    public static final String ACTION_VALUE = "authorization";
    public static final String UPHOLD_SUPPORT_URL = "http://uphold.com/en/brave/support";

    private ExternalNavigationParams mExternalNavigationParams;
    private BraveExternalNavigationHandler mBraveExternalNavigationHandler;
    private BraveRewardsNativeWorker rewardsNativeProxy = BraveRewardsNativeWorker.getInstance();

    public void CompleteUpholdVerification(ExternalNavigationParams params,
            BraveExternalNavigationHandler handler) {
        mExternalNavigationParams = params;
        mBraveExternalNavigationHandler = handler;

        Uri uri = Uri.parse(params.getUrl());
        rewardsNativeProxy.AddObserver(this);
        String path = uri.getPath();
        String query = uri.getQuery();

        if (TextUtils.isEmpty(path) || TextUtils.isEmpty(query)) {
            ReleaseComponents();
            ShowGenericUpholdverificationError();
            return;
        }

        // Ledger expects:
        // path: "/uphold/path"
        // query: "?query"
        path = String.format(Locale.US,"/%s/%s", WALLET_UPHOLD, path);
        query = String.format(Locale.US,"?%s", query);
        rewardsNativeProxy.ProcessRewardsPageUrl(path, query);
    }

    @Override
    public void OnProcessRewardsPageUrl(int error_code,
            String wallet_type, String action,
            String json_args ) {

        String redirect_url = parseJsonArgs (json_args);
        if (BraveRewardsNativeWorker.LEDGER_OK == error_code) {
            if (TextUtils.equals(action, ACTION_VALUE) &&
                    !TextUtils.isEmpty(redirect_url)) {
                mBraveExternalNavigationHandler.
                        clobberCurrentTabWithFallbackUrl (redirect_url,
                        mExternalNavigationParams);
            }
        }
        else if (BraveRewardsNativeWorker.BAT_NOT_ALLOWED == error_code) {
          //open UPHOLD_SUPPORT_URL
          mBraveExternalNavigationHandler.
                  clobberCurrentTabWithFallbackUrl (UPHOLD_SUPPORT_URL,
                  mExternalNavigationParams);
        }
        else {
          ShowGenericUpholdverificationError();
        }
        ReleaseComponents();
    }

    private String parseJsonArgs(String json_args) {
        String redirect_url = "";
        try {
            JSONObject jsonObj = new JSONObject(json_args);
            if (jsonObj.has(UPHOLD_REDIRECT_URL_KEY)) {
                redirect_url = jsonObj.getString(UPHOLD_REDIRECT_URL_KEY);
            }
        }
        catch(JSONException e){ }
        return redirect_url;
    }

    private void ReleaseComponents() {
        //remove observer
        if (rewardsNativeProxy != null) {
            rewardsNativeProxy.RemoveObserver(this);
        }

        mExternalNavigationParams = null;
        mBraveExternalNavigationHandler = null;
    }

    private void ShowGenericUpholdverificationError () {

    }
}