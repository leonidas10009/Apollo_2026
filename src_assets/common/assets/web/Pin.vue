<template>
  <Navbar></Navbar>
  <div id="content" class="container">
    <h1 class="my-4 text-center">{{ currentTab === 'otp' ? $t('pin.otp_pairing') : $t('pin.pin_pairing') }}</h1>
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
    <form v-if="currentTab === 'pin'" class="form d-flex flex-column align-items-center" id="form" @submit.prevent="registerDevice">
      <div class="card flex-column d-flex p-4 mb-4">
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
    <form v-else class="form d-flex flex-column align-items-center" @submit.prevent="requestOTP">
      <div class="card flex-column d-flex p-4 mb-4">
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
      <div v-if="otp" class="card flex-column d-flex p-4 mb-4 text-center">
        <h2 class="display-4">{{ otp }}</h2>
        <a v-if="hostAddr && hostPort" :href="deepLink" class="text-break">{{ deepLink }}</a>
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
</template>

<script>
  import Navbar from './Navbar.vue'
  import { apiFetch } from './fetch_utils'
  import {
    Forward,
    Hash,
    KeyRound,
    Monitor,
    UserRoundSearch,
    X,
  } from '@lucide/vue'

  export default {
    components: {
      Navbar,
      Forward,
      Hash,
      KeyRound,
      Monitor,
      UserRoundSearch,
      X,
    },
    inject: ['i18n'],
    data() {
      return {
        currentTab: 'pin',
        deviceName: '',
        hostAddr: '',
        hostName: '',
        hostPort: '',
        name: '',
        otp: '',
        otpMessage: '',
        otpStatus: 'warning',
        passphrase: '',
        pendingPairings: [],
        pin: '',
        refreshTimer: null,
        selectedPairingId: '',
        status: null,
      };
    },
    computed: {
      deepLink() {
        return `art://${this.hostAddr}:${this.hostPort}?pin=${encodeURIComponent(this.otp)}&passphrase=${encodeURIComponent(this.passphrase)}&name=${encodeURIComponent(this.hostName)}`;
      },
    },
    mounted() {
      this.loadPendingPairings();
      this.refreshTimer = window.setInterval(() => this.loadPendingPairings(), 2000);
    },
    beforeUnmount() {
      window.clearInterval(this.refreshTimer);
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
       * Request a one-time pairing PIN for Artemis clients.
       */
      async requestOTP() {
        this.status = null;
        this.otpMessage = '';
        this.otp = '';

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
          const isLocalHost = ['localhost', '127.0.0.1', '[::1]'].includes(globalThis.location.hostname);
          this.hostAddr = isLocalHost ? (result.ip || globalThis.location.hostname) : globalThis.location.hostname;
          const webPort = Number.parseInt(globalThis.location.port, 10);
          this.hostPort = Number.isFinite(webPort) && webPort > 0 ? webPort - 1 : 47989;
          this.otpStatus = 'success';
          this.otpMessage = this.i18n.t('pin.otp_success');
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
