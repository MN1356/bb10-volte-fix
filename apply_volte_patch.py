#!/usr/bin/env python3
"""
Standalone VoLTE Patch Utility for BlackBerry 10 LTE Devices
Supported Devices:
  - BlackBerry Passport (SQW100-1/2/3/4)
  - BlackBerry Classic Q20 (SQC100-1/2/3/4/5)
  - BlackBerry Q10 (SQN100-1/2/3/4/5)
  - BlackBerry Z30 (STA100-1/2/3/4/5)
  - BlackBerry Leap (STR100-1/2)
  - BlackBerry Z10 LTE (STL100-2/3/4)
BB10 OS Version: 10.3.3.3216 (Stock Radio binary)

Applies all reverse-engineered VoLTE patches to ims_rrm:
  1. PPS Carrier Config Redirection (0x366bed)
  2. VoPS Network Veto Bypass (0x62c54)
  3. Force HD Voice on LTE (0x9e7e4)
  4. Force Codec.AMRWB=1 in config dictionary (0xb8d7a)
  5. Precondition Policy Enforcement (0xb8e6e)
  6. Precondition Carrier Whitelist Bypass (0x114104)
  7. Fast UPDATE Trigger for 183 Session Progress (0x19eb6e)
  8. Default Codec.AMRWB=1 (0x266cde)
  9. Never skip AMR-WB on config check (0x266ce6)
  10. Never skip AMR-WB on media format check (0x266cf2)
"""

import sys
import os
import hashlib

STOCK_SHA256 = "043d38b0f08cf6a9696ef405f3976679e32ff9ad943ec35294c3d3da69a12b67"
PATCHED_SHA256 = "bb8321db808d52b7b5dfd4d5273c0b85558ec1ae3a3803260ffcf878fbe6f706"

PATCHES = [
    {
        "name": "PPS Carrier Config Redirection",
        "offset": 0x366bed,
        "old_bytes": b"/pps/services/carrier_config/radio_qct/ims\0",
        "new_bytes": b"/pps/volte_probe/ims\0".ljust(len(b"/pps/services/carrier_config/radio_qct/ims\0"), b"\0"),
        "description": "Redirects hardcoded PPS path to /pps/volte_probe/ims for custom carrier config"
    },
    {
        "name": "VoPS Veto Bypass",
        "offset": 0x62c54,
        "old_bytes": b"\xc7\xb9",  # cbnz r7, 0x62c88
        "new_bytes": b"\x18\xe0",  # b 0x62c88
        "description": "Bypasses VoPS=0 network veto in selectDomainVoice to allow VoLTE on non-VoPS cells"
    },
    {
        "name": "Force HD Voice on LTE",
        "offset": 0x9e7e4,
        "old_bytes": b"\x1c\xd1",  # bne 0x9e820
        "new_bytes": b"\x00\xbf",  # nop
        "description": "Forces HD Voice service registration on LTE"
    },
    {
        "name": "Force Codec.AMRWB = 1",
        "offset": 0xb8d7a,
        "old_bytes": b"\x01\x46",  # mov r1, r0
        "new_bytes": b"\x01\x21",  # movs r1, #1
        "description": "Forces Codec.AMRWB = 1 in SIP UA configuration dictionary"
    },
    {
        "name": "Force Precondition.Policy = volte",
        "offset": 0xb8e6e,
        "old_bytes": b"\x20\xb1",  # cbz r0, 0xb8e7a
        "new_bytes": b"\x00\xbf",  # nop
        "description": "Forces Precondition.Policy to 'volte' in SIP UA client"
    },
    {
        "name": "Force UsePreconditions = 1",
        "offset": 0x114104,
        "old_bytes": b"\x34\xd0",  # beq 0x114170
        "new_bytes": b"\x34\xe0",  # b 0x114170
        "description": "Bypasses Verizon/ChinaMobile carrier whitelist to enable preconditions for any carrier"
    },
    {
        "name": "Fast UPDATE on 183 Session Progress",
        "offset": 0x19eb6e,
        "old_bytes": b"\x3b\xd0",  # beq 0x19ebe8
        "new_bytes": b"\x00\xbf",  # nop
        "description": "Bypasses 8000ms QCI=1 bearer wait in precondStep(), immediately sending UPDATE so phone rings"
    },
    {
        "name": "Default Codec.AMRWB to Enabled",
        "offset": 0x266cde,
        "old_bytes": b"\x00\x21",  # movs r1, #0
        "new_bytes": b"\x01\x21",  # movs r1, #1
        "description": "Defaults Codec.AMRWB to 1 (enabled) in buildSdpOffer"
    },
    {
        "name": "Never Skip AMR-WB on Config Check",
        "offset": 0x266ce6,
        "old_bytes": b"\x80\xb1",  # cbz r0, 0x266d0a
        "new_bytes": b"\x00\xbf",  # nop
        "description": "Ensures AMR-WB is always offered in SDP INVITE regardless of config veto"
    },
    {
        "name": "Never Skip AMR-WB on Media Check",
        "offset": 0x266cf2,
        "old_bytes": b"\x50\xb1",  # cbz r0, 0x266d0a
        "new_bytes": b"\x00\xbf",  # nop
        "description": "Ensures AMR-WB 16kHz media payload is always appended to SDP offer"
    }
]

def patch_binary(src_path, dst_path):
    print(f"Reading source binary: {src_path}")
    with open(src_path, "rb") as f:
        data = bytearray(f.read())

    src_hash = hashlib.sha256(data).hexdigest()
    if src_hash != STOCK_SHA256:
        print(f"[!] Warning: Source binary hash ({src_hash}) does not match stock 10.3.3.3216 hash ({STOCK_SHA256})")
        if src_hash == PATCHED_SHA256:
            print("[+] Binary is already fully patched!")
            return True
        choice = input("Do you want to proceed anyway? [y/N]: ").strip().lower()
        if choice != "y":
            return False

    print(f"Applying {len(PATCHES)} patches...")
    for idx, patch in enumerate(PATCHES, 1):
        off = patch["offset"]
        old = patch["old_bytes"]
        new = patch["new_bytes"]
        
        curr = bytes(data[off:off+len(old)])
        if curr != old:
            if curr == new:
                print(f"  [{idx}/{len(PATCHES)}] {patch['name']}: Already applied.")
                continue
            else:
                print(f"  [!] Patch '{patch['name']}' mismatch at offset 0x{off:x}! Expected {old.hex()}, found {curr.hex()}")
                return False

        data[off:off+len(new)] = new
        print(f"  [{idx}/{len(PATCHES)}] {patch['name']}: Applied successfully.")

    out_hash = hashlib.sha256(data).hexdigest()
    print(f"Saving patched binary to: {dst_path}")
    with open(dst_path, "wb") as f:
        f.write(data)

    print(f"[+] Done! Patched binary SHA256: {out_hash}")
    if out_hash == PATCHED_SHA256:
        print("[+] Binary verification: PASSED (Exact match with tested runtime image)")
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 apply_volte_patch.py <path_to_stock_ims_rrm> [output_ims_rrm]")
        print("Example: python3 apply_volte_patch.py ims_rrm ims_rrm_patched")
        sys.exit(1)

    src = sys.argv[1]
    dst = sys.argv[2] if len(sys.argv) > 2 else src + ".patched"
    if not os.path.isfile(src):
        print(f"Error: File not found: {src}")
        sys.exit(1)

    patch_binary(src, dst)
