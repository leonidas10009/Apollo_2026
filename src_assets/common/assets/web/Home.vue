<template>
  <div id="content">
    <div class="page-header">
      <div>
        <h1 class="page-header-title">{{ $t('index.welcome') }}</h1>
        <p class="page-header-desc">{{ $t('index.description') }}</p>
      </div>
      <div class="page-header-actions">
        <RouterLink class="btn btn-primary" to="/pin">
          <key-round :size="18" class="icon"></key-round>
          {{ $t('dashboard.action_pair') }}
        </RouterLink>
      </div>
    </div>

    <!-- Fatal Errors Alert -->
    <div class="alert alert-danger my-4" v-if="fancyLogs.some(x => x.level === 'Fatal')">
      <div>
        <div class="d-flex align-items-center mb-3">
          <alert-circle :size="32" class="icon-lg me-3"></alert-circle>
          <div v-html="$t('index.startup_errors')"></div>
        </div>
        <ul class="mb-3">
          <li v-for="v in fancyLogs.filter(x => x.level === 'Fatal')" :key="`${v.timestamp}-${v.value}`">{{v.value}}</li>
        </ul>
        <RouterLink class="btn btn-danger" to="/troubleshooting#logs">
          <file-text :size="18" class="icon"></file-text>
          {{ $t('index.view_logs') }}
        </RouterLink>
      </div>
    </div>

    <div class="alert alert-warning my-4" v-if="missingPermissions.length">
      <div class="d-flex align-items-center gap-3 mb-2">
        <alert-triangle :size="24" class="icon"></alert-triangle>
        <strong>{{ $t('index.permissions_missing_title') }}</strong>
      </div>
      <p>{{ $t('index.permissions_missing_desc') }}</p>
      <RouterLink class="btn btn-warning" to="/troubleshooting#permissions">
        {{ $t('index.permissions_review') }}
      </RouterLink>
    </div>

    <!-- Virtual gamepad broker status -->
    <div class="alert my-4" :class="virtualInputNotice.alertClass" v-if="virtualInputNotice">
      <div>
        <div class="d-flex align-items-center mb-3">
          <alert-triangle v-if="virtualInputNotice.warning" :size="32" class="icon-lg me-3"></alert-triangle>
          <info v-else :size="32" class="icon-lg me-3"></info>
          <div>
            <p class="mb-1"><strong>{{ $t(virtualInputNotice.title) }}</strong></p>
            <p v-for="message in virtualInputNotice.messages" :key="message.key" class="mb-1">
              {{ $t(message.key, message.params || {}) }}
            </p>
          </div>
        </div>
        <RouterLink class="btn" :class="virtualInputNotice.buttonClass" :to="virtualInputNotice.to">
          <gamepad-2 v-if="virtualInputNotice.chooseDriver" :size="18" class="icon"></gamepad-2>
          <wrench v-else :size="18" class="icon"></wrench>
          {{ $t(virtualInputNotice.action) }}
        </RouterLink>
      </div>
    </div>

    <section class="dashboard-hero" :aria-label="$t('dashboard.title')">
      <div class="dashboard-hero-main">
        <div class="dashboard-hero-icon">
          <cast :size="24"></cast>
        </div>
        <div>
          <span class="badge" :class="isStreaming ? 'text-bg-success' : 'text-bg-secondary'">
            <span class="app-status-dot me-1"></span>
            {{ isStreaming ? $t('dashboard.status_streaming') : $t('dashboard.status_idle') }}
          </span>
          <h2 class="dashboard-hero-title">{{ sessionTitle }}</h2>
          <p class="dashboard-hero-subtitle">{{ isStreaming ? $t('dashboard.status_streaming_desc') : $t('dashboard.status_idle_desc') }}</p>
        </div>
      </div>
      <div class="dashboard-hero-actions">
        <RouterLink class="btn btn-outline-primary" to="/clients">
          <users :size="18" class="icon"></users>
          {{ $t('dashboard.action_clients') }}
        </RouterLink>
        <RouterLink class="btn btn-success" to="/apps">
          <gamepad-2 :size="18" class="icon"></gamepad-2>
          {{ $t('dashboard.action_apps') }}
        </RouterLink>
      </div>
    </section>

    <div class="dashboard-stats">
      <div class="stat-card">
        <div class="stat-card-header">
          <span>{{ $t('dashboard.paired_clients') }}</span>
          <users :size="16"></users>
        </div>
        <div class="stat-card-value">{{ pairedClients }}</div>
        <div class="stat-card-meta">
          <template v-if="pairedClients">{{ $t('dashboard.clients_connected', { count: connectedClients }) }}</template>
          <template v-else>{{ $t('dashboard.clients_none') }}</template>
        </div>
      </div>

      <div class="stat-card">
        <div class="stat-card-header">
          <span>{{ $t('dashboard.applications') }}</span>
          <monitor-play :size="16"></monitor-play>
        </div>
        <div class="stat-card-value">{{ appCount }}</div>
        <div class="stat-card-meta">{{ $t('dashboard.applications_desc') }}</div>
      </div>

      <div class="stat-card">
        <div class="stat-card-header">
          <span>{{ $t('dashboard.host_version') }}</span>
          <package :size="16"></package>
        </div>
        <div class="stat-card-value">{{ version ? version.version : '—' }}</div>
        <div class="stat-card-meta">{{ $t(versionStatusKey) }}</div>
      </div>

      <div class="stat-card">
        <div class="stat-card-header">
          <span>{{ $t('dashboard.platform') }}</span>
          <cpu :size="16"></cpu>
        </div>
        <div class="stat-card-value">{{ platform || '—' }}</div>
        <div class="stat-card-meta">{{ gamepadDriver || $t('dashboard.session_details') }}</div>
      </div>
    </div>

    <div class="dashboard-columns">
      <div class="dashboard-panel">
        <div class="dashboard-panel-header">
          <h3 class="dashboard-panel-title">{{ $t('dashboard.quick_actions') }}</h3>
        </div>
        <div class="quick-action-list">
          <RouterLink class="quick-action" to="/pin">
            <span class="quick-action-icon"><key-round :size="16"></key-round></span>
            <span>{{ $t('dashboard.action_pair') }}</span>
            <chevron-right :size="16" class="quick-action-arrow"></chevron-right>
          </RouterLink>
          <RouterLink class="quick-action" to="/clients">
            <span class="quick-action-icon"><shield :size="16"></shield></span>
            <span>{{ $t('dashboard.action_clients') }}</span>
            <chevron-right :size="16" class="quick-action-arrow"></chevron-right>
          </RouterLink>
          <RouterLink class="quick-action" to="/config">
            <span class="quick-action-icon"><settings :size="16"></settings></span>
            <span>{{ $t('dashboard.action_config') }}</span>
            <chevron-right :size="16" class="quick-action-arrow"></chevron-right>
          </RouterLink>
          <RouterLink class="quick-action" to="/troubleshooting">
            <span class="quick-action-icon"><life-buoy :size="16"></life-buoy></span>
            <span>{{ $t('dashboard.action_logs') }}</span>
            <chevron-right :size="16" class="quick-action-arrow"></chevron-right>
          </RouterLink>
        </div>
      </div>

      <div class="dashboard-panel">
        <div class="dashboard-panel-header">
          <h3 class="dashboard-panel-title">{{ $t('dashboard.session_details') }}</h3>
          <RouterLink class="btn btn-sm btn-outline-secondary" to="/clients">{{ $t('dashboard.view_all') }}</RouterLink>
        </div>
        <dl class="row mb-0">
          <dt class="col-sm-5">{{ $t('dashboard.host_name') }}</dt>
          <dd class="col-sm-7 monospace">{{ hostName }}</dd>
          <dt class="col-sm-5">{{ $t('dashboard.current_application') }}</dt>
          <dd class="col-sm-7">{{ currentAppName || $t('dashboard.no_active_app') }}</dd>
          <dt class="col-sm-5">{{ $t('dashboard.connected_devices') }}</dt>
          <dd class="col-sm-7">{{ connectedClients }} / {{ pairedClients }}</dd>
        </dl>

        <div v-if="clients.length" class="mt-3 pt-3 border-top border-secondary-subtle">
          <div v-for="client in clients.slice(0, 4)" :key="client.uuid" class="d-flex align-items-center justify-content-between py-1">
            <span class="text-truncate">{{ client.name || $t('pin.unpair_single_unknown') }}</span>
            <span class="badge" :class="client.connected ? 'text-bg-success' : 'text-bg-secondary'">
              {{ client.connected ? $t('pin.connected') : $t('pin.offline') }}
            </span>
          </div>
        </div>
        <p v-else class="text-muted mb-0 mt-3">{{ $t('dashboard.clients_none') }}</p>
      </div>
    </div>

    <!-- Version and release information -->
    <div class="dashboard-panel mb-4" v-if="version">
      <div class="dashboard-panel-header">
        <h3 class="dashboard-panel-title">{{ $t('dashboard.updates') }}</h3>
        <span class="badge text-bg-secondary">v{{ version.version }}</span>
      </div>

      <div v-if="loading" class="my-3">
        {{ $t('index.loading_latest') }}
      </div>

      <div class="alert alert-success my-3" v-if="buildVersionIsDirty">
        <package :size="18" class="icon"></package>
        {{ $t('index.version_dirty') }} 🌇
      </div>

      <div class="alert alert-info my-3" v-if="installedVersionNotStable">
        <info :size="18" class="icon"></info>
        {{ $t('index.installed_version_not_stable') }}
      </div>

      <div v-else-if="(!preReleaseBuildAvailable || !notifyPreReleases) && !stableBuildAvailable && !buildVersionIsDirty">
        <div class="alert alert-success my-3">
          <check-circle :size="18" class="icon"></check-circle>
          {{ $t('index.version_latest') }}
        </div>
      </div>

      <div v-if="notifyPreReleases && preReleaseBuildAvailable">
        <div class="alert alert-warning my-3">
          <div class="d-flex align-items-center justify-content-between gap-3 flex-wrap mb-3">
            <div class="d-flex align-items-center gap-3 flex-wrap">
              <alert-circle :size="18" class="icon"></alert-circle>
              <span>{{ $t('index.new_pre_release') }}</span>
              <h5 class="mb-0">{{ preReleaseVersion.release.name }}</h5>
            </div>
            <a class="btn btn-success flex-shrink-0" :href="preReleaseVersion.release.html_url" target="_blank">
              <download :size="18" class="icon"></download>
              {{ $t('index.download') }}
            </a>
          </div>

          <div class="markdown-body release-notes" v-html="convertMarkdownToHtml(preReleaseVersion.release.body)"></div>
        </div>
      </div>

      <div v-if="stableBuildAvailable">
        <div class="alert alert-warning my-3">
          <div class="d-flex align-items-center justify-content-between gap-3 flex-wrap mb-3">
            <div class="d-flex align-items-center gap-3 flex-wrap">
              <alert-circle :size="18" class="icon"></alert-circle>
              <span>{{ $t('index.new_stable') }}</span>
              <h5 class="mb-0">{{ githubVersion.release.name }}</h5>
            </div>
            <a class="btn btn-success flex-shrink-0" :href="githubVersion.release.html_url" target="_blank">
              <download :size="18" class="icon"></download>
              {{ $t('index.download') }}
            </a>
          </div>

          <div class="markdown-body release-notes" v-html="convertMarkdownToHtml(githubVersion.release.body)"></div>
        </div>
      </div>
    </div>

    <!-- Resources -->
    <div class="my-4">
      <Resource-Card :installed-version-not-stable="installedVersionNotStable"></Resource-Card>
    </div>
  </div>
