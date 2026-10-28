#!/usr/bin/env python3
"""
Standalone VoLTE Patch Utility for BlackBerry Passport (SQW100-1)
BB10 OS Version: 10.3.3.3216 (Stock Radio binary)

Applies all 5 reverse-engineered VoLTE patches to ims_rrm:
  1. PPS Carrier Config Redirection (0x366bed)
  2. VoPS Network Veto Bypass (0x62c54)
  3. Precondition Carrier Whitelist Bypass (0x114104)
  4. Precondition Policy Enforcement (0xb8e6e)
  5. Fast UPDATE Trigger for 183 Session Progress (0x19eb6e)
"""

import sys
import os
import hashlib

STOCK_SHA256 = "043d38b0f08cf6a9696ef405f3976679e32ff9ad943ec35294c3d3da69a12b67"
PATCHED_SHA256 = "3353f44609206b8ebcb138ad2b39752af76339e94a839cc0699842608b73b3c2"

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
    }
]

def patch_binary(src_path, dst_path):
    print(f"Reading source binary: {src_path}")
    with open(src_path, "rb") as f:
        data = bytearray(f.read())
    
    src_sha = hashlib.sha256(data).hexdigest()
    print(f"Source SHA256: {src_sha}")
    if src_sha != STOCK_SHA256:
        print(f"[!] Warning: Source does not match expected stock Passport SHA256 ({STOCK_SHA256}).")
        confirm = input("Continue anyway? (y/N): ") if sys.stdin.isatty() else "y"
        if confirm.strip().lower() != "y":
            print("Aborted.")
            sys.exit(1)
    
    print("\nApplying patches:")
    for p in PATCHES:
        off = p["offset"]
        old = p["old_bytes"]
        new = p["new_bytes"]
        current = bytes(data[off:off+len(old)])
        if current != old:
            print(f"[!] Offset {hex(off)} ({p['name']}): byte mismatch!")
            print(f"    Expected: {old.hex()}")
            print(f"    Found:    {current.hex()}")
            if current == new:
                print(f"    (Already patched)")
                continue
            else:
                raise ValueError(f"Cannot patch {p['name']} at {hex(off)}")
        data[off:off+len(new)] = new
        print(f" [+] Patched {p['name']} at {hex(off)}: {old.hex()} -> {new.hex()}")
    
    dst_sha = hashlib.sha256(data).hexdigest()
    print(f"\nResult SHA256: {dst_sha}")
    if dst_sha == PATCHED_SHA256:
        print("[+] SHA256 matches verified Passport VoLTE binary target!")
    
    os.makedirs(os.path.dirname(os.path.abspath(dst_path)), exist_ok=True)
    with open(dst_path, "wb") as f:
        f.write(data)
    
    # Make executable
    os.chmod(dst_path, 0o755)
    print(f"[+] Written patched binary to: {dst_path}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <stock_ims_rrm_path> <output_patched_path>")
        print(f"Example: {sys.argv[0]} /radio/bin/ims_rrm /var/rootdata/q10_volte/runtime/ims_rrm")
        sys.exit(1)
    patch_binary(sys.argv[1], sys.argv[2])
