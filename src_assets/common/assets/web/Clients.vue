<template>
  <Navbar></Navbar>
  <div id="content" class="container">
    <div class="card my-4">
      <div class="card-body">
        <div class="d-flex justify-content-end align-items-center">
          <h1 class="me-auto mb-0">{{ $t('pin.device_management') }}</h1>
          <button class="btn btn-danger" :disabled="unpairAllPressed" @click="unpairAll">
            {{ $t('pin.unpair_all') }}
          </button>
        </div>
        <p class="mb-0 mt-3">{{ $t('pin.device_management_desc') }}</p>
        <p class="mb-0">
          {{ $t('pin.device_management_warning') }}
          <a href="https://github.com/ClassicOldSong/Apollo/wiki/Permission-System" target="_blank">
            {{ $t('_common.learn_more') }}
          </a>
        </p>
        <div class="alert alert-success mt-3" v-if="unpairAllStatus === true">
          {{ $t('pin.unpair_all_success') }}
        </div>
        <div class="alert alert-danger mt-3" v-if="unpairAllStatus === false">
          {{ $t('pin.unpair_all_error') }}
        </div>
        <div class="alert alert-danger mt-3" v-if="status">{{ status }}</div>
      </div>

      <ul class="list-group list-group-flush" v-if="clients.length > 0">
        <li v-for="client in clients" :key="client.uuid" class="list-group-item">
          <div v-if="client.editing" class="d-flex flex-column">
            <div class="d-flex align-items-center">
              <span class="badge me-2" :class="client.editPerm >= permissionMapping.launch ? 'text-bg-danger' : 'text-bg-primary'">
                [ {{ permToStr(client.editPerm) }} ]
              </span>
              <input v-model="client.editName" class="form-control flex-grow-1" type="text"
                     :placeholder="$t('pin.device_name')" @keyup.enter="saveClient(client)" />
              <button class="btn btn-success ms-2" :title="$t('_common.save')" @click="saveClient(client)">
                <Check :size="18" class="icon"></Check>
              </button>
              <button class="btn btn-secondary ms-2" :title="$t('_common.cancel')" @click="cancelEdit(client)">
                <X :size="18" class="icon"></X>
              </button>
            </div>

            <div class="d-flex flex-wrap justify-content-center mt-3">
              <div v-for="group in permissionGroups" :key="group.key" class="d-flex flex-column mx-2 mb-2">
                <div class="mx-2 fw-semibold">{{ $t(group.label) }}:</div>
                <button v-for="perm in group.permissions" :key="perm.name"
                        class="my-1 btn btn-sm"
                        :disabled="isSuppressed(client.editPerm, perm.name, perm.suppressed_by)"
                        :class="(isSuppressed(client.editPerm, perm.name, perm.suppressed_by) || checkPermission(client.editPerm, perm.name)) ? 'btn-success' : 'btn-outline-secondary'"
                        @click="togglePermission(client, perm.name)">
                  {{ $t(`permissions.${perm.name}`) }}
                </button>
              </div>
            </div>

            <Checkbox class="mb-3"
                      id="enable_legacy_ordering"
                      label="pin.enable_legacy_ordering"
                      desc="pin.enable_legacy_ordering_desc"
                      v-model="client.editEnableLegacyOrdering"
                      default="true"
            ></Checkbox>

            <Checkbox v-if="platform === 'windows'"
                      class="mb-3"
                      id="always_use_virtual_display"
                      label="pin.always_use_virtual_display"
                      desc="pin.always_use_virtual_display_desc"
                      v-model="client.editAlwaysUseVirtualDisplay"
                      default="false"
            ></Checkbox>

            <div class="mb-3 mt-2">
              <label :for="`display_mode_override_${client.uuid}`" class="form-label">
                {{ $t('pin.display_mode_override') }}
              </label>
              <input type="text" class="form-control" :id="`display_mode_override_${client.uuid}`"
                     v-model="client.editDisplayMode" placeholder="1920x1080x59.94"
                     @input="validateModeOverride" />
              <div class="form-text">
                {{ $t('pin.display_mode_override_desc') }}
                <a href="https://github.com/ClassicOldSong/Apollo/wiki/Display-Mode-Override" target="_blank">
                  {{ $t('_common.learn_more') }}
                </a>
              </div>
            </div>

            <Checkbox class="mb-3"
                      id="allow_client_commands"
                      label="pin.allow_client_commands"
                      desc="pin.allow_client_commands_desc"
                      v-model="client.editAllowClientCommands"
                      default="true"
            ></Checkbox>

            <template v-for="cmdType in ['do', 'undo']" :key="cmdType">
              <div v-if="client.editAllowClientCommands" class="mb-3 mt-2 d-flex flex-column">
                <label class="mb-0 form-label">{{ $t(`pin.client_${cmdType}_cmd`) }}</label>
                <div class="form-text">
                  {{ $t(`pin.client_${cmdType}_cmd_desc`) }}
                  <a href="https://github.com/ClassicOldSong/Apollo/wiki/Client-Commands" target="_blank">
                    {{ $t('_common.learn_more') }}
                  </a>
                </div>
                <table class="mt-2 table" v-if="client[`edit_${cmdType}`].length > 0">
                  <thead>
                    <tr>
                      <th scope="col">{{ $t('_common.cmd_val') }}</th>
                      <th scope="col" v-if="platform === 'windows'">{{ $t('_common.run_as') }}</th>
                      <th scope="col"></th>
                    </tr>
                  </thead>
                  <tbody>
                    <tr v-for="(command, index) in client[`edit_${cmdType}`]" :key="index">
                      <td>
                        <input type="text" class="form-control monospace" v-model="command.cmd" />
                      </td>
                      <td v-if="platform === 'windows'" class="align-middle">
                        <Checkbox :id="`client-${cmdType}-cmd-admin-${index}`"
                                  label="_common.elevated"
                                  desc=""
                                  v-model="command.elevated"
                        ></Checkbox>
                      </td>
                      <td class="align-middle">
                        <button class="btn btn-danger btn-sm me-2" :title="$t('_common.delete')"
                                @click="removeCmd(client[`edit_${cmdType}`], index)">
                          <Trash2 :size="16" class="icon"></Trash2>
                        </button>
                        <button class="btn btn-success btn-sm" :title="$t('_common.add')"
                                @click="addCmd(client[`edit_${cmdType}`], index)">
                          <Plus :size="16" class="icon"></Plus>
                        </button>
                      </td>
                    </tr>
                  </tbody>
                </table>
                <button class="btn btn-success align-self-start mt-2" @click="addCmd(client[`edit_${cmdType}`], -1)">
                  <Plus :size="18" class="icon"></Plus>
                  {{ $t('apps.add_cmds') }}
                </button>
              </div>
            </template>
          </div>

          <div v-else class="d-flex align-items-center">
            <span class="badge me-2" :class="client.perm >= permissionMapping.launch ? 'text-bg-danger' : 'text-bg-primary'">
              [ {{ permToStr(client.perm) }} ]
            </span>
            <span class="me-2">{{ client.name !== "" ? client.name : $t('pin.unpair_single_unknown') }}</span>
            <span v-if="client.connected" class="badge text-bg-success">{{ $t('pin.connected') }}</span>
            <div class="ms-auto d-flex">
              <button v-if="client.connected" class="btn btn-warning btn-sm me-2" :title="$t('pin.disconnect')"
                      @click="disconnectClient(client.uuid)">
                <Unplug :size="16" class="icon"></Unplug>
              </button>
              <button class="btn btn-primary btn-sm me-2" :title="$t('pin.edit_client')" @click="editClient(client)">
                <Pencil :size="16" class="icon"></Pencil>
              </button>
              <button class="btn btn-danger btn-sm" :title="$t('pin.unpair_client')" @click="unpairSingle(client.uuid)">
                <Trash2 :size="16" class="icon"></Trash2>
              </button>
            </div>
          </div>
        </li>
      </ul>
      <div v-else class="list-group-item p-3 text-center">
        <em>{{ $t('pin.unpair_single_no_devices') }}</em>
      </div>
    </div>
  </div>
