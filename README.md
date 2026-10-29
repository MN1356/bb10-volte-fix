# BlackBerry 10 VoLTE Revival & Patch Suite
### Native Voice over LTE Calling on BlackBerry Passport, Q10, and Classic (Q20)

[![Target Devices](https://img.shields.io/badge/Devices-Passport%20%7C%20Q10%20%7C%20Q20%20Classic-blue.svg)](#)
[![Target OS](https://img.shields.io/badge/BB10%20OS-10.3.3.3216-green.svg)](#)
[![Carriers](https://img.shields.io/badge/Carriers-Viettel%20%7C%20MobiFone%20%7C%20VinaPhone-orange.svg)](#)
[![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)](#)

A reverse-engineered binary patch utility and deployment suite enabling native **VoLTE (Voice over LTE)** calling on rooted **BlackBerry 10** devices (**BlackBerry Passport**, **BlackBerry Q10**, and **BlackBerry Classic Q20**) on modern 4G LTE networks in Vietnam (**Viettel**, **MobiFone**, **VinaPhone**), as well as standard GSMA 3GPP / IR.92 IMS networks.

---

## 📌 Background & Technical Challenge

With 2G and 3G networks shutting down across telecom operators worldwide, BlackBerry 10 devices face complete cellular voice service failure because stock BB10 firmware fails to negotiate VoLTE calls on modern networks, even while 4G LTE data works normally.

Deep binary analysis of `/radio/bin/ims_rrm` on BlackBerry OS **10.3.3.3216** revealed 5 critical software barriers preventing IMS registration and outgoing call establishment:

1. **PPS Carrier Config Path Hardcoding**: The stock daemon loads operator profiles strictly from `/pps/services/carrier_config/radio_qct/ims`.
2. **VoPS=0 Network Veto**: When a base station broadcasts `VoPS = 0` (Voice over PS not supported in SIB2), `selectDomainVoice()` aborts VoLTE and forces CSFB (Circuit Switched Fallback) to nonexistent 2G/3G networks.
3. **Precondition Policy Incompatibility**: Outgoing SIP `INVITE` sessions fail unless `Precondition.Policy` is enforced to `volte`.
4. **Hardcoded Carrier Whitelist**: Precondition negotiation (`UsePreconditions = 1`) is locked behind hardcoded carrier whitelist checks.
5. **Slow/Missing QCI=1 UPDATE**: On carriers where dedicated QCI=1 voice bearers are deferred until answer, `precondStep()` waits up to 8000ms for bearer state changes before sending the second SIP `UPDATE`, causing call attempts to time out without the recipient phone ever ringing.

---

## 🔍 The 5 Reverse-Engineered Binary Patches

All 5 binary patches target the ARMv7 Thumb-2 binary `/radio/bin/ims_rrm`:

| # | Patch Name | Offset (Hex) | Stock Bytes | Patched Bytes | Disassembly (Stock $\to$ Patched) | Description |
|---|---|---|---|---|---|---|
| 1 | **PPS Config Redirection** | `0x366bed` | `/pps/services/...` | `/pps/volte_probe/...` | ASCII Path string redirection | Redirects PPS config read to `/pps/volte_probe/ims` to load custom IR.92 operator profiles. |
| 2 | **VoPS Veto Bypass** | `0x62c54` | `c7 b9` | `18 e0` | `cbnz r7, 0x62c88` $\to$ `b 0x62c88` | Bypasses VoPS=0 network rejection in `selectDomainVoice` to enable VoLTE on non-VoPS cells. |
| 3 | **Precondition Policy Force** | `0xb8e6e` | `20 b1` | `00 bf` | `cbz r0, 0xb8e7a` $\to$ `nop` | Enforces `Precondition.Policy = "volte"` during SIP UA client configuration. |
| 4 | **Precondition Whitelist Bypass** | `0x114104` | `34 d0` | `34 e0` | `beq 0x114170` $\to$ `b 0x114170` | Bypasses carrier whitelist in `ImsPpsCarrierCfgIr92Object` to enable `UsePreconditions = 1` for any carrier. |
| 5 | **Fast UPDATE Trigger** | `0x19eb6e` | `3b d0` | `00 bf` | `beq 0x19ebe8` $\to$ `nop` | Bypasses 8000ms bearer wait in `precondStep()`, immediately sending SIP `UPDATE` upon `183 Session Progress` so the phone rings. |

### Binary Integrity Verification
* **Stock 10.3.3.3216 `ims_rrm` SHA256 (Shared across Passport, Q10, Q20):**  
  `043d38b0f08cf6a9696ef405f3976679e32ff9ad943ec35294c3d3da69a12b67`
* **Patched `ims_rrm` SHA256:**  
  `3353f44609206b8ebcb138ad2b39752af76339e94a839cc0699842608b73b3c2`

---

## 📱 Hardware & Device Compatibility

| Device Model | Model Codes | Chipset / Modem | VoLTE Status | AutoRoot Method |
|---|---|---|---|---|
| **BlackBerry Passport** | `SQW100-1`, `SQW100-2`, `SQW100-3`, `SQW100-4` | Snapdragon 800 (MSM8974-AA / MDM9x25) | ✅ Supported | Passport AutoRoot v4 |
| **BlackBerry Q10** | `SQN100-1`, `SQN100-2`, `SQN100-3`, `SQN100-4`, `SQN100-5` | Snapdragon S4 Plus (MSM8960 / MDM9x15) | ✅ Supported | `Q10_10.3.03.3216_SQN100-1-2-3-4-5` |
| **BlackBerry Classic (Q20)** | `SQC100-1`, `SQC100-2`, `SQC100-3`, `SQC100-4` | Snapdragon S4 Plus (MSM8960 / MDM9x15) | ✅ Supported | `Classic_10.3.03.3216_SQC100-1-2-3-4` |

> [!WARNING]
> **Never cross-flash AutoRoot images:** While the runtime VoLTE patches and daemon scripts are identical across all BB10 devices, the initial AutoRoot flashing stages modify the device IFS/kernel image. Always flash the AutoRoot recipe specific to your phone model.

> [!NOTE]
> **Check LTE Bands:** Ensure your phone variant supports your carrier's LTE frequencies (e.g., Vietnam carriers primarily operate LTE Band 3 at 1800MHz and Band 7/8).

---

## 📶 Supported Carrier Profiles (Vietnam)

Profiles are located in `profiles/`:

| Carrier | MCC-MNC | Data APN | IMS APN | Profile ID | Carrier VoLTE Registration |
|---|---|---|---|---|---|
| **Viettel** | `452-04` | `v-internet` | `ims` | `viettel` | SMS `HDCALL` to `191` (or via My Viettel) |
| **MobiFone** | `452-01` | `m-wap`, `m3-world` | `ims` | `mobifone` | SMS `DK VOLTE` to `999` (or via My MobiFone) |
| **VinaPhone** | `452-02` | `m3-world`, `v-internet` | `ims` | `vinaphone` | SMS `HD CALL ON` to `888` (or via My VNPT) |
| **Universal Vietnam** | `452-01/02/04/05` | `m-wap`, `m3-world`, `v-internet` | `ims` | `vietnam-all` | Works with any Vietnam SIM card |

The deployer script automatically detects the SIM card inserted in the phone and selects the corresponding profile dynamically.

---

## 🚀 Deployment & Usage

### Method 1: Windows Single-Click Executable (`BB10VoLTEPatcher.exe`)

1. Enable **Development Mode** on your BlackBerry 10 phone (Settings $\to$ Security and Privacy $\to$ Development Mode).
2. Connect the phone to your computer via USB (or ensure both are on the same Wi-Fi network).
3. Run `BB10VoLTEPatcher.exe`:
   * Enter the phone's IP address (default: `192.168.0.101` for USB or Wi-Fi IP).
   * Select your Carrier Profile (or press Enter for automatic SIM detection).
4. The tool will authenticate over root SSH, unpack the runtime, register permissions with QNX `/proc/boot/pathtrust`, apply the binary patches, and activate the VoLTE daemon.

---

### Method 2: Direct Shell Deployment (`deploy_to_passport.sh`)

Works directly from Linux or macOS:
```bash
# Auto-detect carrier from inserted SIM (recommended)
./deploy_to_passport.sh <PHONE_IP> ~/.ssh/id_rsa auto

# Or force a specific carrier profile:
./deploy_to_passport.sh <PHONE_IP> ~/.ssh/id_rsa mobifone
./deploy_to_passport.sh <PHONE_IP> ~/.ssh/id_rsa vinaphone
./deploy_to_passport.sh <PHONE_IP> ~/.ssh/id_rsa viettel
./deploy_to_passport.sh <PHONE_IP> ~/.ssh/id_rsa vietnam-all
```

---

### Method 3: Standalone Python Patcher (`apply_volte_patch.py`)

If you want to manually patch an extracted `/radio/bin/ims_rrm` binary file without deploying:
```bash
python3 apply_volte_patch.py /path/to/stock/ims_rrm /path/to/output/ims_rrm
```

---

## 🛠️ Building the Standalone Executable from Source

To compile the standalone Windows executable on Linux (using Zig or MinGW-w64):

```bash
# 1. Package the runtime distribution
cd passport_volte_dist && tar -czf ../passport_volte_package.tar.gz . && cd ..

# 2. Embed the private key and payload into embedded_data.h
python3 embed_assets.py path/to/id_rsa passport_volte_package.tar.gz embedded_data.h

# 3. Compile with Zig (cross-compiles to Windows without any dependencies):
zig cc -target x86_64-windows-gnu -DHAVE_EMBEDDED_DATA main.c -o BB10VoLTEPatcher.exe

# Or compile with MinGW-w64:
x86_64-w64-mingw32-gcc -DHAVE_EMBEDDED_DATA main.c -o BB10VoLTEPatcher.exe
```

---

## 🛡️ Safety & Non-Destructive Design

* **No NAND modification**: The phone's read-only system partition (`/radio/bin/ims_rrm`) is **never modified**.
* **Runtime dynamic loading**: The patched binary executes entirely from temporary runtime memory (`/var/rootdata/q10_volte/runtime/ims_rrm`) and registers with QNX `/proc/boot/pathtrust`.
* **Safe recovery**: Simply rebooting the device restores 100% factory state.
* **Auto-Start on Boot**: An AutoRoot v4 addon hook (`/var/addons/q10-unicom-volte/init/start.sh`) automatically relaunches the VoLTE daemon on system boot.

---

## 🤝 Credits & Acknowledgements

* [Octo-o-o-o/bb10-volte](https://github.com/Octo-o-o-o/bb10-volte) - Pioneer work on BB10 VoLTE daemon architecture and runtime environment.
* [imqinyu/BB10DeviceVoLTEResearch](https://github.com/imqinyu/BB10DeviceVoLTEResearch) - BB10 IMS reverse-engineering research.
