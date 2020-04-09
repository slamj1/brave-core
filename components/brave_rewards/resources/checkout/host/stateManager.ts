/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

type Listener<State> = (state: State) => void

export function createStateManager<State> (initialState: State) {
  const listeners = new Set<Listener<State>>()
  const state = { ...initialState }

  function update (source: Partial<State>) {
    Object.assign(state, source)
    for (const listener of listeners) {
      listener(state) // TODO(zenparsing): Error handling
    }
  }

  function addListener (listener: Listener<State>) {
    if (!listeners.has(listener)) {
      listeners.add(listener)
      listener(state)
    }
    return () => { listeners.delete(listener) }
  }

  return { state, update, addListener }
}
