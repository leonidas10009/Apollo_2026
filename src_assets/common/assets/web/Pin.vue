<template>
  <div id="content">
    <div class="page-header justify-content-center text-center">
      <div class="w-100">
        <h1 class="page-header-title">{{ currentTab === 'otp' ? $t('pin.otp_pairing') : $t('pin.pin_pairing') }}</h1>
      </div>
    </div>
    <div class="pin-panel">
    <ul class="nav nav-pills justify-content-center mb-3">
      <li class="nav-item">
        <button class="nav-link" :class="{active: currentTab === 'otp'}" @click="currentTab = 'otp'">
          {{ $t('pin.otp_pairing') }}
        </button>
      </li>
      <li class="nav-item">
        <button class="nav-link" :class="{active: currentTab === 'pin'}" @click="currentTab = 'pin'">
          {{ $t('pin.pin_pairing') }}
        </button>
      </li>
    </ul>
    <form v-if="currentTab === 'pin'" class="form d-flex flex-column align-items-center pin-form" id="form" @submit.prevent="registerDevice">
      <div class="card flex-column d-flex p-4 mb-4 w-100">
        <div class="input-group mt-2">
          <label for="pairing-input" class="visually-hidden">{{ $t('pin.select_pairing') }}</label>
          <span class="input-group-text">
            <user-round-search :size="18" class="icon"></user-round-search>
          </span>
          <select v-model="selectedPairingId" class="form-select" id="pairing-input" required>
            <option disabled value="">
              {{ pendingPairings.length ? $t('pin.select_pairing') : $t('pin.no_pending_pairings') }}
            </option>
            <option v-for="pairing in pendingPairings" :key="pairing.id" :value="pairing.id">
              {{ pairing.name || $t('pin.unknown_device') }} — {{ pairing.address || $t('pin.unknown_address') }}
            </option>
          </select>
          <button
            type="button"
            class="btn btn-outline-danger"
            :disabled="!selectedPairingId"
            :title="$t('pin.cancel_pairing')"
            @click="cancelSelectedPairing"
          >
            <x :size="18" class="icon"></x>
          </button>
        </div>
        <div class="input-group mt-2">
          <label for="pin-input" class="visually-hidden">{{ $t('navbar.pin') }}</label>
          <span class="input-group-text">
            <hash :size="18" class="icon"></hash>
          </span>
          <input v-model="pin" type="text" pattern="\d{4}" maxlength="4" inputmode="numeric" :placeholder="`${$t('navbar.pin')}`" id="pin-input" class="form-control" required />
        </div>
        <div class="input-group my-4">
          <label for="name-input" class="visually-hidden">{{ $t('pin.device_name') }}</label>
          <span class="input-group-text">
            <monitor :size="18" class="icon"></monitor>
          </span>
          <input v-model="name" type="text" :placeholder="`${$t('pin.device_name')}`" id="name-input" class="form-control" required />
        </div>
        <button type="submit" class="btn btn-primary">
          <forward :size="18" class="icon"></forward>
          {{ $t('pin.send') }}
        </button>
      </div>
    </form>
    <form v-else class="form d-flex flex-column align-items-center pin-form" @submit.prevent="requestOTP">
      <div class="card flex-column d-flex p-4 mb-4 w-100">
        <div class="input-group mt-2">
          <label for="otp-passphrase" class="visually-hidden">{{ $t('pin.otp_passphrase') }}</label>
          <span class="input-group-text">
            <key-round :size="18" class="icon"></key-round>
          </span>
          <input v-model="passphrase" type="text" pattern="[0-9a-zA-Z]{4,}"
                 :placeholder="$t('pin.otp_passphrase')" id="otp-passphrase" class="form-control" required />
        </div>
        <div class="input-group my-4">
          <label for="otp-device-name" class="visually-hidden">{{ $t('pin.device_name') }}</label>
          <span class="input-group-text">
            <monitor :size="18" class="icon"></monitor>
          </span>
          <input v-model="deviceName" type="text" :placeholder="$t('pin.device_name')"
                 id="otp-device-name" class="form-control" />
        </div>
        <button type="submit" class="btn btn-primary">
          <key-round :size="18" class="icon"></key-round>
          {{ $t('pin.generate_pin') }}
        </button>
      </div>
      <div v-if="otp || otpExpired" class="card flex-column d-flex p-4 mb-4 text-center">
        <div v-show="otp && hostAddr" ref="qrContainer" class="pin-qr bg-white p-2 rounded mx-auto mb-3"></div>
        <h2 class="display-4">{{ otpExpired ? $t('pin.otp_expired') : otp }}</h2>
        <div v-if="otp && hostAddr && !editingHost" class="d-flex flex-column align-items-center gap-2">
          <div class="d-flex align-items-center justify-content-center gap-2">
            <a :href="deepLink" class="text-break">art://{{ hostAddr }}:{{ hostPort }}</a>
            <button type="button" class="btn btn-sm btn-outline-secondary" :title="$t('apps.edit')"
                    :aria-label="$t('apps.edit')" @click="editHost">
              <pencil :size="16" class="icon"></pencil>
            </button>
          </div>
          <button v-if="!isLocalHost" type="button" class="btn btn-primary" @click="pairNow">
            {{ $t('pin.pair_now') }}
          </button>
        </div>
        <div v-else-if="otp && editingHost" class="d-flex flex-column gap-2">
          <label class="visually-hidden" for="otp-host-address">{{ $t('pin.host_address') }}</label>
          <input id="otp-host-address" v-model="hostAddr" class="form-control" :placeholder="$t('pin.host_address')" />
          <label class="visually-hidden" for="otp-host-port">{{ $t('pin.host_port') }}</label>
          <input id="otp-host-port" v-model="hostPort" inputmode="numeric" class="form-control" :placeholder="$t('pin.host_port')" />
          <button type="button" class="btn btn-primary" :disabled="!canSaveHost" @click="saveHost">
            {{ $t('_common.save') }}
          </button>
        </div>
      </div>
      <div v-if="otpMessage" class="alert" :class="`alert-${otpStatus}`">{{ otpMessage }}</div>
      <div class="alert alert-info">{{ $t('pin.otp_msg') }}</div>
    </form>
    <div class="alert alert-warning">
      <b>{{ $t('_common.warning') }}</b> {{ $t('pin.warning_msg') }}
    </div>
    <RouterLink class="btn btn-outline-primary mb-3" to="/clients">
      {{ $t('pin.device_management') }}
    </RouterLink>
    <div v-if="status" :class="`alert alert-${status.type}`" role="alert">{{ status.message }}</div>
    </div>
  </div>
