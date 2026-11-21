import { mount } from '@vue/test-utils'
import { afterEach, describe, expect, it, vi } from 'vitest'

import Featured from '../../src_assets/common/assets/web/Featured.vue'

function mountFeatured() {
  return mount(Featured, {
    global: {
      mocks: { $t: key => key },
      stubs: { SimpleIcon: true },
    },
  })
}

afterEach(() => {
  vi.unstubAllGlobals()
})

describe('Featured Apps curated directory', () => {
  it('only offers Artemis and never requests the upstream Moonlight directory', () => {
    const fetchSpy = vi.fn()
    vi.stubGlobal('fetch', fetchSpy)

    const wrapper = mountFeatured()
    const cards = wrapper.findAll('.featured-app-card')

    expect(fetchSpy).not.toHaveBeenCalled()
    expect(cards).toHaveLength(1)
    expect(cards[0].text()).toContain('Artemis 2026')
    expect(wrapper.text()).not.toContain('Moonlight PC')
    expect(wrapper.text()).not.toContain('Moonlight for Android')
    wrapper.unmount()
  })

  it('renders the local Artemis icon and download link', () => {
    const wrapper = mountFeatured()
    const card = wrapper.get('.featured-app-card')

    expect(card.get('.featured-app-icon img').attributes('src')).toBe('/assets/artemis-icon.png')
    expect(card.get('a[href="https://github.com/leonidas10009/Artemis-2026/releases/latest"]').exists()).toBe(true)
    wrapper.unmount()
  })

  it('keeps Artemis visible when the client category is selected', async () => {
    const wrapper = mountFeatured()
    const buttons = wrapper.findAll('.btn-group > .btn')

    expect(buttons).toHaveLength(2)
    await buttons[1].trigger('click')

    expect(buttons[1].classes()).toContain('active')
    expect(wrapper.findAll('.featured-app-card')).toHaveLength(1)
    wrapper.unmount()
  })

  it('shows the Artemis screenshot as a contained thumbnail', () => {
    const wrapper = mountFeatured()
    const screenshot = wrapper.get('.screenshot-thumbnail')

    expect(screenshot.attributes('src')).toBe('/assets/artemis-screenshot.png')
    expect(screenshot.attributes('alt')).toContain('Artemis 2026')
    wrapper.unmount()
  })
})
