# BlackBerry Passport (SQW100-1) VoLTE Revival & Patch Suite

[![Target Device](https://img.shields.io/badge/Device-BlackBerry%20Passport%20(SQW100--1)-blue.svg)](#)
[![Target OS](https://img.shields.io/badge/BB10%20OS-10.3.3.3216-green.svg)](#)
[![VoLTE](https://img.shields.io/badge/Feature-VoLTE%20%2F%204G%20Calling-orange.svg)](#)
[![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)](#)

A reverse-engineered binary patch utility and deployment suite enabling native **VoLTE (Voice over LTE)** calling on rooted **BlackBerry Passport** (`SQW100-1`) devices on modern 4G LTE networks (tested with Viettel Vietnam MCC `452`, MNC `04`, as well as standard 3GPP / IR.92 IMS networks).

---

## 📌 Background & Motivation

With 2G and 3G networks shutting down across many countries, BlackBerry 10 devices face immediate voice service loss because stock BB10 firmware fails to negotiate VoLTE calls on most carriers, even when 4G data is connected.

Analysis of `/radio/bin/ims_rrm` on BlackBerry OS 10.3.3.3216 revealed multiple software barriers preventing IMS registration and outgoing call establishment:
1. **PPS Carrier Config Path Hardcoding**: The stock daemon loads operator profiles strictly from `/pps/services/carrier_config/radio_qct/ims`.
2. **VoPS=0 Network Veto**: When a base station broadcasts `VoPS = 0` (Voice over PS not supported in SIB2), `selectDomainVoice()` aborts VoLTE and forces CSFB to unavailable 2G/3G networks.
3. **Precondition Policy Incompatibility**: Outgoing SIP `INVITE` sessions fail unless `Precondition.Policy` is enforced to `volte`.
4. **Hardcoded Carrier Whitelist**: Precondition negotiation (`UsePreconditions = 1`) is locked behind hardcoded carrier whitelist checks.
5. **Slow/Missing QCI=1 UPDATE**: On carriers where QCI=1 dedicated bearer establishment is deferred until answer, `precondStep()` waits up to 8000ms for bearer state changes before sending SIP `UPDATE`, causing call attempts to time out without the recipient phone ringing.

---

## 🔍 The 5 Reverse-Engineered Binary Patches

All 5 binary patches target the ARMv7 Thumb-2 binary `/radio/bin/ims_rrm`:

| # | Patch Name | Offset (Hex) | Stock Bytes | Patched Bytes | Disassembly (Stock $\to$ Patched) | Description |
|---|------------|--------------|-------------|---------------|-----------------------------------|-------------|
| 1 | **PPS Config Redirection** | `0x366bed` | `/pps/services/...` | `/pps/volte_probe/...` | ASCII Path string redirection | Redirects PPS config read to `/pps/volte_probe/ims` to load custom IR.92 operator profiles. |
| 2 | **VoPS Veto Bypass** | `0x62c54` | `c7 b9` | `18 e0` | `cbnz r7, 0x62c88` $\to$ `b 0x62c88` | Bypasses VoPS=0 network rejection in `selectDomainVoice` to enable VoLTE on non-VoPS cells. |
| 3 | **Precondition Policy Force** | `0xb8e6e` | `20 b1` | `00 bf` | `cbz r0, 0xb8e7a` $\to$ `nop` | Enforces `Precondition.Policy = "volte"` during SIP UA client configuration. |
| 4 | **Precondition Whitelist Bypass** | `0x114104` | `34 d0` | `34 e0` | `beq 0x114170` $\to$ `b 0x114170` | Bypasses carrier whitelist in `ImsPpsCarrierCfgIr92Object` to enable `UsePreconditions = 1` for any carrier. |
| 5 | **Fast UPDATE Trigger** | `0x19eb6e` | `3b d0` | `00 bf` | `beq 0x19ebe8` $\to$ `nop` | Bypasses 8000ms bearer wait in `precondStep()`, immediately sending SIP `UPDATE` upon `183 Session Progress` so the phone rings. |

### Binary Integrity Verification
* **Stock SQW100-1 `ims_rrm` SHA256:**  
  `043d38b0f08cf6a9696ef405f3976679e32ff9ad943ec35294c3d3da69a12b67`
* **Patched `ims_rrm` SHA256:**  
  `3353f44609206b8ebcb138ad2b39752af76339e94a839cc0699842608b73b3c2`

---

## 📂 Repository Contents

* **[`apply_volte_patch.py`](apply_volte_patch.py)**: Standalone Python 3 script that reads stock `ims_rrm`, applies all 5 patches, verifies SHA256 hashes, and outputs the patched binary.
* **[`volte_patches.json`](volte_patches.json)**: Machine-readable JSON specification containing patch offsets, byte sequences, and functional documentation.
* **[`main.c`](main.c)**: Clean C source code for the Windows deployer and installer (`PassportVoLTEPatcher.exe`).
* **[`embed_assets.py`](embed_assets.py)**: Utility to convert SSH private keys and payload archives into an embedded C header for building single-file standalone patcher binaries.
* **[`deploy_to_passport.sh`](deploy_to_passport.sh)**: Shell script to push and start the VoLTE runtime on a rooted BlackBerry Passport over SSH/SCP.

---

## 🚀 Usage

### 1. Standalone Python Patcher
You can patch an extracted stock `ims_rrm` directly:
```bash
python3 apply_volte_patch.py /path/to/stock/ims_rrm /path/to/output/ims_rrm
```

### 2. Direct Deployment via Shell
To deploy to a rooted Passport connected via Wi-Fi or USB:
```bash
./deploy_to_passport.sh 192.168.0.101 ~/.ssh/id_rsa
```

### 3. Compiling the Windows Deployer (`main.c`)
To build the standalone Windows utility using MinGW:
```bash
# Standalone mode (looks for id_rsa and payload bundle in the current folder)
x86_64-w64-mingw32-gcc main.c -o PassportVoLTEPatcher.exe

# Or with TinyCC:
tcc main.c -o PassportVoLTEPatcher.exe
```

To build a **self-contained single-click executable** with embedded credentials:
```bash
# 1. Generate embedded header
python3 embed_assets.py path/to/id_rsa path/to/passport_volte_package.tar.gz embedded_data.h

# 2. Compile with embedded flag
x86_64-w64-mingw32-gcc -DHAVE_EMBEDDED_DATA main.c -o PassportVoLTEPatcher.exe
```

---

## 🛡️ Safety & Non-Destructive Design

* **No NAND modification**: The phone's read-only system flash (`/radio/bin/ims_rrm`) is **never modified**.
* **Runtime dynamic loading**: The patched binary executes entirely from temporary runtime memory (`/var/rootdata/q10_volte/runtime/ims_rrm`) and registers with QNX `/proc/boot/pathtrust`.
* **Safe recovery**: Simply rebooting the device restores 100% factory state.

---

## 🤝 Credits & Acknowledgements
* [Octo-o-o-o/bb10-volte](https://github.com/Octo-o-o-o/bb10-volte) - Pioneer work on BB10 VoLTE daemon architecture and runtime environment.
* [imqinyu/BB10DeviceVoLTEResearch](https://github.com/imqinyu/BB10DeviceVoLTEResearch) - BB10 IMS reverse-engineering research.
