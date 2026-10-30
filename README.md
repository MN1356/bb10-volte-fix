# BlackBerry 10 VoLTE Revival & Patch Suite
### Native Voice over LTE (VoLTE) Calling on BlackBerry Passport, Classic (Q20), Q10, Z30, Leap & Z10 LTE

[![Target Devices](https://img.shields.io/badge/Devices-Passport%20%7C%20Classic%20Q20%20%7C%20Q10%20%7C%20Z30%20%7C%20Leap%20%7C%20Z10%20LTE-blue.svg)](#-hardware--device-compatibility)
[![Target OS](https://img.shields.io/badge/BB10%20OS-10.3.3.3216-green.svg)](#)
[![VoLTE Status](https://img.shields.io/badge/VoLTE-Two--Way%20Calling%20(MO%2FMT)-brightgreen.svg)](#)
[![Carriers](https://img.shields.io/badge/Carriers-Viettel%20%7C%20MobiFone%20%7C%20VinaPhone%20%7C%20IR.92-orange.svg)](#-supported-carrier-profiles-vietnam--universal)
[![Release](https://img.shields.io/badge/Release-v1.2.0%20Standalone-purple.svg)](#-deployment--usage)
[![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)](#)

A complete reverse-engineered binary patch utility and deployment suite enabling native **VoLTE (Voice over LTE)** calling on rooted **BlackBerry 10** devices running **OS 10.3.3.3216** across modern 4G LTE networks (including Vietnam carriers **Viettel**, **MobiFone**, **VinaPhone**, and international GSMA 3GPP IR.92 IMS networks).

> 🇻🇳 **Tài liệu Tiếng Việt:** Xem hướng dẫn chi tiết từng bước bằng tiếng Việt tại [HUONG_DAN_PATCH_CAC_MAY_BB10.md](file:///home/nhatu3rn/Downloads/bb10%20lte%20revival/HUONG_DAN_PATCH_CAC_MAY_BB10.md).

---

## 📌 Background & Technical Challenge

With 2G and 3G networks shutting down across telecom operators worldwide, BlackBerry 10 devices face complete cellular voice service failure. Even though 4G LTE mobile data continues to work, stock BlackBerry 10 OS fails to establish native VoLTE voice calls due to legacy limitations, hardcoded carrier whitelists, and incompatible IMS stack behaviors.

Deep reverse engineering of the ARMv7 Thumb-2 IMS daemon binary (`/radio/bin/ims_rrm`) on BlackBerry OS **10.3.3.3216** identified key software roadblocks preventing native IMS voice registration and call negotiation:

1. **Hardcoded PPS Carrier Configuration Path**: Operator profiles were strictly restricted to `/pps/services/carrier_config/radio_qct/ims`.
2. **VoPS=0 Network Veto**: When a 4G LTE base station broadcasts `VoPS = 0` (Voice over PS not supported in SIB2 / attach accept), `selectDomainVoice()` prematurely aborted VoLTE attempts and initiated CSFB (Circuit Switched Fallback) to nonexistent 2G/3G networks.
3. **Carrier Whitelist Locks**: Precondition negotiation (`UsePreconditions = 1`) was locked behind hardcoded carrier checks (historically Verizon and China Mobile only).
4. **SIP Precondition Deadlock**: Outgoing SIP `INVITE` sessions were rejected unless `Precondition.Policy` was explicitly enforced to `volte`.
5. **Slow / Missing QCI=1 UPDATE Deadlock**: On carriers where dedicated QCI=1 voice bearers are deferred until call answer, `precondStep()` paused up to 8000ms waiting for bearer state changes before sending the secondary SIP `UPDATE`, causing call setups to time out before the recipient ever rang.
6. **Codec Mismatches (AMR-WB vs AMR-NB)**: Missing AMR-WB SDP media payloads led to immediate call rejections (`488 Not Acceptable Here` / `Incompatible media format` / `Cause 111`).
7. **Incoming Call (MT) CSFB Rejection**: Without explicit `VDP E-UTRAN = 4` (PS Voice Only) and full MMTel ICSI feature tags (`+g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel"` + `+g.3gpp.smsip`), operator switches attempted CSFB for incoming calls, terminating them instantly.

---

## 🔍 The 10 Reverse-Engineered Binary Patches

All 10 binary patches target the stock ARMv7 Thumb-2 binary `/radio/bin/ims_rrm` (shared across all BB10 devices on OS 10.3.3.3216):

| # | Patch Name | Offset (Hex) | Stock Bytes | Patched Bytes | Disassembly (Stock $\to$ Patched) | Description |
|---|---|---|---|---|---|---|
| 1 | **PPS Config Redirection** | `0x366bed` | `/pps/services/...` | `/pps/volte_probe/...` | ASCII string redirection | Redirects PPS config read to `/pps/volte_probe/ims` to load custom IR.92 operator profiles. |
| 2 | **VoPS Veto Bypass** | `0x62c54` | `c7 b9` | `18 e0` | `cbnz r7, 0x62c88` $\to$ `b 0x62c88` | Bypasses VoPS=0 network rejection in `selectDomainVoice` to enable VoLTE on non-VoPS cells. |
| 3 | **Force HD Voice on LTE** | `0x9e7e4` | `1c d1` | `00 bf` | `bne 0x9e820` $\to$ `nop` | Forces HD Voice service registration in `updateImsServices` on 4G LTE. |
| 4 | **Force Codec.AMRWB = 1** | `0xb8d7a` | `01 46` | `01 21` | `mov r1, r0` $\to$ `movs r1, #1` | Forces `Codec.AMRWB = 1` in SIP UA configuration dictionary. |
| 5 | **Precondition Policy Force** | `0xb8e6e` | `20 b1` | `00 bf` | `cbz r0, 0xb8e7a` $\to$ `nop` | Enforces `Precondition.Policy = "volte"` during SIP UA client configuration. |
| 6 | **Precondition Whitelist Bypass** | `0x114104` | `34 d0` | `34 e0` | `beq 0x114170` $\to$ `b 0x114170` | Bypasses carrier whitelist in `ImsPpsCarrierCfgIr92Object` to enable `UsePreconditions = 1` for any carrier. |
| 7 | **Fast UPDATE Trigger** | `0x19eb6e` | `3b d0` | `00 bf` | `beq 0x19ebe8` $\to$ `nop` | Bypasses 8000ms bearer wait in `precondStep()`, immediately sending SIP `UPDATE` upon `183 Session Progress`. |
| 8 | **Default Codec.AMRWB Enabled** | `0x266cde` | `00 21` | `01 21` | `movs r1, #0` $\to$ `movs r1, #1` | Sets `Codec.AMRWB` default to enabled (`1`) in `buildSdpOffer`. |
| 9 | **Never Skip AMR-WB on Config Check** | `0x266ce6` | `80 b1` | `00 bf` | `cbz r0, 0x266d0a` $\to$ `nop` | Ensures AMR-WB is always offered in SDP INVITE regardless of config veto. |
| 10 | **Never Skip AMR-WB on Media Check** | `0x266cf2` | `50 b1` | `00 bf` | `cbz r0, 0x266d0a` $\to$ `nop` | Guarantees AMR-WB 16kHz media payload is appended to SDP offer. |

### Binary Integrity Verification
* **Stock 10.3.3.3216 `ims_rrm` SHA256 (Shared across all BB10 models):**  
  `043d38b0f08cf6a9696ef405f3976679e32ff9ad943ec35294c3d3da69a12b67`
* **Patched `ims_rrm` SHA256 (All 10 patches applied):**  
  `bb8321db808d52b7b5dfd4d5273c0b85558ec1ae3a3803260ffcf878fbe6f706`

---

## ✨ Key Features & Enhancements (v1.2.0)

- **Two-Way Calling Support (Gọi Đi & Nhận Cuộc Gọi Đến)**: Full support for both Mobile Originated (MO - outgoing) and Mobile Terminated (MT - incoming) calls on 4G LTE. Sets `VDP E-UTRAN = 4` (PS Voice Only) and registers MMTel ICSI feature tags (`+g.3gpp.icsi-ref="urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel"` & `+g.3gpp.smsip`), completely preventing carrier switches from dropping incoming calls.
- **Full Audio Codec Support (AMR-WB 16kHz HD Voice & AMR-NB 8kHz)**: Complete SDP offer/answer compatibility with Viettel, MobiFone, VinaPhone, and global operators; resolves `488 Not Acceptable Here` / `Cause 111`.
- **Real-Time Dynamic Audio Bridge**: In-call seamless switching between Handset (earpiece) and Speakerphone via the native UI **Audio** button (sub-50ms responsive polling bridge).
- **Incoming Caller ID Purification**: Real-time daemon monitoring PPS call objects (`private/calls`, `public/calls`, `protected/calls`), stripping ugly carrier SIP routing tails (e.g. `@ims.mnc004...`, `;user=phone`, `sip:`, `tel:`) so clean contact names and phone numbers appear on screen and in call logs.
- **Preserved Native BBM Video & Apps**: The system daemon `ebmservice` is preserved and remains fully functional alongside 3GPP IR.92 VoLTE.
- **Persistent Auto-Start on Boot**: Integrated into AutoRoot v4 `/var/addons/q10-unicom-volte/init/start.sh` so VoLTE automatically reactivates on device boot without re-running scripts.
- **Standalone 1-Click Windows Executable (`BB10VoLTEPatcher.exe`)**: All-in-one compiled executable embedding root SSH private keys and runtime distribution packages—no Python or separate folder extraction required.

---

## 📱 Hardware & Device Compatibility

| Device Model | Model Codes | Chipset / Modem | VoLTE Status | AutoRoot Target |
|---|---|---|---|---|
| **BlackBerry Passport / Silver** | `SQW100-1`, `SQW100-2`, `SQW100-3`, `SQW100-4` | Snapdragon 800 (MSM8974-AA / MDM9x25) | ✅ Supported | Passport AutoRoot v4 |
| **BlackBerry Classic (Q20)** | `SQC100-1`, `SQC100-2`, `SQC100-3`, `SQC100-4`, `SQC100-5` | Snapdragon S4 Plus (MSM8960 / MDM9x15) | ✅ Supported | `Classic_10.3.03.3216_SQC100-1-2-3-4` |
| **BlackBerry Q10** | `SQN100-1`, `SQN100-2`, `SQN100-3`, `SQN100-4`, `SQN100-5` | Snapdragon S4 Plus (MSM8960 / MDM9x15) | ✅ Supported | `Q10_10.3.03.3216_SQN100-1-2-3-4-5` |
| **BlackBerry Z30** | `STA100-1`, `STA100-2`, `STA100-3`, `STA100-4`, `STA100-5` | Snapdragon S4 Pro (MSM8960T Pro / MDM9x15) | ✅ Supported | `Z30_10.3.03.3216` AutoRoot |
| **BlackBerry Leap** | `STR100-1`, `STR100-2` | Snapdragon S4 Plus (MSM8960 / MDM9x15) | ✅ Supported | `Leap_10.3.03.3216` AutoRoot |
| **BlackBerry Z10 (4G LTE)** | `STL100-2`, `STL100-3`, `STL100-4` | Snapdragon S4 Plus (MSM8960 / MDM9x15) | ✅ Supported | `Z10_10.3.03.3216_STL100-2-3-4` AutoRoot |

> [!WARNING]
> **Check Device Variants:** Only 4G LTE variants are supported. The BlackBerry Z10 `STL100-1` uses a Texas Instruments OMAP 4470 chipset without an LTE baseband modem and does not support VoLTE.

> [!IMPORTANT]
> **Never cross-flash AutoRoot images:** While the runtime VoLTE patches and daemon scripts are identical across all BB10 devices, the initial AutoRoot flashing stages modify the device IFS/kernel image. Always flash the AutoRoot recipe specific to your phone model.

---

## 📶 Supported Carrier Profiles (Vietnam & Universal)

Profiles are located in `passport_volte_dist/device/profiles/`:

| Carrier | MCC-MNC | Data APN | IMS APN | Profile ID | Carrier VoLTE Registration |
|---|---|---|---|---|---|
| **Viettel** | `452-04` | `v-internet` | `ims` | `viettel` | SMS `HDCALL` to `191` (or My Viettel app) |
| **MobiFone** | `452-01` | `m-wap`, `m3-world` | `ims` | `mobifone` | SMS `DK VOLTE` to `999` (or My MobiFone app) |
| **VinaPhone** | `452-02` | `m3-world`, `v-internet` | `ims` | `vinaphone` | SMS `HD CALL ON` to `888` (or My VNPT app) |
| **Universal Vietnam** | `452-01/02/04/05` | `m-wap`, `m3-world`, `v-internet` | `ims` | `vietnam-all` | Works with any Vietnam SIM card |

> [!TIP]
> **Airplane Mode Cycle:** After initial installation or after swapping SIM cards, toggle **Airplane Mode ON for 15 seconds** and then turn it OFF. This forces the carrier tower to establish a fresh 4G LTE EPS / IMS session with VoLTE two-way routing.

---

## 🚀 Deployment & Usage

### Method 1: Windows 1-Click Standalone Executable (`BB10VoLTEPatcher.exe`)

1. Connect your BlackBerry 10 phone to your Windows PC via USB.
   *(On phone: Settings $\to$ Storage and Access: enable 'Access using USB' and set to 'Connect to Windows')*.
2. Ensure **Development Mode** is enabled on the device (Settings $\to$ Security and Privacy $\to$ Development Mode).
3. Double-click `BB10VoLTEPatcher.exe`:
   * Press **Enter** to accept the default USB IP (`169.254.0.1`), or enter the phone's Wi-Fi IP address.
   * Select option **1** for automatic carrier detection from your inserted SIM.
4. The tool automatically handles Windows file permissions (`icacls`), connects over root SSH, unpacks the runtime, registers permissions with QNX `/proc/boot/pathtrust`, applies the binary patches, and activates the VoLTE daemon.

---

### Method 2: Cross-Platform Shell Deployment (`deploy_to_bb10.sh` / `deploy_to_bb10.bat`)

Works directly from Linux, macOS, or Windows CMD/PowerShell:

```bash
# Deploy to USB IP (default 169.254.0.1) with auto carrier detection:
./deploy_to_bb10.sh 169.254.0.1

# Deploy via Wi-Fi IP:
./deploy_to_bb10.sh 192.168.1.50

# Force a specific carrier profile:
./deploy_to_bb10.sh 169.254.0.1 viettel
./deploy_to_bb10.sh 169.254.0.1 mobifone
./deploy_to_bb10.sh 169.254.0.1 vinaphone
./deploy_to_bb10.sh 169.254.0.1 vietnam-all
```

On Windows without the standalone EXE:
```cmd
deploy_to_bb10.bat 169.254.0.1 auto
```

---

### Method 3: Standalone Python Binary Patcher (`apply_volte_patch.py`)

To manually patch an extracted `/radio/bin/ims_rrm` binary file offline without deploying:

```bash
# Check patch definitions and apply all 10 patches
python3 apply_volte_patch.py /path/to/stock/ims_rrm /path/to/output/ims_rrm
```

The script verifies original and patched SHA256 hashes against [`volte_patches.json`](file:///home/nhatu3rn/Downloads/bb10%20lte%20revival/volte_patches.json).

---

## 🛠️ Building the Standalone Executable from Source

To compile the standalone Windows executable on Linux (using Zig or MinGW-w64):

```bash
# 1. Package the runtime distribution
cd passport_volte_dist && tar -czf ../passport_volte_package.tar.gz . && cd ..

# 2. Embed the private root SSH key and payload into embedded_data.h
python3 embed_assets.py id_rsa passport_volte_package.tar.gz embedded_data.h

# 3. Cross-compile with Zig (zero dependencies):
zig cc -target x86_64-windows-gnu -DHAVE_EMBEDDED_DATA main.c -o BB10VoLTEPatcher.exe

# Or compile with MinGW-w64:
x86_64-w64-mingw32-gcc -DHAVE_EMBEDDED_DATA main.c -o BB10VoLTEPatcher.exe
```

---

## 🛡️ Safety & Non-Destructive Design

* **Zero NAND Risk**: The read-only system partition (`/radio/bin/ims_rrm`) is **never modified or overwritten**.
* **RAM/Runtime Execution**: The patched binary executes entirely from temporary runtime memory (`/var/rootdata/q10_volte/runtime/ims_rrm`) verified by QNX `/proc/boot/pathtrust`.
* **Safe Recovery**: Deleting `/var/rootdata/q10_volte/enabled` or rebooting the phone safely resets all radio settings.
* **Non-Invasive Daemon**: Native system daemons (`ebmservice`, `pps`, `cellular`) are preserved without binary destruction.

---

## 🤝 Credits & Acknowledgements

* [Octo-o-o-o/bb10-volte](https://github.com/Octo-o-o-o/bb10-volte) - Pioneer work on BB10 VoLTE daemon architecture and runtime environment.
* [imqinyu/BB10DeviceVoLTEResearch](https://github.com/imqinyu/BB10DeviceVoLTEResearch) - Initial BB10 IMS reverse-engineering research.
