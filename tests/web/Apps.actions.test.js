import { flushPromises } from '@vue/test-utils'
import { beforeEach, describe, expect, it, vi } from 'vitest'

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

  it('moves a dragged application to its drop position', () => {
    const context = {
      apps: [{ name: 'Alpha' }, { name: 'Bravo' }, { name: 'Charlie' }, { name: 'Delta' }],
      canReorder: true,
      draggingApp: 0,
      dragOverIndex: 0,
      listReordered: false,
      onDragEnd: Apps.methods.onDragEnd,
    }

    Apps.methods.onDrop.call(context, { preventDefault: vi.fn() }, 3)

    expect(context.apps.map(app => app.name)).toEqual(['Bravo', 'Charlie', 'Alpha', 'Delta'])
    expect(context.listReordered).toBe(true)
    expect(context.draggingApp).toBe(-1)
  })

  it('appends a dragged application with the end drop target', () => {
    const context = {
      apps: [{ name: 'Alpha' }, { name: 'Bravo' }, { name: 'Charlie' }],
      canReorder: true,
      draggingApp: 0,
      dragOverIndex: 0,
      listReordered: false,
      onDragEnd: Apps.methods.onDragEnd,
    }

    Apps.methods.onDropAtEnd.call(context, { preventDefault: vi.fn() })

    expect(context.apps.map(app => app.name)).toEqual(['Bravo', 'Charlie', 'Alpha'])
    expect(context.listReordered).toBe(true)
  })

  it('ignores drops while the visible order is filtered or sorted', () => {
    const context = {
      apps: [{ name: 'Alpha' }, { name: 'Bravo' }],
      canReorder: false,
      draggingApp: 0,
      dragOverIndex: 0,
      listReordered: false,
      onDragEnd: Apps.methods.onDragEnd,
    }

    Apps.methods.onDrop.call(context, { preventDefault: vi.fn() }, 1)

    expect(context.apps.map(app => app.name)).toEqual(['Alpha', 'Bravo'])
    expect(context.listReordered).toBe(false)
  })

  it('alphabetizes the persisted order without saving immediately', () => {
    const alert = vi.fn()
    vi.stubGlobal('alert', alert)

    const context = {
      apps: [{ name: 'Zulu' }, { name: 'alpha' }, { name: 'Bravo' }],
      listReordered: false,
      $t: key => key,
    }

    Apps.methods.alphabetizeApps.call(context)

    expect(context.apps.map(app => app.name)).toEqual(['alpha', 'Bravo', 'Zulu'])
    expect(context.listReordered).toBe(true)
    expect(alert).not.toHaveBeenCalled()
    vi.unstubAllGlobals()
  })

  it('persists the current application order', async () => {
    apiFetch.mockResolvedValueOnce({
      ok: true,
      json: () => Promise.resolve({ status: true }),
    })
    const context = {
      apps: [{ uuid: 'uuid-b' }, { uuid: 'uuid-a' }],
      savingOrder: false,
      loadApps: vi.fn(() => Promise.resolve()),
      $t: key => key,
    }

    const pending = Apps.methods.saveOrder.call(context)
    expect(context.savingOrder).toBe(true)
    await pending

    expect(apiFetch).toHaveBeenCalledWith('./api/apps/reorder', expect.objectContaining({
      method: 'POST',
      body: JSON.stringify({ order: ['uuid-b', 'uuid-a'] }),
    }))
    expect(context.loadApps).toHaveBeenCalledOnce()
    expect(context.savingOrder).toBe(false)
  })

  it('exports an Artemis launcher entry for an application', () => {
    const link = { href: '', download: '', click: vi.fn() }
    const createElement = vi.spyOn(document, 'createElement').mockReturnValue(link)

    const context = {
      hostUUID: 'host-uuid',
      hostName: 'Host name',
    }
    Apps.methods.exportLauncherFile.call(context, { uuid: 'app-uuid', name: 'My/App' })

    expect(link.download).toBe('My_App.art')
    const content = decodeURIComponent(link.href.slice(link.href.indexOf(',') + 1))
    expect(content).toContain('# Artemis app entry')
    expect(content).toContain('[host_uuid] host-uuid')
    expect(content).toContain('[host_name] Host name')
    expect(content).toContain('[app_uuid] app-uuid')
    expect(content).toContain('[app_name] My/App')
    expect(link.click).toHaveBeenCalledOnce()

    createElement.mockRestore()
  })
})
