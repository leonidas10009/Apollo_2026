import { createRouter, createWebHistory } from 'vue-router'

/**
 * Web UI routes rendered from the single Vite entry page.
 *
 * Lazy page imports keep the source split into focused components while the
 * server serves the same index document for every browser route. Routes with
 * `layout: 'bare'` skip the application shell, which is used by first-run and
 * logout screens.
 */
const routes = [
  { path: '/', component: () => import('./Home.vue'), meta: { titleKey: 'navbar.home' } },
  { path: '/apps', component: () => import('./Apps.vue'), meta: { titleKey: 'navbar.applications' } },
  { path: '/clients', component: () => import('./Clients.vue'), meta: { titleKey: 'client_card.clients' } },
  { path: '/config', component: () => import('./Config.vue'), meta: { titleKey: 'navbar.configuration' } },
  { path: '/featured', component: () => import('./Featured.vue'), meta: { titleKey: 'navbar.featured' } },
  { path: '/logout', component: () => import('./Logout.vue'), meta: { layout: 'bare' } },
  { path: '/password', component: () => import('./Password.vue'), meta: { titleKey: 'navbar.password' } },
  { path: '/pin', component: () => import('./Pin.vue'), meta: { titleKey: 'navbar.pin' } },
  { path: '/troubleshooting', component: () => import('./Troubleshooting.vue'), meta: { titleKey: 'navbar.troubleshoot' } },
  { path: '/welcome', component: () => import('./Welcome.vue'), meta: { layout: 'bare' } },
  { path: '/:pathMatch(.*)*', redirect: '/' },
]

const router = createRouter({
  history: createWebHistory(),
  routes,
  scrollBehavior(to) {
    return to.hash ? { el: to.hash } : { top: 0 }
  },
})

export default router
