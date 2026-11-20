import { describe, expect, it } from 'vitest'

import Pin from '../../src_assets/common/assets/web/Pin.vue'

describe('pin pairing helpers', () => {
  it('builds an encoded Artemis deep link', () => {
    const link = Pin.computed.deepLink.call({
      hostAddr: '192.168.1.10',
      hostPort: '47989',
      otp: '1234',
      passphrase: 'a b&c',
      hostName: 'My PC',
    })

    expect(link).toBe('art://192.168.1.10:47989?pin=1234&passphrase=a%20b%26c&name=My%20PC')
  })

  it('only accepts a host and a valid TCP port', () => {
    expect(Pin.computed.canSaveHost.call({ hostAddr: 'host', hostPort: '47989' })).toBe(true)
    expect(Pin.computed.canSaveHost.call({ hostAddr: '', hostPort: '47989' })).toBe(false)
    expect(Pin.computed.canSaveHost.call({ hostAddr: 'host', hostPort: '0' })).toBe(false)
    expect(Pin.computed.canSaveHost.call({ hostAddr: 'host', hostPort: '65536' })).toBe(false)
    expect(Pin.computed.canSaveHost.call({ hostAddr: 'host', hostPort: 'not-a-port' })).toBe(false)
  })

  it('stores the manually confirmed host for the browser session', () => {
    sessionStorage.clear()
    const context = { hostAddr: '10.0.0.5', hostPort: '47989' }

    Pin.methods.writeHostCache.call(context)

    expect(JSON.parse(sessionStorage.getItem('hostInfo'))).toEqual({
      hostAddr: '10.0.0.5',
      hostPort: '47989',
    })
    expect(Pin.methods.readHostCache.call({})).toEqual({
      hostAddr: '10.0.0.5',
      hostPort: '47989',
    })
  })

  it('ignores invalid cached host data', () => {
    sessionStorage.setItem('hostInfo', '{"hostAddr":""}')
    expect(Pin.methods.readHostCache.call({})).toBeNull()

    sessionStorage.setItem('hostInfo', 'not json')
    expect(Pin.methods.readHostCache.call({})).toBeNull()
  })
})
