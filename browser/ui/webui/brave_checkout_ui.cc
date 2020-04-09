/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/brave_checkout_ui.h"

#include <memory>

#include "base/memory/weak_ptr.h"
#include "base/values.h"
#include "brave/browser/ui/webui/basic_ui.h"
#include "brave/components/brave_rewards/browser/rewards_service.h"
#include "brave/components/brave_rewards/browser/rewards_service_factory.h"
#include "brave/components/brave_rewards/browser/rewards_service_observer.h"
#include "brave/components/brave_rewards/resources/grit/brave_rewards_resources.h"
#include "brave/components/brave_rewards/resources/grit/brave_rewards_checkout_generated_map.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/public/browser/web_ui_message_handler.h"

namespace {

using brave_rewards::RewardsService;
using brave_rewards::RewardsServiceFactory;
using brave_rewards::RewardsServiceObserver;

class CheckoutMessageHandler : public content::WebUIMessageHandler,
                               public brave_rewards::RewardsServiceObserver {
 public:
  CheckoutMessageHandler();

  CheckoutMessageHandler(const CheckoutMessageHandler&) = delete;
  CheckoutMessageHandler& operator=(const CheckoutMessageHandler&) = delete;

  ~CheckoutMessageHandler() override;

  // WebUIMessageHandler
  void RegisterMessages() override;

  // RewardsServiceObserver
  void OnWalletInitialized(RewardsService* service, int32_t result) override;
  void OnRewardsMainEnabled(RewardsService* service, bool enabled) override;

 private:
  RewardsService* GetRewardsService();

  // Message handlers
  void OnGetWalletBalance(const base::ListValue* args);
  void OnGetAnonWalletStatus(const base::ListValue* args);
  void OnGetExternalWallet(const base::ListValue* args);
  void OnGetRewardsEnabled(const base::ListValue* args);
  void OnEnableRewards(const base::ListValue* args);
  void OnCreateWallet(const base::ListValue* args);

  // Rewards service callbacks
  void FetchBalanceCallback(
      int32_t status,
      std::unique_ptr<brave_rewards::Balance> balance);

  void GetExternalWalletCallback(
      int32_t status,
      std::unique_ptr<brave_rewards::ExternalWallet> wallet);

  void GetAnonWalletStatusCallback(uint32_t result);
  void GetRewardsMainEnabledCallback(bool enabled);
  void CreateWalletCallback(int32_t result);

  RewardsService* rewards_service_ = nullptr;  // NOT OWNED
  base::WeakPtrFactory<CheckoutMessageHandler> weak_factory_;
};

CheckoutMessageHandler::CheckoutMessageHandler() : weak_factory_(this) {}

CheckoutMessageHandler::~CheckoutMessageHandler() {
  if (rewards_service_) {
    rewards_service_->RemoveObserver(this);
  }
}

RewardsService* CheckoutMessageHandler::GetRewardsService() {
  if (!rewards_service_) {
    Profile* profile = Profile::FromWebUI(web_ui());
    rewards_service_ = RewardsServiceFactory::GetForProfile(profile);
    if (rewards_service_) {
      rewards_service_->AddObserver(this);
    }
  }
  return rewards_service_;
}

void CheckoutMessageHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "getWalletBalance",
      base::BindRepeating(&CheckoutMessageHandler::OnGetWalletBalance,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getAnonWalletStatus",
      base::BindRepeating(&CheckoutMessageHandler::OnGetAnonWalletStatus,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getExternalWallet",
      base::BindRepeating(&CheckoutMessageHandler::OnGetExternalWallet,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "getRewardsEnabled",
      base::BindRepeating(&CheckoutMessageHandler::OnGetRewardsEnabled,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "enableRewards",
      base::BindRepeating(&CheckoutMessageHandler::OnEnableRewards,
                          base::Unretained(this)));

  web_ui()->RegisterMessageCallback(
      "createWallet",
      base::BindRepeating(&CheckoutMessageHandler::OnCreateWallet,
                          base::Unretained(this)));
}

void CheckoutMessageHandler::OnWalletInitialized(
    RewardsService* service,
    int32_t result) {
  if (IsJavascriptAllowed()) {
    base::Value response(base::Value::Type::DICTIONARY);
    response.SetIntKey("status", result);
    FireWebUIListener("walletInitialized", std::move(response));
  }
}

void CheckoutMessageHandler::OnRewardsMainEnabled(
    RewardsService* service,
    bool enabled) {
  GetRewardsMainEnabledCallback(enabled);
}

void CheckoutMessageHandler::OnGetWalletBalance(
    const base::ListValue* args) {
  if (auto* service = GetRewardsService()) {
    AllowJavascript();
    service->FetchBalance(base::BindOnce(
        &CheckoutMessageHandler::FetchBalanceCallback,
        weak_factory_.GetWeakPtr()));
  }
}

void CheckoutMessageHandler::OnGetAnonWalletStatus(
    const base::ListValue* args) {
  if (auto* service = GetRewardsService()) {
    AllowJavascript();
    service->GetAnonWalletStatus(base::BindOnce(
        &CheckoutMessageHandler::GetAnonWalletStatusCallback,
        weak_factory_.GetWeakPtr()));
  }
}

void CheckoutMessageHandler::OnGetExternalWallet(
    const base::ListValue* args) {
  if (auto* service = GetRewardsService()) {
    AllowJavascript();
    service->GetExternalWallet(
        "uphold", // TODO(zenparsing): Take from params?
        base::BindOnce(
            &CheckoutMessageHandler::GetExternalWalletCallback,
            weak_factory_.GetWeakPtr()));
  }
}

void CheckoutMessageHandler::OnGetRewardsEnabled(
    const base::ListValue* args) {
  if (auto* service = GetRewardsService()) {
    AllowJavascript();
    service->GetRewardsMainEnabled(
        base::Bind(
            &CheckoutMessageHandler::GetRewardsMainEnabledCallback,
            weak_factory_.GetWeakPtr()));
  }
}

void CheckoutMessageHandler::OnEnableRewards(
    const base::ListValue* args) {
  if (auto* service = GetRewardsService()) {
    AllowJavascript();
    service->SetRewardsMainEnabled(1);
  }
}

void CheckoutMessageHandler::OnCreateWallet(
    const base::ListValue* args) {
  if (auto* service = GetRewardsService()) {
    AllowJavascript();
    service->CreateWallet(
        base::Bind(
            &CheckoutMessageHandler::CreateWalletCallback,
            weak_factory_.GetWeakPtr()));
  }
}

void CheckoutMessageHandler::FetchBalanceCallback(
    int32_t status,
    std::unique_ptr<brave_rewards::Balance> balance) {
  if (!IsJavascriptAllowed())
    return;

  base::Value response(base::Value::Type::DICTIONARY);
  response.SetIntKey("status", status);

  if (status == 0 && balance) {
    base::Value details(base::Value::Type::DICTIONARY);
    details.SetDoubleKey("total", balance->total);

    base::Value rates_dict(base::Value::Type::DICTIONARY);
    for (auto& pair : balance->rates) {
      rates_dict.SetDoubleKey(pair.first, pair.second);
    }
    details.SetKey("rates", std::move(rates_dict));

    response.SetKey("details", std::move(details));
  }

  FireWebUIListener("walletBalanceUpdated", response);
}

void CheckoutMessageHandler::GetAnonWalletStatusCallback(
    uint32_t status) {
  if (!IsJavascriptAllowed())
    return;

  base::Value response(base::Value::Type::DICTIONARY);
  response.SetIntKey("status", status);
  FireWebUIListener("anonWalletStatusUpdated", response);
}

void CheckoutMessageHandler::GetExternalWalletCallback(
    int32_t status,
    std::unique_ptr<brave_rewards::ExternalWallet> wallet) {
  if (!IsJavascriptAllowed())
    return;

  base::Value response(base::Value::Type::DICTIONARY);
  response.SetIntKey("status", status);

  if (status == 0 && wallet) {
    // TODO(zenparsing): Do we need all of this?
    base::Value details(base::Value::Type::DICTIONARY);
    details.SetStringKey("token", wallet->token);
    details.SetStringKey("address", wallet->address);
    // TODO(zenparsing): Would prefer to return string enum rather than int
    details.SetIntKey("status", static_cast<int>(wallet->status));
    details.SetStringKey("type", wallet->type);
    details.SetStringKey("verifyUrl", wallet->verify_url);
    details.SetStringKey("addUrl", wallet->add_url);
    details.SetStringKey("withdrawUrl", wallet->withdraw_url);
    details.SetStringKey("userName", wallet->user_name);
    details.SetStringKey("accountUrl", wallet->account_url);

    response.SetKey("details", std::move(details));
  }

  FireWebUIListener("externalWalletUpdated", response);
}

void CheckoutMessageHandler::GetRewardsMainEnabledCallback(bool enabled) {
  if (!IsJavascriptAllowed())
    return;

  base::Value response(base::Value::Type::DICTIONARY);
  response.SetBoolKey("rewardsEnabled", enabled);
  FireWebUIListener("rewardsEnabledUpdated", response);
}

void CheckoutMessageHandler::CreateWalletCallback(int32_t result) {
  // Empty
  // TODO(zenparsing): Is this handled by WalletInitialized?
}


}  // namespace

BraveCheckoutUI::BraveCheckoutUI(
    content::WebUI* web_ui,
    const std::string& name)
    : ConstrainedWebDialogUI(web_ui) {
  // TODO(zenparsing): Handle profile->IsOffTheRecord()?
  Profile* profile = Profile::FromWebUI(web_ui);

  content::WebUIDataSource::Add(profile, CreateBasicUIHTMLSource(
      profile,
      name,
      kBraveRewardsCheckoutGenerated,
      kBraveRewardsCheckoutGeneratedSize,
      IDR_BRAVE_REWARDS_CHECKOUT_HTML));

  web_ui->AddMessageHandler(std::make_unique<CheckoutMessageHandler>());
}

BraveCheckoutUI::~BraveCheckoutUI() {}
