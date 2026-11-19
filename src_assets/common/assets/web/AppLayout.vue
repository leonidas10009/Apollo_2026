<template>
  <div class="app-shell">
    <div
      v-if="sidebarOpen"
      class="app-sidebar-backdrop"
      aria-hidden="true"
      @click="closeSidebar"
    ></div>

    <aside id="app-sidebar" class="app-sidebar" :class="{ 'is-open': sidebarOpen }">
      <div class="app-sidebar-brand">
        <RouterLink class="app-sidebar-logo" to="/" :aria-label="$t('navbar.home')" @click="closeSidebar">
          <img :src="logoUrl" height="28" alt="Apollo">
        </RouterLink>
        <div class="d-flex align-items-center">
          <RouterLink class="app-brand-name text-decoration-none" to="/" @click="closeSidebar">Apollo</RouterLink>
          <span v-if="version" class="app-brand-badge" :title="$t('app_shell.version')">{{ versionShort }}</span>
        </div>
        <button type="button" class="app-sidebar-close d-lg-none" :aria-label="$t('app_shell.close_menu')" @click="closeSidebar">
          <X :size="20"></X>
        </button>
      </div>

      <nav class="app-sidebar-nav" :aria-label="$t('app_shell.main_navigation')">
        <div class="app-nav-section">{{ $t('app_shell.section_main') }}</div>
        <RouterLink
          v-for="item in mainNav"
          :key="item.to"
          class="app-nav-link"
          :class="{ active: isActive(item.to) }"
          :to="item.to"
          @click="closeSidebar"
        >
          <component :is="item.icon" :size="18" class="icon"></component>
          <span>{{ $t(item.labelKey) }}</span>
        </RouterLink>

        <div class="app-nav-section">{{ $t('app_shell.section_system') }}</div>
        <RouterLink
          v-for="item in systemNav"
          :key="item.to"
          class="app-nav-link"
          :class="{ active: isActive(item.to) }"
          :to="item.to"
          @click="closeSidebar"
        >
          <component :is="item.icon" :size="18" class="icon"></component>
          <span>{{ $t(item.labelKey) }}</span>
        </RouterLink>
      </nav>

      <div class="app-sidebar-footer">
        <div class="app-host-card">
          <div class="app-host-card-header">
            <span>{{ $t('app_shell.host_service') }}</span>
            <span class="app-status">
              <span class="app-status-dot"></span>
              {{ $t('app_shell.online') }}
            </span>
          </div>
          <div class="app-host-name" :title="hostName">{{ hostName }}</div>
          <div class="app-host-meta">
            <span v-if="version">v{{ versionShort }}</span>
            <span v-if="version && platform"> · </span>
            <span v-if="platform">{{ platform }}</span>
          </div>
        </div>
      </div>
    </aside>

    <div class="app-main">
      <header class="app-topbar">
        <button
          type="button"
          class="app-icon-button d-lg-none"
          :aria-label="$t('app_shell.open_menu')"
          @click="openSidebar"
        >
          <Menu :size="20"></Menu>
        </button>
        <h1 class="app-topbar-title">{{ pageTitle }}</h1>

        <div class="app-topbar-actions">
          <button
            type="button"
            class="app-search-trigger"
            :aria-label="$t('app_shell.search_placeholder')"
            @click="openPalette"
          >
            <Search :size="16"></Search>
            <span>{{ $t('app_shell.search_placeholder') }}</span>
            <kbd>Ctrl K</kbd>
          </button>

          <RouterLink class="app-pin-button" to="/pin">
            <KeyRound :size="16"></KeyRound>
            <span>{{ $t('app_shell.pair_device') }}</span>
          </RouterLink>

          <ThemeToggle></ThemeToggle>

          <div class="dropdown">
            <button
              type="button"
              class="app-user-button"
              data-bs-toggle="dropdown"
              aria-expanded="false"
              :aria-label="$t('app_shell.user_menu')"
            >
              <CircleUserRound :size="20"></CircleUserRound>
            </button>
            <ul class="dropdown-menu dropdown-menu-end">
              <li>
                <RouterLink class="dropdown-item d-flex align-items-center" to="/password">
                  <Shield :size="18" class="icon"></Shield>
                  {{ $t('navbar.password') }}
                </RouterLink>
              </li>
              <li><hr class="dropdown-divider"></li>
              <li>
                <button type="button" class="dropdown-item d-flex align-items-center" @click="logout">
                  <LogOut :size="18" class="icon"></LogOut>
                  {{ $t('navbar.logout') }}
                </button>
              </li>
            </ul>
          </div>
        </div>
      </header>

      <main class="app-content">
        <slot></slot>
      </main>
    </div>

    <div
      v-if="paletteOpen"
      class="command-palette-backdrop"
      role="presentation"
      @click.self="closePalette"
    >
      <div class="command-palette" role="dialog" aria-modal="true" :aria-label="$t('app_shell.search_placeholder')">
        <div class="command-palette-search">
          <Search :size="18"></Search>
          <input
            ref="paletteInput"
            v-model="paletteQuery"
            type="text"
            :placeholder="$t('app_shell.search_placeholder')"
            :aria-label="$t('app_shell.search_placeholder')"
            @keydown.down.prevent="moveSelection(1)"
            @keydown.up.prevent="moveSelection(-1)"
            @keydown.enter.prevent="runSelectedCommand"
            @keydown.esc.prevent="closePalette"
          />
          <button type="button" class="app-icon-button" :aria-label="$t('_common.close')" @click="closePalette">
            <X :size="16"></X>
          </button>
        </div>
        <div class="command-palette-list">
          <button
            v-for="(command, index) in filteredCommands"
            :key="command.key"
            type="button"
            class="command-palette-item"
            :class="{ 'is-active': index === paletteIndex }"
            @mouseenter="paletteIndex = index"
            @click="runCommand(command)"
          >
            <component :is="command.icon" :size="16" class="icon"></component>
            <span>{{ $t(command.labelKey) }}</span>
            <kbd>{{ $t(command.groupKey) }}</kbd>
          </button>
          <div v-if="filteredCommands.length === 0" class="command-palette-empty">
            {{ $t('app_shell.no_results') }}
          </div>
        </div>
      </div>
    </div>

    <Notification></Notification>
  </div>