</template>

<script>
  import Navbar from './Navbar.vue'
  import Checkbox from './Checkbox.vue'
  import { apiFetch } from './fetch_utils'
  import {
    Check,
    Pencil,
    Plus,
    Trash2,
    Unplug,
    X,
  } from '@lucide/vue'

  const permissionMapping = {
    input_controller: 0x00000100,
    input_touch: 0x00000200,
    input_pen: 0x00000400,
    input_mouse: 0x00000800,
    input_kbd: 0x00001000,
    _all_inputs: 0x00001F00,

    clipboard_set: 0x00010000,
    clipboard_read: 0x00020000,
    file_upload: 0x00040000,
    file_dwnload: 0x00080000,
    server_cmd: 0x00100000,
    _all_operations: 0x001F0000,

    list: 0x01000000,
    view: 0x02000000,
    launch: 0x04000000,
    _allow_view: 0x06000000,
    _all_actions: 0x07000000,

    _default: 0x03000000,
    _no: 0x00000000,
    _all: 0x071F1F00,
  }

  const permissionGroups = [
    {
      key: 'action',
      label: 'permissions.group_action',
      permissions: [
        {name: 'list', suppressed_by: ['view', 'launch']},
        {name: 'view', suppressed_by: ['launch']},
        {name: 'launch', suppressed_by: []},
      ],
    },
    {
      key: 'operation',
      label: 'permissions.group_operation',
      permissions: [
        {name: 'clipboard_set', suppressed_by: []},
        {name: 'clipboard_read', suppressed_by: []},
        {name: 'file_upload', suppressed_by: []},
        {name: 'file_dwnload', suppressed_by: []},
        {name: 'server_cmd', suppressed_by: []},
      ],
    },
    {
      key: 'input',
      label: 'permissions.group_input',
      permissions: [
        {name: 'input_controller', suppressed_by: []},
        {name: 'input_touch', suppressed_by: []},
        {name: 'input_pen', suppressed_by: []},
        {name: 'input_mouse', suppressed_by: []},
        {name: 'input_kbd', suppressed_by: []},
      ],
    },
  ]

  export default {
    components: {
      Navbar,
      Checkbox,
      Check,
      Pencil,
      Plus,
      Trash2,
      Unplug,
      X,
    },
    inject: ['i18n'],
    data() {
      return {
        clients: [],
        platform: '',
        permissionGroups,
        permissionMapping,
        status: '',
        unpairAllPressed: false,
        unpairAllStatus: null,
      }
    },
    mounted() {
      this.refreshClients()
    },
    methods: {
      refreshClients() {
        apiFetch('./api/clients/list')
          .then(response => response.json())
          .then(response => {
            this.platform = response.platform || ''
            if (response.status === true && response.named_certs && response.named_certs.length) {
              this.clients = response.named_certs.map(client => ({
                ...client,
                perm: Number.parseInt(client.perm, 10) || 0,
                connected: !!client.connected,
                editing: false,
                editPerm: Number.parseInt(client.perm, 10) || 0,
                editName: client.name || '',
                editDisplayMode: client.display_mode || '',
                editAllowClientCommands: client.allow_client_commands !== false,
                editEnableLegacyOrdering: client.enable_legacy_ordering !== false,
                editAlwaysUseVirtualDisplay: client.always_use_virtual_display === true,
                edit_do: structuredClone(client.do || []),
                edit_undo: structuredClone(client.undo || []),
              }))
            } else {
              this.clients = []
            }
          })
          .catch(error => {
            console.error(error)
            this.status = this.i18n.t('pin.load_failure')
          })
      },
      permToStr(perm) {
        return [24, 16, 8].map(shift => ((perm >> shift) & 0xFF).toString(16).toUpperCase().padStart(2, '0')).join(' ')
      },
      checkPermission(perm, permission) {
        return (perm & permissionMapping[permission]) !== 0
      },
      isSuppressed(perm, permission, suppressedBy) {
        return suppressedBy.some(suppressed => this.checkPermission(perm, suppressed))
      },
      togglePermission(client, permission) {
        client.editPerm ^= permissionMapping[permission]
      },
      validateModeOverride(event) {
        const value = event.target.value.trim()
        event.target.setCustomValidity(value && !value.match(/^\d+x\d+x\d+(\.\d+)?$/) ? this.i18n.t('pin.display_mode_override_error') : '')
        event.target.reportValidity()
      },
      editClient(client) {
        this.clients.filter(other => other.editing).forEach(other => this.cancelEdit(other))
        client.editing = true
        client.editPerm = client.perm
        client.editName = client.name || ''
        client.editDisplayMode = client.display_mode || ''
        client.editAllowClientCommands = client.allow_client_commands !== false
        client.editEnableLegacyOrdering = client.enable_legacy_ordering !== false
        client.editAlwaysUseVirtualDisplay = client.always_use_virtual_display === true
        client.edit_do = structuredClone(client.do || [])
        client.edit_undo = structuredClone(client.undo || [])
      },
      cancelEdit(client) {
        client.editing = false
      },
      addCmd(commands, index) {
        const command = {cmd: '', elevated: false}
        if (index < 0) {
          commands.push(command)
        } else {
          commands.splice(index + 1, 0, command)
        }
      },
      removeCmd(commands, index) {
        commands.splice(index, 1)
      },
      serializeCommands(commands) {
        return (commands || [])
          .filter(command => command.cmd && command.cmd.trim())
          .map(command => ({cmd: command.cmd.trim(), elevated: !!command.elevated}))
      },
      saveClient(client) {
        this.status = ''
        const body = {
          uuid: client.uuid,
          name: client.editName || '',
          display_mode: client.editDisplayMode.trim(),
          allow_client_commands: client.editAllowClientCommands,
          enable_legacy_ordering: client.editEnableLegacyOrdering,
          always_use_virtual_display: client.editAlwaysUseVirtualDisplay,
          perm: client.editPerm & permissionMapping._all,
          do: this.serializeCommands(client.edit_do),
          undo: this.serializeCommands(client.edit_undo),
        }
        apiFetch('./api/clients/update-details', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify(body),
        })
          .then(response => response.json())
          .then(response => {
            if (response.status) {
              client.editing = false
              this.refreshClients()
            } else {
              throw new Error(this.i18n.t('pin.save_client_error'))
            }
          })
          .catch(error => {
            this.status = error.message || this.i18n.t('pin.save_client_error')
          })
      },
      disconnectClient(uuid) {
        apiFetch('./api/clients/disconnect', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({uuid}),
        }).finally(() => this.refreshClients())
      },
      unpairSingle(uuid) {
        apiFetch('./api/clients/unpair', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({uuid}),
        }).finally(() => this.refreshClients())
      },
      unpairAll() {
        this.unpairAllPressed = true
        apiFetch('./api/clients/unpair-all', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
        })
          .then(response => response.json())
          .then(response => {
            this.unpairAllStatus = response.status
            setTimeout(() => { this.unpairAllStatus = null }, 5000)
            this.refreshClients()
          })
          .finally(() => { this.unpairAllPressed = false })
      },
    },
  }
</script>
