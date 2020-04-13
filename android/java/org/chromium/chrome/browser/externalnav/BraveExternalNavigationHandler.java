/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.externalnav;

import org.chromium.chrome.browser.externalnav.ExternalNavigationHandler;
import org.chromium.chrome.browser.externalnav.ExternalNavigationHandler.OverrideUrlLoadingResult;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.BraveUphold;

public class BraveExternalNavigationHandler extends ExternalNavigationHandler {
    private BraveUphold mBraveUphold;
    public BraveExternalNavigationHandler(Tab tab) {
        super(tab);
    }

    public BraveExternalNavigationHandler(ExternalNavigationDelegate delegate) {
        super(delegate);
    }

    @Override
    public @OverrideUrlLoadingResult int shouldOverrideUrlLoading(ExternalNavigationParams params) {
        if (isUpholdOverride(params)) {
            CompleteUpholdVerification(params);
            return OverrideUrlLoadingResult.OVERRIDE_WITH_CLOBBERING_TAB;
        }
        return super.shouldOverrideUrlLoading(params);
    }

    private boolean isUpholdOverride(ExternalNavigationParams params) {
        if (!params.getUrl().startsWith(BraveUphold.UPHOLD_REDIRECT_URL)) return false;
        return true;
    }

    private void CompleteUpholdVerification(ExternalNavigationParams params) {
        mBraveUphold = new BraveUphold();
        mBraveUphold.CompleteUpholdVerification(params, this);
    }
}