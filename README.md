# Retained RAM and newlib Example

This repository is based on the
[`retained-ram-newlib-example`](https://github.com/nrfconnect/retained-ram-newlib-example)
template and pinned to `nRF Connect SDK v3.3.0-rc1` in
[`west.yml`](west.yml).

The example demonstrates the two relevant retained-RAM setups on
`nrf54l15dk/nrf54l15/cpuapp`:

- Partition Manager enabled:
  `app/pm_static.yml` carves the top 4 KB out of `sram_primary`.
- Partition Manager disabled:
  `app/boards/nrf54l15dk_nrf54l15_cpuapp_no_pm.overlay` shrinks
  `&cpuapp_sram` so the same 4 KB block is no longer part of normal SRAM.

The sample also enables `newlib` with float formatting and probes `_sbrk()`
so you can see whether the heap stops at the retained-RAM boundary.

The examples below assume this repository is used as the manifest repository in
a west workspace and is checked out as `retained-ram-newlib-example` at the
workspace root.

## west patch

The `newlib` fix is bundled in this repository under `zephyr/patches.yml`.
After `west update`, apply it from the workspace root:

```sh
west patch -dm zephyr apply
```

The patch changes default `newlib` heap sizing to use the linker-defined RAM
end instead of `CONFIG_SRAM_SIZE`. That makes `_sbrk()` follow the actual RAM
limit after a Partition Manager carveout.

## Build

Partition Manager example:

```sh
source <ncs-root>/activate-nrf.sh
cd <workspace-root>
west build -p always -b nrf54l15dk/nrf54l15/cpuapp \
  retained-ram-newlib-example/app -d build/pm
```

No-Partition-Manager example:

```sh
source <ncs-root>/activate-nrf.sh
cd <workspace-root>
west build -p always -b nrf54l15dk/nrf54l15/cpuapp \
  retained-ram-newlib-example/app -d build/no-pm -- \
  -DSB_CONFIG_PARTITION_MANAGER=n \
  -DEXTRA_DTC_OVERLAY_FILE=boards/nrf54l15dk_nrf54l15_cpuapp_no_pm.overlay
```

## Flash

PM build:

```sh
west flash -d build/pm
```

No-PM build:

```sh
west flash -d build/no-pm
```

## Expected output

With the patch applied:

- PM build: `_sbrk()` stops at `__kernel_ram_end`, even though
  `CONFIG_SRAM_SIZE` still reflects full SRAM.
- No-PM build: `_sbrk()` only stops correctly when `&cpuapp_sram` is shrunk in
  devicetree.

The application prints `_end`, `__kernel_ram_end`, the retained-RAM base, and
the result of the boundary `_sbrk()` request so the behavior is visible on the
UART console.
