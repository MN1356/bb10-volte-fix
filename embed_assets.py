#!/usr/bin/env python3
"""
Helper script to embed an SSH key and VoLTE deployment package into embedded_data.h
for compiling a standalone self-contained PassportVoLTEPatcher.exe.
"""

import sys
import os

def to_c_array(name, data):
    lines = [f"const unsigned char {name}[] = {{"]
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_vals = ", ".join(f"0x{b:02x}" for b in chunk)
        lines.append(f"    {hex_vals},")
    lines.append("};")
    lines.append(f"const unsigned int {name}_len = {len(data)};\n")
    return "\n".join(lines)

def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <path_to_id_rsa> <path_to_payload.tar.gz> [output_header]")
        print("Example:")
        print(f"  {sys.argv[0]} ~/.ssh/id_rsa passport_volte_package.tar.gz embedded_data.h")
        sys.exit(1)

    key_path = sys.argv[1]
    payload_path = sys.argv[2]
    out_path = sys.argv[3] if len(sys.argv) > 3 else "embedded_data.h"

    with open(key_path, "rb") as f:
        key_data = f.read()

    with open(payload_path, "rb") as f:
        payload_data = f.read()

    with open(out_path, "w") as f:
        f.write("/* Auto-generated embedded assets header */\n\n")
        f.write(to_c_array("EMBEDDED_SSH_KEY", key_data))
        f.write(to_c_array("EMBEDDED_PAYLOAD", payload_data))

    print(f"[+] Successfully generated {out_path} ({len(key_data)} bytes key, {len(payload_data)} bytes payload)")
    print(f"[+] You can now compile with: gcc -DHAVE_EMBEDDED_DATA main.c -o PassportVoLTEPatcher.exe")

if __name__ == "__main__":
    main()
