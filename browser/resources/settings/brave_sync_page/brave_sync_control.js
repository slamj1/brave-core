// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
/**
 * @fileoverview
 * 'settings-sync-account-section' is the settings page containing sign-in
 * settings.
 */
cr.exportPath('settings');

/** @const {number} */
settings.MAX_SIGNIN_PROMO_IMPRESSION = 10;

Polymer({
  is: 'settings-brave-sync-control',

  behaviors: [
    WebUIListenerBehavior,
    PrefsBehavior,
  ],

  properties: {
    /**
     * Preferences state.
     */
    prefs: {
      type: Object,
      notify: true,
    },

    /**
     * The current sync status, supplied by parent element.
     * @type {!settings.SyncStatus}
     */
    syncStatus: Object,


    // This property should be set by the parent only and should not change
    // after the element is created.
    embeddedInSubpage: {
      type: Boolean,
      reflectToAttribute: true,
    },

    // This property should be set by the parent only and should not change
    // after the element is created.
    hideButtons: {
      type: Boolean,
      value: false,
      reflectToAttribute: true,
    },

  },

  /**
   * Returns the class of the sync icon.
   * @return {string}
   * @private
   */
  getSyncIconStyle_: function() {
    if (this.syncStatus.disabled) {
      return 'sync-disabled';
    }
    if (!this.syncStatus.hasError) {
      return 'sync';
    }
    // Specific error cases below.
    if (this.syncStatus.hasUnrecoverableError) {
      return 'sync-problem';
    }
    if (this.syncStatus.statusAction == settings.StatusAction.REAUTHENTICATE) {
      return 'sync-paused';
    }
    return 'sync-problem';
  },

  /**
   * Returned value must match one of iron-icon's settings:(*) icon name.
   * @return {string}
   * @private
   */
  getSyncIcon_: function() {
    switch (this.getSyncIconStyle_()) {
      case 'sync-problem':
        return 'settings:sync-problem';
      case 'sync-paused':
        return 'settings:sync-disabled';
      default:
        return 'cr:sync';
    }
  },

  /**
   * Determines if the sync button should be disabled in response to
   * either a first setup flow or chrome sign-in being disabled.
   * @return {boolean}
   * @private
   */
  shouldDisableSyncButton_: function() {
    if (this.hideButtons ||
        !loadTimeData.getBoolean('privacySettingsRedesignEnabled')) {
      // Maintain existing behaviour if hidden or flag disabled
      return this.computeShowSetupButtons_();
    }
    return !!this.syncStatus.firstSetupInProgress ||
        !this.getPref('signin.allowed_on_next_startup').value;
  },

  /**
   * @return {boolean}
   * @private
   */
  shouldShowTurnOffButton_: function() {
    return !this.hideButtons && !this.showSetupButtons_ &&
        !!this.syncStatus.signedIn;
  },

  /**
   * @return {boolean}
   * @private
   */
  shouldShowErrorActionButton_: function() {
    if (this.embeddedInSubpage &&
        this.syncStatus.statusAction ==
            settings.StatusAction.ENTER_PASSPHRASE) {
      // In a subpage the passphrase button is not required.
      return false;
    }
    return !this.hideButtons && !this.showSetupButtons_ &&
        !!this.syncStatus.signedIn && !!this.syncStatus.hasError &&
        this.syncStatus.statusAction != settings.StatusAction.NO_ACTION;
  },


  /** @private */
  onErrorButtonTap_: function() {
    switch (this.syncStatus.statusAction) {
      case settings.StatusAction.REAUTHENTICATE:
        this.syncBrowserProxy_.startSignIn();
        break;
      case settings.StatusAction.SIGNOUT_AND_SIGNIN:
        if (this.syncStatus.domain) {
          settings.navigateTo(settings.routes.SIGN_OUT);
        } else {
          // Silently sign the user out without deleting their profile and
          // prompt them to sign back in.
          this.syncBrowserProxy_.signOut(false);
          this.syncBrowserProxy_.startSignIn();
        }
        break;
      case settings.StatusAction.UPGRADE_CLIENT:
        settings.navigateTo(settings.routes.ABOUT);
        break;
      case settings.StatusAction.RETRIEVE_TRUSTED_VAULT_KEYS:
        this.syncBrowserProxy_.startKeyRetrieval();
        break;
      case settings.StatusAction.ENTER_PASSPHRASE:
      case settings.StatusAction.CONFIRM_SYNC_SETTINGS:
      default:
        settings.navigateTo(settings.routes.SYNC);
    }
  },

  /**
   * @return {boolean}
   * @private
   */
  computeShowSetupButtons_: function() {
    console.error(this.syncStatus);
    return true;
    return !this.hideButtons && !!this.syncStatus.firstSetupInProgress;
  },

  /** @private */
  onSetupCancel_: function() {
    this.fire('sync-setup-done', false);
  },

  /** @private */
  onSetupConfirm_: function() {
    this.fire('sync-setup-done', true);
  },
});
