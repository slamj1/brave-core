/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/renderer/brave_content_settings_agent_impl_farbling_helper.h"

#define BRAVE_TO_DATA_URL_INTERNAL                                 \
  switch (GetBraveFarblingLevel(GetDocument().GetFrame())) {       \
    case BraveFarblingLevel::OFF:                                  \
      break;                                                       \
    case BraveFarblingLevel::BALANCED: {                           \
      image_bitmap = brave::BraveSessionCache::From(GetDocument()) \
                         .PerturbPixels(image_bitmap);             \
      break;                                                       \
    }                                                              \
    case BraveFarblingLevel::MAXIMUM:                              \
      break;                                                       \
    default:                                                       \
      NOTREACHED();                                                \
  }

#include "../../../../../../../third_party/blink/renderer/core/html/canvas/html_canvas_element.cc"

#undef BRAVE_TO_DATA_URL_INTERNAL