</template>

<script>
  import { marked } from 'marked'
  import ResourceCard from './ResourceCard.vue'
  import SunshineVersion from './sunshine_version'
  import {
    AlertCircle,
    AlertTriangle,
    Cast,
    ChevronRight,
    Cpu,
    FileText,
    Wrench,
    Gamepad2,
    Info,
    CheckCircle,
    Download,
    KeyRound,
    LifeBuoy,
    MonitorPlay,
    Package,
    Settings,
    Shield,
    Users,
  } from '@lucide/vue'

  // Configure marked to allow HTML
  marked.setOptions({
    breaks: true,
    gfm: true,
    headerIds: true,
    mangle: false,
    sanitize: false
  });

  console.log("Hello, Apollo!")
  export default {
    components: {
      ResourceCard,
      AlertCircle,
      AlertTriangle,
      Cast,
      ChevronRight,
      Cpu,
      FileText,
      Wrench,
      Package,
      Info,
      CheckCircle,
      Download,
      Gamepad2,
      KeyRound,
      LifeBuoy,
      MonitorPlay,
      Settings,
      Shield,
      Users,
    },
    data() {
      return {
        version: null,
        githubVersion: null,
        notifyPreReleases: false,
        preReleaseVersion: null,
        loading: true,
        logs: null,
        platform: "",
        controllerEnabled: false,
        gamepadDriver: '',
        virtualhid: null,
        virtualhidLicense: null,
        vigembus: null,
        permissions: [],
        appsData: null,
        clients: [],
      }
    },
    async created() {
      try {
        let config = await fetch("./api/config").then((r) => r.json());
        this.notifyPreReleases = config.notify_pre_releases;
        this.platform = config.platform;
        this.controllerEnabled = config.controller !== "disabled";
        this.gamepadDriver = config.gamepad_driver || '';
        this.version = new SunshineVersion(null, config.version);
        try {
          const response = await fetch('./api/permissions');
          this.permissions = (await response.json()).permissions || [];
        } catch (e) {
          console.error('Failed to fetch permission status:', e);
        }
        try {
          this.appsData = await fetch('./api/apps').then((r) => r.json());
        } catch (e) {
          console.error('Failed to fetch applications:', e);
        }
        try {
          const clientsResponse = await fetch('./api/clients/list').then((r) => r.json());
          this.clients = clientsResponse.named_certs || [];
        } catch (e) {
          console.error('Failed to fetch clients:', e);
        }
        console.log("Version: ", this.version.version)
        this.githubVersion = new SunshineVersion(await fetch("https://api.github.com/repos/ClassicOldSong/Apollo/releases/latest").then((r) => r.json()), null);
        console.log("GitHub Version: ", this.githubVersion.version)
        this.preReleaseVersion = new SunshineVersion((await fetch("https://api.github.com/repos/ClassicOldSong/Apollo/releases").then((r) => r.json())).find(release => release.prerelease), null);
        console.log("Pre-Release Version: ", this.preReleaseVersion.version)

        // Read the broker version on both platforms to identify development builds.
        if (this.platform === 'windows' || this.platform === 'macos') {
          try {
            const virtualInputStatus = await fetch("./api/virtual-input/status").then((r) => r.json());
            this.virtualhid = virtualInputStatus.virtualhid;
            if (this.platform === 'windows') {
              this.vigembus = virtualInputStatus.vigembus;
            }
          } catch (e) {
            console.error("Failed to fetch virtual input driver status:", e);
          }
        }
        if (this.platform === 'windows' || this.platform === 'macos') {
          try {
            this.virtualhidLicense = await fetch("./api/virtual-input/license").then((r) => r.json());
          } catch (e) {
            console.error("Failed to fetch Virtual HID Broker license status:", e);
          }
        }
      } catch (e) {
        console.error(e);
      }
      try {
        this.logs = (await fetch("./api/logs").then(r => r.text()))
      } catch (e) {
        console.error(e)
      }
      this.loading = false;
    },
    computed: {
      /** Return required permissions that the current platform can check and has not granted. */
      missingPermissions() {
        return this.permissions.filter(permission => permission.required && permission.verifiable && permission.status !== 'granted');
      },
      /** Applications configured on this host. */
      apps() {
        return this.appsData?.apps || [];
      },
      /** Number of configured applications. */
      appCount() {
        return this.apps.length;
      },
      /** Display name of the host reported by the applications endpoint. */
      hostName() {
        return this.appsData?.host_name || globalThis.location?.hostname || 'localhost';
      },
      /** Application currently running, if any. */
      currentApp() {
        const currentId = this.appsData?.current_app;
        return this.apps.find(app => app.uuid === currentId) || null;
      },
      /** Name of the running application. */
      currentAppName() {
        return this.currentApp?.name || '';
      },
      /** Number of paired devices. */
      pairedClients() {
        return this.clients.length;
      },
      /** Number of devices connected right now. */
      connectedClients() {
        return this.clients.filter(client => client.connected).length;
      },
      /** Whether a session or application is active. */
      isStreaming() {
        return !!this.currentAppName || this.connectedClients > 0;
      },
      /** Headline for the dashboard hero card. */
      sessionTitle() {
        return this.currentAppName || (this.isStreaming ? this.$t('dashboard.status_streaming') : this.$t('dashboard.status_idle'));
      },
      /**
       * Build the virtual-input message shown on the home page.
       * Warn about broker or gamepad driver issues when virtual gamepads are enabled.
       */
      virtualInputNotice() {
        if (!this.controllerEnabled || this.gamepadDriver === 'none') {
          return null;
        }

        if (this.platform === 'macos') {
          return this.buildMacosVirtualInputNotice();
        }

        if (this.platform !== 'windows' || !this.virtualhid || !this.vigembus) {
          return null;
        }

        const vigembusUsable = this.vigembus.installed && this.vigembus.version_compatible;

        if (!this.gamepadDriver) {
          return this.buildVirtualInputNotice(false, 'index.gamepad_driver_choice_title', [{
            key: 'index.gamepad_driver_choice_desc',
          }], {
            action: 'index.choose_gamepad_driver',
            chooseDriver: true,
            to: '/config#gamepad_driver',
          });
        }

        if (this.gamepadDriver === 'vigembus') {
          return this.buildVigembusNotice(vigembusUsable);
        }

        if (this.virtualhid.installed) {
          return this.buildInstalledVirtualhidNotice(vigembusUsable);
        }

        if (this.gamepadDriver === 'virtualhid') {
          return this.buildVirtualInputNotice(true, 'index.virtualhid_broker_unavailable_title', [{ key: 'index.virtualhid_required_desc' }]);
        }

        if (this.controllerEnabled && !vigembusUsable) {
          return this.buildVirtualInputNotice(true, 'index.virtual_input_unavailable_title', [{ key: 'index.virtual_input_unavailable_desc' }]);
        }

        return this.buildVirtualInputNotice(
          false,
          'index.virtualhid_optional_title',
          [{ key: this.controllerEnabled ? 'index.virtualhid_optional_vigembus_desc' : 'index.virtualhid_optional_desc' }],
        );
      },
      installedVersionNotStable() {
        if (!this.githubVersion || !this.version) {
          return false;
        }
        return this.version.isGreater(this.githubVersion);
      },
      stableBuildAvailable() {
        if (!this.githubVersion || !this.version) {
          return false;
        }
        return this.githubVersion.isGreater(this.version);
      },
      preReleaseBuildAvailable() {
        if (!this.preReleaseVersion || !this.githubVersion || !this.version) {
          return false;
        }
        return this.preReleaseVersion.isGreater(this.version) && this.preReleaseVersion.isGreater(this.githubVersion);
      },
      buildVersionIsDirty() {
        if (!this.version) {
          return false;
        }
        return this.version.version?.split(".").length === 5 &&
          this.version.version.includes("dirty")
      },
      /** Localization key describing the installed build compared with upstream releases. */
      versionStatusKey() {
        if (this.loading) {
          return 'dashboard.build_loading'
        }
        if (this.buildVersionIsDirty) {
          return 'dashboard.build_dirty'
        }
        if (this.installedVersionNotStable) {
          return 'dashboard.build_prerelease'
        }
        if (this.stableBuildAvailable) {
          return 'dashboard.build_update'
        }
        return 'dashboard.build_current'
      },
      /** Parse the text errors, calculating the text, the timestamp and the level */
      fancyLogs() {
        if (!this.logs) return [];
        let regex = /(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}]):\s/g;
        let rawLogLines = (this.logs.split(regex)).splice(1);
        let logLines = []
        for (let i = 0; i < rawLogLines.length; i += 2) {
          logLines.push({ timestamp: rawLogLines[i], level: rawLogLines[i + 1].split(":")[0], value: rawLogLines[i + 1] });
        }
        return logLines;
      }
    },
    methods: {
      /**
       * Build the macOS broker notice, prioritizing license and service warnings.
       *
       * @returns {object|null} Warning or development-build notice, when applicable.
       */
      buildMacosVirtualInputNotice() {
        if (this.virtualhidLicense && !this.virtualhidLicense.licensed) {
          return this.virtualhidLicense.service_available
            ? this.buildVirtualInputNotice(true, 'index.virtualhid_macos_license_title', [{ key: 'index.virtualhid_macos_license_desc' }])
            : this.buildVirtualInputNotice(true, 'index.virtualhid_broker_unavailable_title', [{ key: 'index.virtualhid_macos_broker_desc' }]);
        }
        return this.virtualhid?.development_version
          ? this.buildVirtualInputNotice(false, 'index.virtualhid_development_title', [{ key: 'index.virtualhid_development_desc' }])
          : null;
      },
      /**
       * Build a home-page notice for the current virtual-input state.
       *
       * @param {boolean} warning Whether the notice represents an actionable warning.
       * @param {string} title Localization key for the notice title.
       * @param {object[]} messages Localized message descriptors.
       * @param {object} options Optional action and destination overrides.
       * @returns {object} Notice data consumed by the template.
       */
      buildVirtualInputNotice(warning, title, messages, options = {}) {
        return {
          action: options.action || 'index.review_virtual_input',
          alertClass: warning ? 'alert-warning' : 'alert-info',
          buttonClass: warning ? 'btn-warning' : 'btn-info',
          chooseDriver: options.chooseDriver || false,
          to: options.to || '/troubleshooting#virtualhid',
          messages,
          title,
          warning,
        };
      },
      /**
       * Build the notice for an explicitly selected ViGEmBus backend.
       *
       * @param {boolean} vigembusUsable Whether ViGEmBus is installed and compatible.
       * @returns {object|null} Warning data, or no notice when ViGEmBus is usable.
       */
      buildVigembusNotice(vigembusUsable) {
        if (!this.controllerEnabled || vigembusUsable) {
          return null;
        }
        return this.buildVirtualInputNotice(true, 'index.vigembus_required_title', [{
          key: this.vigembus.installed ? 'index.vigembus_outdated_desc' : 'index.vigembus_not_installed_desc',
          params: { version: this.vigembus.version, supported_versions: this.vigembus.supported_versions },
        }]);
      },
      /**
       * Build the notice for an installed Virtual HID Driver.
       *
       * @param {boolean} vigembusUsable Whether ViGEmBus is available as a fallback.
       * @returns {object|null} Warning or informational data, or no notice when fully usable.
       */
      buildInstalledVirtualhidNotice(vigembusUsable) {
        const messages = [];
        if (!this.virtualhid.version_compatible && !this.virtualhid.development_version) {
          messages.push({
            key: 'index.virtualhid_outdated_desc',
            params: { version: this.virtualhid.version, supported_versions: this.virtualhid.supported_versions },
          });
        }
        if (this.virtualhidLicense && !this.virtualhidLicense.licensed) {
          const licenseMessageKey = this.gamepadDriver === 'all' && vigembusUsable ?
            'index.virtualhid_license_invalid_fallback_desc' :
            'index.virtualhid_license_invalid_desc';
          messages.push({ key: licenseMessageKey });
        }
        if (messages.length) {
          return this.buildVirtualInputNotice(true, 'index.virtualhid_attention_title', messages);
        }
        if (this.virtualhid.development_version) {
          return this.buildVirtualInputNotice(false, 'index.virtualhid_development_title', [{ key: 'index.virtualhid_development_desc' }]);
        }
        return null;
      },
      convertMarkdownToHtml(markdown) {
        if (!markdown) return '';
        return marked.parse(markdown);
      }
    }
  }
</script>
