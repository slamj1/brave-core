/** Copyright (c) 2019 The Brave Authors. All rights reserved.
  * This Source Code Form is subject to the terms of the Mozilla Public
  * License, v. 2.0. If a copy of the MPL was not distributed with this file,
  * You can obtain one at http://mozilla.org/MPL/2.0/.
  */

package org.chromium.chrome.browser;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

import org.chromium.chrome.browser.BraveActivity;
import org.chromium.chrome.browser.BraveRewardsExternalWallet;
import org.chromium.chrome.R;

public class BraveRewardsVerifyWalletActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.verify_wallet_activity);
        SetVerifyWalletBtnClickHandler();
    }

    void SetVerifyWalletBtnClickHandler () {
        Button btnVerifyWallet = (Button)findViewById(R.id.verify_wallet_btn);
        btnVerifyWallet.setOnClickListener( (View v) -> {
            String verify_url = getIntent().getStringExtra(BraveRewardsExternalWallet.VERIFY_URL);
            Intent intent = new Intent();
            intent.putExtra(BraveActivity.OPEN_URL, verify_url);
            setResult(RESULT_OK, intent);
            finish();
        });
    }
}
