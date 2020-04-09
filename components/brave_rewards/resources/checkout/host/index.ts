/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { createStateManager } from './stateManager'

import {
  WalletInfo,
  WalletState,
  OrderInfo,
  Host,
  HostState
} from '../interfaces'

function parseDialogArgs (): OrderInfo {
  // TODO(zenparsing): Error handling?
  const argString = chrome.getVariableValue('dialogArguments')
  const { orderInfo } = Object(JSON.parse(argString))
  return {
    description: orderInfo.description,
    total: orderInfo.total
  }
}

// User wallet data is collated using data from multiple calls
// to the rewards service. WalletCollator is used to aggregate
// this data and provide a WalletInfo object when all data is
// available.
function createWalletCollator () {
  const needed = new Set(['balance', 'verified', 'status'])

  const walletInfo: WalletInfo = {
    balance: 0,
    state: 'not-created'
  }

  function update (key: string, fn: () => void): WalletInfo | undefined {
    needed.delete(key)
    fn()
    return needed.size === 0
      ? { ...walletInfo }
      : undefined
  }

  return {

    setBalance (balance: number) {
      return update('balance', () => {
        walletInfo.balance = balance
      })
    },

    resetBalance () {
      needed.add('balance')
    },

    setVerified (verified: boolean) {
      return update('verified', () => {
        if (verified) {
          walletInfo.state = 'verified'
        }
      })
    },

    setStatus (status: WalletState) {
      return update('status', () => {
        walletInfo.state = status
      })
    }

  }
}

function addWebUIListeners (listeners: object) {
  for (const key of Object.keys(listeners)) {
    self.cr.addWebUIListener(key, (event: any) => {
      // TODO(zenparsing): Disable logging in production?
      console.log(key, event)
      listeners[key](event)
    })
  }
}

export function createHost (): Host {
  const stateManager = createStateManager<HostState>({
    orderInfo: parseDialogArgs()
  })

  const walletCollator = createWalletCollator()

  addWebUIListeners({

    walletBalanceUpdated (event: any) {
      const { details } = event
      const total = details ? details.total as number : 0
      // TODO(zenparsing): This isn't quite right
      const rates = details ? details.rates as Record<string, number> : {}
      // TODO(zenparsing): Get from service?
      const lastUpdated = new Date().toISOString()

      stateManager.update({
        walletInfo: walletCollator.setBalance(total),
        exchangeRateInfo: { rates, lastUpdated }
      })
    },

    anonWalletStatusUpdated (event: any) {
      // TODO(zenparsing): Handle "corrupted" (17) differently?
      // TODO(zenparsing): Using number literals is sad
      const status = event.status === 12 ? 'created' : 'not-created'
      stateManager.update({
        walletInfo: walletCollator.setStatus(status)
      })
    },

    externalWalletUpdated (event: any) {
      if (!event.details) {
        return
      }
      const verified = event.details.status === 1
      stateManager.update({
        walletInfo: walletCollator.setVerified(verified)
      })
    },

    rewardsEnabledUpdated (event: any) {
      stateManager.update({
        settings: { rewardsEnabled: event.rewardsEnabled }
      })
    },

    walletInitialized (event: any) {
      walletCollator.resetBalance()
      chrome.send('getWalletBalance')
      chrome.send('getAnonWalletStatus')
    }

  })

  chrome.send('getRewardsEnabled')
  chrome.send('getWalletBalance')
  chrome.send('getAnonWalletStatus')
  chrome.send('getExternalWallet')

  // TODO(zenparsing): Is this required?
  self.i18nTemplate.process(document, self.loadTimeData)

  return {

    getLocaleString (key: string) {
      return self.loadTimeData.getString(key)
    },

    closeDialog () {
      chrome.send('dialogClose')
    },

    enableRewards () {
      chrome.send('enableRewards')

      const { walletInfo } = stateManager.state
      const shouldCreate = walletInfo && walletInfo.state === 'not-created'

      if (shouldCreate) {
        chrome.send('createWallet')
        stateManager.update({
          walletInfo: walletCollator.setStatus('creating')
        })
      }
    },

    payWithCreditCard (...args) {
      console.log('payWithCreditCard', ...args)
      // TODO(zenparsing): Send update to service
    },

    payWithWallet (...args) {
      console.log('payWithWallet', ...args)
      // TODO(zenparsing): Send update to service
    },

    addListener: stateManager.addListener

  }
}