</template>

<script>
import { CircleUserRound, Gamepad2, KeyRound, LayoutDashboard, LifeBuoy, LogOut, Menu, Search, Settings, Shield, Star, Users, X } from '@lucide/vue'
import ThemeToggle from './ThemeToggle.vue'
import Notification from './Notification.vue'

const MAIN_NAV = [
  { to: '/', labelKey: 'navbar.home', icon: 'LayoutDashboard' },
  { to: '/apps', labelKey: 'navbar.applications', icon: 'Gamepad2' },
  { to: '/clients', labelKey: 'client_card.clients', icon: 'Users' },
  { to: '/pin', labelKey: 'navbar.pin', icon: 'KeyRound' },
]

const SYSTEM_NAV = [
  { to: '/config', labelKey: 'navbar.configuration', icon: 'Settings' },
  { to: '/featured', labelKey: 'navbar.featured', icon: 'Star' },
  { to: '/troubleshooting', labelKey: 'navbar.troubleshoot', icon: 'LifeBuoy' },
]

/**
 * Shared navigation shell with a responsive sidebar, top bar, command palette,
 * and the global notification host.
 */
export default {
  components: {
    CircleUserRound,
    Gamepad2,
    KeyRound,
    LayoutDashboard,
    LifeBuoy,
    LogOut,
    Menu,
    Notification,
    Search,
    Settings,
    Shield,
    Star,
    ThemeToggle,
    Users,
    X,
  },
  data() {
    return {
      hostName: globalThis.location?.hostname || 'localhost',
      logoUrl: '/images/logo-apollo-45.png',
      mobileSidebarOpen: false,
      paletteIndex: 0,
      paletteOpen: false,
      paletteQuery: '',
      platform: '',
      version: '',
    }
  },
  computed: {
    mainNav() {
      return MAIN_NAV
    },
    systemNav() {
      return SYSTEM_NAV
    },
    pageTitle() {
      return this.$route.meta.titleKey ? this.$t(this.$route.meta.titleKey) : 'Apollo'
    },
    versionShort() {
      return this.version.split('.').slice(0, 3).join('.')
    },
    sidebarOpen() {
      return this.mobileSidebarOpen
    },
    commands() {
      const navigation = [...MAIN_NAV, ...SYSTEM_NAV].map(item => ({
        key: `nav:${item.to}`,
        labelKey: item.labelKey,
        groupKey: 'app_shell.group_navigation',
        icon: item.icon,
        to: item.to,
      }))
      return [
        ...navigation,
        {
          key: 'action:password',
          labelKey: 'navbar.password',
          groupKey: 'app_shell.group_account',
          icon: 'Shield',
          to: '/password',
        },
        {
          key: 'action:logout',
          labelKey: 'navbar.logout',
          groupKey: 'app_shell.group_account',
          icon: 'LogOut',
          action: 'logout',
        },
      ]
    },
    filteredCommands() {
      const query = this.paletteQuery.trim().toLowerCase()
      if (!query) {
        return this.commands
      }
      return this.commands.filter(command => this.$t(command.labelKey).toLowerCase().includes(query))
    },
  },
  watch: {
    '$route.fullPath'() {
      this.closeSidebar()
      this.closePalette()
    },
    filteredCommands() {
      this.paletteIndex = 0
    },
  },
  mounted() {
    globalThis.addEventListener('keydown', this.handleKeydown)
    this.loadHostSummary()
  },
  beforeUnmount() {
    globalThis.removeEventListener('keydown', this.handleKeydown)
  },
  methods: {
    /**
     * @brief Check whether a navigation destination matches the current route.
     *
     * @param {string} path Destination route path.
     * @return {boolean} Whether the destination is the active section.
     */
    isActive(path) {
      if (path === '/') {
        return this.$route.path === '/'
      }
      return this.$route.path === path || this.$route.path.startsWith(`${path}/`)
    },
    openSidebar() {
      this.mobileSidebarOpen = true
    },
    closeSidebar() {
      this.mobileSidebarOpen = false
    },
    openPalette() {
      this.paletteOpen = true
      this.paletteIndex = 0
      this.$nextTick(() => this.$refs.paletteInput?.focus())
    },
    closePalette() {
      this.paletteOpen = false
      this.paletteQuery = ''
      this.paletteIndex = 0
    },
    moveSelection(direction) {
      if (!this.filteredCommands.length) {
        return
      }
      const next = this.paletteIndex + direction
      this.paletteIndex = (next + this.filteredCommands.length) % this.filteredCommands.length
    },
    runSelectedCommand() {
      const command = this.filteredCommands[this.paletteIndex]
      if (command) {
        this.runCommand(command)
      }
    },
    /**
     * @brief Execute a command palette entry.
     *
     * @param {object} command Command descriptor.
     */
    runCommand(command) {
      this.closePalette()
      if (command.action === 'logout') {
        this.logout()
        return
      }
      this.$router.push(command.to)
    },
    /**
     * @brief Handle the global command palette shortcut.
     *
     * @param {KeyboardEvent} event Keyboard event.
     */
    handleKeydown(event) {
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'k') {
        event.preventDefault()
        if (this.paletteOpen) {
          this.closePalette()
        } else {
          this.openPalette()
        }
        return
      }
      if (event.key === 'Escape' && this.paletteOpen) {
        this.closePalette()
      }
    },
    /**
     * @brief Load the host version and platform for the sidebar status card.
     */
    async loadHostSummary() {
      try {
        const config = await fetch('./api/config').then(response => response.json())
        this.version = config.version || ''
        this.platform = config.platform || ''
        this.hostName = config.sunshine_name || this.hostName
      } catch (error) {
        console.error('Failed to load host summary:', error)
      }
    },
    logout() {
      const cacheBuster = Date.now().toString()
      const logoutPageUrl = new URL('/logout', globalThis.location.href)
      const request = new XMLHttpRequest()
      const finish = () => {
        globalThis.location.replace(logoutPageUrl.toString())
      }

      request.open('GET', '/', true, 'sunshine-logout', cacheBuster)
      request.setRequestHeader('Cache-Control', 'no-store')
      request.onload = finish
      request.onerror = finish
      request.ontimeout = finish
      request.timeout = 5000
      request.send()
    },
  },
}
</script>
