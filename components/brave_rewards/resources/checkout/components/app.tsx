/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { LocaleContext } from '../../ui/components/checkout/localeContext'
import { DialogFrame } from '../../ui/components/checkout/dialogFrame'
import { EnableRewardsPanel } from '../../ui/components/checkout/enableRewardsPanel'
import { PaymentMethodPanel } from '../../ui/components/checkout/paymentMethodPanel'
import { LoadingPanel } from '../../ui/components/checkout/loadingPanel'
import { AddFundsPanel } from '../../ui/components/checkout/addFundsPanel'
import { PaymentProcessing } from '../../ui/components/checkout/paymentProcessing'
import { PaymentComplete } from '../../ui/components/checkout/paymentComplete'

import {
  WalletInfo,
  OrderInfo,
  ExchangeRateInfo,
  Settings,
  Host
} from '../interfaces'

import {
  createExchangeFormatter,
  formatLastUpdatedDate,
  formatTokenValue
} from '../formatting'

type FlowState =
  'start' |
  'add-funds' |
  'payment-processing' |
  'payment-complete'

interface AppProps {
  host: Host
  exchangeCurrency: string
}

export function App (props: AppProps) {
  const locale = React.useContext(LocaleContext)

  const [flowState, setFlowState] = React.useState<FlowState>('start')
  const [rateInfo, setRateInfo] = React.useState<ExchangeRateInfo | undefined>()
  const [walletInfo, setWalletInfo] = React.useState<WalletInfo | undefined>()
  const [orderInfo, setOrderInfo] = React.useState<OrderInfo | undefined>()
  const [settings, setSettings] = React.useState<Settings | undefined>()

  React.useEffect(() => {
    return props.host.addListener((state) => {
      setRateInfo(state.exchangeRateInfo)
      setOrderInfo(state.orderInfo)
      setWalletInfo(state.walletInfo)
      setSettings(state.settings)
    })
  }, [props.host])

  const onClose = () => { props.host.closeDialog() }

  if (
    !settings ||
    !rateInfo ||
    !orderInfo ||
    !walletInfo ||
    walletInfo.state === 'creating'
  ) {
    const loadingText = walletInfo && walletInfo.state === 'creating'
      ? locale.get('creatingWallet')
      : ''

    return (
      <DialogFrame showTitle={true} showBackground={false} onClose={onClose}>
        <LoadingPanel text={loadingText} />
      </DialogFrame>
    )
  }

  if (!settings.rewardsEnabled) {
    return (
      <DialogFrame showTitle={false} showBackground={true} onClose={onClose}>
        <EnableRewardsPanel onEnableRewards={props.host.enableRewards} />
      </DialogFrame>
    )
  }

  const showTitle =
    flowState !== 'payment-complete'

  const showBackground =
    flowState !== 'payment-complete' &&
    flowState !== 'payment-processing'

  const onShowAddFunds = () => setFlowState('add-funds')
  const onCancelAddFunds = () => setFlowState('start')

  const formatExchange = createExchangeFormatter(
    rateInfo.rates,
    props.exchangeCurrency)

  const amountNeeded = Math.max(0, orderInfo.total - walletInfo.balance)

  function getAddFundsAmounts () {
    // TODO(zenparsing): Calculate three options
    return []
  }

  return (
    <DialogFrame
      showTitle={showTitle}
      showBackground={showBackground}
      onClose={onClose}
    >
      {
        flowState === 'start' ?
          <PaymentMethodPanel
            rewardsEnabled={settings.rewardsEnabled}
            orderDescription={orderInfo.description}
            orderTotal={formatTokenValue(orderInfo.total)}
            orderTotalConverted={formatExchange(orderInfo.total)}
            walletBalance={formatTokenValue(walletInfo.balance)}
            walletBalanceConverted={formatExchange(walletInfo.balance)}
            walletLastUpdated={formatLastUpdatedDate(rateInfo.lastUpdated)}
            walletVerified={walletInfo.state === 'verified'}
            hasSufficientFunds={amountNeeded <= 0}
            onPayWithCreditCard={props.host.payWithCreditCard}
            onPayWithWallet={props.host.payWithWallet}
            onShowAddFunds={onShowAddFunds}
          /> :
        flowState === 'add-funds' ?
          <AddFundsPanel
            amountNeeded={formatTokenValue(amountNeeded)}
            walletBalance={formatTokenValue(walletInfo.balance)}
            walletBalanceConverted={formatExchange(walletInfo.balance)}
            unitValueConverted={formatExchange(1)}
            amountOptions={getAddFundsAmounts()}
            onCancel={onCancelAddFunds}
            onPayWithCreditCard={props.host.payWithCreditCard}
          /> :
        flowState === 'payment-processing' ?
          <PaymentProcessing /> :
        flowState === 'payment-complete' ?
          <PaymentComplete /> : ''
      }
    </DialogFrame>
  )
}
