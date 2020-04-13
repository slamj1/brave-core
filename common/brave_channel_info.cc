/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/common/brave_channel_info.h"

#include "chrome/common/channel_info.h"

namespace brave {

std::string GetChannelName() {
  std::string channel_name = chrome::GetChannelName();
  if (channel_name.empty()) {
    channel_name = "release";
  }

  return channel_name;
}

}  // namespace brave
