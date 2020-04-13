/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

export type WalletState =
  'not-created' |
  'creating' |
  'create-failed' |
  'corrupted' |
  'created' |
  'verified'

export interface WalletInfo {
  state: WalletState
  balance: number
}

export interface ExchangeRateInfo {
  rates: Record<string, number>
  lastUpdated: string
}

export interface OrderInfo {
  description: string
  total: number
}

export interface Settings {
  rewardsEnabled: boolean
}

export interface HostState {
  walletInfo?: WalletInfo
  exchangeRateInfo?: ExchangeRateInfo
  orderInfo?: OrderInfo
  settings?: Settings
}

export type HostListener = (state: HostState) => void

export interface Host {
  getLocaleString: (key: string) => string
  closeDialog: () => void
  payWithCreditCard: () => void
  payWithWallet: () => void
  enableRewards: () => void
  addListener: (callback: HostListener) => () => void
}
