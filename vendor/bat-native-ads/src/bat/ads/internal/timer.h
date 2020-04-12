/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_TIMER_H_
#define BAT_ADS_INTERNAL_TIMER_H_

#include <stdint.h>

#include "base/bind.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

namespace ads {

class Timer {
 public:
  Timer();

  ~Timer();

  // Start a timer to run at the given |delay| from now. If the timer is already
  // running, it will be replaced to call the given |user_task|. Returns the
  // time the delayed task will be fired
  base::Time Start(
      const uint64_t delay,
      base::OnceClosure user_task);

  // Start a timer to run at a geometrically distributed number of seconds
  // |~delay| from now for privacy-focused events. If the timer is already
  // running, it will be replaced to call the given |user_task|. Returns the
  // time the delayed task will be fired
  base::Time StartWithPrivacy(
      const uint64_t delay,
      base::OnceClosure user_task);

  // Returns true if the timer is running (i.e., not stopped)
  bool IsRunning() const;

  // Call this method to stop the timer. It is a no-op if the timer is not
  // running
  void Stop();

 private:
  base::OneShotTimer timer_;
};

}  // namespace ads

#endif  // BAT_ADS_INTERNAL_TIMER_H_
