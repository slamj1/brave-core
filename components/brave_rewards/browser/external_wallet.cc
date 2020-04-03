/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/values.h"
#include "base/json/json_writer.h"
#include "brave/components/brave_rewards/browser/external_wallet.h"

namespace brave_rewards {

  ExternalWallet::ExternalWallet() :
      status(0) {
  }

  ExternalWallet::~ExternalWallet() { }

  ExternalWallet::ExternalWallet(const ExternalWallet &properties) {
    token = properties.token;
    address = properties.address;
    status = properties.status;
    type = properties.type;
    verify_url = properties.verify_url;
    add_url = properties.add_url;
    withdraw_url = properties.withdraw_url;
    user_name = properties.user_name;
    account_url = properties.account_url;
  }

  std::string ExternalWallet::toJson() {
    std::string json_wallet;
    base::Value dict(base::Value::Type::DICTIONARY);
    dict.SetKey("token", base::Value(token));
    dict.SetKey("address", base::Value(address));
    dict.SetKey("status", base::Value((int32_t)(status)));  //enum class WalletStatus : int32_t
    dict.SetKey("type", base::Value(type));
    dict.SetKey("verify_url", base::Value(verify_url));
    dict.SetKey("add_url", base::Value(add_url));
    dict.SetKey("withdraw_url", base::Value(withdraw_url));
    dict.SetKey("user_name", base::Value(user_name));
    dict.SetKey("account_url", base::Value(account_url));
    base::JSONWriter::Write(dict, &json_wallet);
    return json_wallet;
  }
}  // namespace brave_rewards