</template>

<script>
  import { apiFetch } from './fetch_utils'
  import {
    Forward,
    Hash,
    KeyRound,
    Monitor,
    Pencil,
    UserRoundSearch,
    X,
  } from '@lucide/vue'

  export default {
    components: {
      Forward,
      Hash,
      KeyRound,
      Monitor,
      Pencil,
      UserRoundSearch,
      X,
    },
    inject: ['i18n'],
    data() {
      return {
        currentTab: 'pin',
        deviceName: '',
        editingHost: false,
        hostAddr: '',
        hostName: '',
        hostPort: '',
        isLocalHost: false,
        name: '',
        otp: '',
        otpExpired: false,
        otpMessage: '',
        otpStatus: 'warning',
        otpTimer: null,
        passphrase: '',
        pendingPairings: [],
        pin: '',
        qrRenderedFor: '',
        refreshTimer: null,
        selectedPairingId: '',
        status: null,
      };
    },
    computed: {
      deepLink() {
        return `art://${this.hostAddr}:${this.hostPort}?pin=${encodeURIComponent(this.otp)}&passphrase=${encodeURIComponent(this.passphrase)}&name=${encodeURIComponent(this.hostName)}`;
      },
      /**
       * A host can only be saved when the address is present and the port is a valid TCP port.
       */
      canSaveHost() {
        const host = String(this.hostAddr || '').trim();
        const portText = String(this.hostPort || '').trim();
        if (!host || host.includes('://') || /[\s/?#]/.test(host)) {
          return false;
        }
        if (!/^\d{1,5}$/.test(portText)) {
          return false;
        }
        const port = Number(portText);
        return port >= 1 && port <= 65535;
      },
    },
    watch: {
      deepLink() {
        this.$nextTick(() => this.renderQR());
      },
    },
    mounted() {
      this.isLocalHost = ['localhost', '127.0.0.1', '[::1]'].includes(globalThis.location?.hostname);
      this.loadPendingPairings();
      this.refreshTimer = window.setInterval(() => this.loadPendingPairings(), 2000);
    },
    beforeUnmount() {
      window.clearInterval(this.refreshTimer);
      this.clearOtpExpiryTimer();
    },
    methods: {
      /**
       * Refresh the authenticated list of pairing requests awaiting approval.
       */
      async loadPendingPairings() {
        try {
          const response = await apiFetch('./api/pin', {method: 'GET'});
          if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
          }

          const body = await response.json();
          this.pendingPairings = body.pairings || [];
          if (!this.pendingPairings.some((pairing) => pairing.id === this.selectedPairingId)) {
            this.selectedPairingId = this.pendingPairings.length === 1 ? this.pendingPairings[0].id : '';
          }
        } catch (error) {
          console.error('Failed to load pending pairing requests', error);
          this.status = {type: 'danger', message: this.i18n.t('pin.load_failure')};
        }
      },

      /**
       * Read the host the operator previously confirmed, if any.
       */
      readHostCache() {
        try {
          const cached = JSON.parse(globalThis.sessionStorage?.getItem('hostInfo'));
          if (cached && typeof cached.hostAddr === 'string' && cached.hostAddr) {
            return cached;
          }
        } catch (error) {
          console.debug('Ignoring invalid cached host information', error);
        }
        return null;
      },

      /**
       * Remember a manually confirmed host for the rest of the browser session.
       */
      writeHostCache() {
        try {
          globalThis.sessionStorage?.setItem('hostInfo', JSON.stringify({
            hostAddr: this.hostAddr,
            hostPort: String(this.hostPort),
          }));
        } catch (error) {
          console.debug('Unable to cache host information', error);
        }
      },

      /**
       * Draw the pairing QR code for the current deep link.
       */
      renderQR() {
        const container = this.$refs.qrContainer;
        if (!container || !this.otp || !this.hostAddr || typeof QRCode === 'undefined') {
          return;
        }

        const deepLink = this.deepLink;
        if (this.qrRenderedFor === deepLink) {
          return;
        }

        container.innerHTML = '';
        // qrcodejs renders a canvas and an img fallback into the supplied container.
        new QRCode(container, {
          text: deepLink,
          width: 192,
          height: 192,
          correctLevel: QRCode.CorrectLevel.M,
        });
        this.qrRenderedFor = deepLink;
      },

      /**
       * Toggle manual host editing and restore the last confirmed values.
       */
      editHost() {
        if (!this.editingHost) {
          const cached = this.readHostCache();
          if (cached) {
            this.hostAddr = cached.hostAddr;
            this.hostPort = cached.hostPort;
          }
        }
        this.editingHost = !this.editingHost;
      },

      /**
       * Confirm the manually entered host and refresh the QR code.
       */
      saveHost() {
        if (!this.canSaveHost) {
          return;
        }
        this.hostAddr = String(this.hostAddr).trim();
        this.hostPort = String(Number.parseInt(this.hostPort, 10));
        this.writeHostCache();
        this.editingHost = false;
        this.$nextTick(() => this.renderQR());
      },

      /**
       * Open the Artemis deep link from an explicit user click so browsers do not block the popup.
       */
      pairNow() {
        if (this.deepLink) {
          globalThis.open(this.deepLink, '_blank', 'noopener');
        }
      },

      /**
       * Expire the displayed OTP after the three minutes the server honours it.
       */
      startOtpExpiryTimer() {
        this.clearOtpExpiryTimer();
        this.otpTimer = globalThis.setTimeout(() => {
          this.otpTimer = null;
          this.otp = '';
          this.otpExpired = true;
          this.qrRenderedFor = '';
          this.otpStatus = 'warning';
          this.otpMessage = this.i18n.t('pin.otp_expired_msg');
        }, 3 * 60 * 1000);
      },

      clearOtpExpiryTimer() {
        if (this.otpTimer !== null) {
          globalThis.clearTimeout(this.otpTimer);
          this.otpTimer = null;
        }
      },

      /**
       * Request a one-time pairing PIN for Artemis clients.
       */
      async requestOTP() {
        if (this.editingHost) {
          return;
        }

        this.status = null;
        this.otpMessage = '';
        this.otp = '';
        this.otpExpired = false;
        this.qrRenderedFor = '';
        this.clearOtpExpiryTimer();

        try {
          const response = await apiFetch('./api/otp', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json'
            },
            body: JSON.stringify({
              passphrase: this.passphrase,
              deviceName: this.deviceName,
            }),
          });
          const result = await response.json();
          if (!result.status) {
            this.otpStatus = 'danger';
            this.otpMessage = result.error || this.i18n.t('pin.otp_expired_msg');
            return;
          }

          this.otp = result.otp;
          this.hostName = result.name || '';
          const cachedHost = this.readHostCache();
          if (cachedHost) {
            this.hostAddr = cachedHost.hostAddr;
            this.hostPort = String(cachedHost.hostPort);
          } else {
            this.hostAddr = this.isLocalHost ? (result.ip || globalThis.location.hostname) : globalThis.location.hostname;
            const webPort = Number.parseInt(globalThis.location.port, 10);
            this.hostPort = Number.isFinite(webPort) && webPort > 0 ? String(webPort - 1) : '47989';
          }
          this.otpStatus = 'success';
          this.otpMessage = this.i18n.t('pin.otp_success');
          this.startOtpExpiryTimer();
        } catch (error) {
          console.error('Failed to request OTP', error);
          this.otpStatus = 'danger';
          this.otpMessage = this.i18n.t('pin.otp_expired_msg');
        }
      },

      /**
       * Apply the entered PIN only to the pairing request selected by the operator.
       */
      async registerDevice() {
        this.status = null;
        if (!this.selectedPairingId) {
          this.status = {type: 'danger', message: this.i18n.t('pin.select_pairing_required')};
          return;
        }

        const body = JSON.stringify({
          pairing_id: this.selectedPairingId,
          pin: this.pin,
          name: this.name,
        });
        const response = await apiFetch('./api/pin', {
          method: "POST",
          headers: {
            'Content-Type': 'application/json'
          },
          body,
        });
        const result = await response.json();
        if (result.status === true) {
          this.status = {type: 'success', message: this.i18n.t('pin.pair_success')};
          this.pin = '';
          this.name = '';
        } else {
          this.status = {type: 'danger', message: this.i18n.t('pin.pair_failure')};
        }
        await this.loadPendingPairings();
      },

      /**
       * Cancel the selected pending request without affecting other clients.
       */
      async cancelSelectedPairing() {
        if (!this.selectedPairingId) {
          return;
        }

        const response = await apiFetch('./api/pin', {
          method: 'DELETE',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({pairing_id: this.selectedPairingId}),
        });
        const result = await response.json();
        this.status = result.status === true
          ? {type: 'success', message: this.i18n.t('pin.cancel_success')}
          : {type: 'danger', message: this.i18n.t('pin.cancel_failure')};
        await this.loadPendingPairings();
      }
    }
  }
</script>

<style scoped>
.pin-qr {
  width: fit-content;
}

.pin-qr :deep(img),
.pin-qr :deep(canvas) {
  display: block;
}
</style>
