import { beforeEach, describe, expect, it, vi } from 'vitest'

vi.mock('../../src_assets/common/assets/web/fetch_utils', () => ({
  apiFetch: vi.fn(() => Promise.resolve({})),
}))

import { apiFetch } from '../../src_assets/common/assets/web/fetch_utils'
import Clients from '../../src_assets/common/assets/web/Clients.vue'

beforeEach(() => vi.clearAllMocks())

describe('client actions', () => {
  it('unpairs a single client by UUID and refreshes the list', async () => {
    const context = {
      refreshClients: vi.fn(),
    }

    await Clients.methods.unpairSingle.call(context, 'client-deck')

    expect(apiFetch).toHaveBeenCalledWith('./api/clients/unpair', expect.objectContaining({
      method: 'POST',
      body: JSON.stringify({ uuid: 'client-deck' }),
    }))
    expect(context.refreshClients).toHaveBeenCalledOnce()
  })

  it('disconnects a client by UUID and refreshes the list', async () => {
    const context = {
      refreshClients: vi.fn(),
    }

    await Clients.methods.disconnectClient.call(context, 'client-tv')

    expect(apiFetch).toHaveBeenCalledWith('./api/clients/disconnect', expect.objectContaining({
      method: 'POST',
      body: JSON.stringify({ uuid: 'client-tv' }),
    }))
    expect(context.refreshClients).toHaveBeenCalledOnce()
  })
})
