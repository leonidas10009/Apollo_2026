<template>
  <AppLayout v-if="!useBareLayout">
    <RouterView />
  </AppLayout>
  <RouterView v-else />
</template>

<script>
import AppLayout from './AppLayout.vue'

export default {
  components: {
    AppLayout,
  },
  computed: {
    useBareLayout() {
      return this.$route.meta.layout === 'bare'
    },
  },
}
</script>
