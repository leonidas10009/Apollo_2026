import { flushPromises } from '@vue/test-utils'
import { beforeEach, describe, expect, it, vi } from 'vitest'

vi.mock('../../src_assets/common/assets/web/Navbar.vue', () => ({
  default: { template: '<div />' },
}))
vi.mock('../../src_assets/common/assets/web/fetch_utils', () => ({
  apiFetch: vi.fn(() => Promise.resolve({ status: 200, ok: true })),
}))

import { apiFetch } from '../../src_assets/common/assets/web/fetch_utils'
import Apps from '../../src_assets/common/assets/web/Apps.vue'

beforeEach(() => vi.clearAllMocks())

describe('application list actions', () => {
  it('keeps the original application index when sorting and filtering', () => {
    const context = {
      apps: [{ name: 'Zulu' }, { name: 'Alpha' }, { name: 'Bravo' }],
      searchQuery: '',
      sortMode: 'asc',
    }

    const sorted = Apps.computed.displayedApps.call(context)
    expect(sorted.map(item => item.app.name)).toEqual(['Alpha', 'Bravo', 'Zulu'])
    expect(sorted.map(item => item.index)).toEqual([1, 2, 0])

    context.searchQuery = 'bravo'
    const filtered = Apps.computed.displayedApps.call(context)
    expect(filtered).toHaveLength(1)
    expect(filtered[0]).toMatchObject({ app: { name: 'Bravo' }, index: 2 })
  })

  it('deletes by original index and refreshes without reloading the page', async () => {
    const context = {
      deleteTarget: { index: 2, name: 'Example' },
      closeDeleteModal: vi.fn(),
      loadApps: vi.fn(() => Promise.resolve()),
      $t: key => key,
    }

    Apps.methods.confirmDelete.call(context)
    await flushPromises()

    expect(apiFetch).toHaveBeenCalledWith('./api/apps/2', expect.objectContaining({ method: 'DELETE' }))
    expect(context.closeDeleteModal).toHaveBeenCalledOnce()
    expect(context.loadApps).toHaveBeenCalledOnce()
  })

  it('reports a failed deletion instead of refreshing the list', async () => {
    apiFetch.mockResolvedValueOnce({ ok: false, status: 500 })
    const alert = vi.fn()
    vi.stubGlobal('alert', alert)

    const context = {
      deleteTarget: { index: 4, name: 'Example' },
      closeDeleteModal: vi.fn(),
      loadApps: vi.fn(() => Promise.resolve()),
      $t: key => key,
    }

    Apps.methods.confirmDelete.call(context)
    await flushPromises()

    expect(context.closeDeleteModal).not.toHaveBeenCalled()
    expect(context.loadApps).not.toHaveBeenCalled()
    expect(alert).toHaveBeenCalled()
    vi.unstubAllGlobals()
  })
})
