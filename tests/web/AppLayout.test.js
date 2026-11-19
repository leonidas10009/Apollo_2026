import { mount, flushPromises } from '@vue/test-utils'
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest'

import AppLayout from '../../src_assets/common/assets/web/AppLayout.vue'

const RouterLinkStub = {
  props: ['to'],
  template: '<a :href="to"><slot /></a>',
}

/**
 * @brief Mount the application shell with a deterministic host summary.
 *
 * @returns {Promise<import('@vue/test-utils').VueWrapper>} Mounted shell.
 */
async function mountLayout() {
  vi.stubGlobal('fetch', vi.fn(async url => {
    if (url === './api/config') {
      return {
        json: async () => ({ version: '2026.927.1200', platform: 'windows' }),
      }
    }
    throw new Error(`Unexpected fetch: ${url}`)
  }))

  const router = { push: vi.fn() }
  const wrapper = mount(AppLayout, {
    attachTo: document.body,
    global: {
      mocks: {
        $route: { path: '/', meta: { titleKey: 'navbar.home' } },
        $router: router,
        $t: key => key,
      },
      stubs: {
        RouterLink: RouterLinkStub,
        ThemeToggle: true,
        Notification: true,
      },
    },
  })
  await flushPromises()
  return { wrapper, router }
}

beforeEach(() => {
  document.body.replaceChildren()
})

afterEach(() => {
  vi.unstubAllGlobals()
  document.body.replaceChildren()
})

describe('application shell', () => {
  it('renders every navigation destination and the host summary', async () => {
    const { wrapper } = await mountLayout()

    const hrefs = wrapper.findAll('.app-nav-link').map(link => link.attributes('href'))
    expect(hrefs).toEqual(['/', '/apps', '/clients', '/pin', '/config', '/featured', '/troubleshooting'])
    expect(wrapper.get('.app-topbar-title').text()).toBe('navbar.home')
    expect(wrapper.get('.app-host-name').text()).toBe('localhost')
    expect(wrapper.text()).toContain('v2026.927.1200')
    expect(wrapper.text()).toContain('windows')
    wrapper.unmount()
  })

  it('opens the command palette with Ctrl+K and navigates with Enter', async () => {
    const { wrapper, router } = await mountLayout()

    window.dispatchEvent(new KeyboardEvent('keydown', { key: 'k', ctrlKey: true }))
    await wrapper.vm.$nextTick()
    expect(wrapper.find('.command-palette').exists()).toBe(true)

    await wrapper.get('.command-palette-search input').setValue('configuration')
    expect(wrapper.findAll('.command-palette-item')).toHaveLength(1)

    await wrapper.get('.command-palette-search input').trigger('keydown', { key: 'Enter' })
    expect(router.push).toHaveBeenCalledWith('/config')
    expect(wrapper.find('.command-palette').exists()).toBe(false)
    wrapper.unmount()
  })

  it('filters the command palette to an empty state', async () => {
    const { wrapper } = await mountLayout()

    await wrapper.get('.app-search-trigger').trigger('click')
    await wrapper.get('.command-palette-search input').setValue('no-such-destination')

    expect(wrapper.findAll('.command-palette-item')).toHaveLength(0)
    expect(wrapper.get('.command-palette-empty').text()).toBe('app_shell.no_results')
    wrapper.unmount()
  })

  it('toggles the mobile sidebar and closes it with the backdrop', async () => {
    const { wrapper } = await mountLayout()

    const menu = wrapper.findAll('.app-topbar .app-icon-button')[0]
    await menu.trigger('click')
    expect(wrapper.get('.app-sidebar').classes()).toContain('is-open')

    await wrapper.get('.app-sidebar-backdrop').trigger('click')
    expect(wrapper.get('.app-sidebar').classes()).not.toContain('is-open')
    wrapper.unmount()
  })
})
