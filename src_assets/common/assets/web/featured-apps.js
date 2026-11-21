/**
 * @file featured-apps.js
 * @brief Curated directory for the Featured Apps page.
 *
 * Apollo 2026 ships its own client ecosystem. The upstream LizardByte app
 * directory only lists official Moonlight clients, which do not expose the
 * Apollo/Artemis extensions used by this host. Keep this list local so the page
 * is deterministic, works offline and never promotes an incompatible client.
 */

export const FEATURED_CATEGORIES = [
  { id: 'apollo:client', name: 'Clients', originalId: 'client' },
]

export const FEATURED_APPS = [
  {
    id: 'artemis-2026',
    name: 'Artemis 2026',
    tagline: 'Android and Android TV client for Apollo 2026',
    description:
      'The official Artemis client for Apollo 2026. It includes the Apollo interface, ' +
      'remote power actions, tablet layouts, controller support and the Artemis streaming core.',
    category: 'apollo:client',
    platforms: ['android', 'tv'],
    icon: '/assets/artemis-icon.png',
    screenshots: ['/assets/artemis-screenshot.png'],
    links: {
      github: 'https://github.com/leonidas10009/Artemis-2026',
      download: 'https://github.com/leonidas10009/Artemis-2026/releases/latest',
      documentation: 'https://github.com/leonidas10009/Artemis-2026#readme',
    },
    official: true,
    github: {
      lastUpdated: '2026-11-21T10:17:19Z',
    },
  },
]
