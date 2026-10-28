====================================================================
BlackBerry Passport (SQW100-1) VoLTE Revival Tool (Windows)
Carrier: Viettel Vietnam (MCC 452, MNC 04)
Target OS: BlackBerry 10 OS 10.3.3.3216 (Rooted)
====================================================================

WHAT IS THIS?
This tool automatically patches and deploys the VoLTE telephony stack 
to your rooted BlackBerry Passport over Wi-Fi / USB.

FEATURES:
- Self-contained: The SSH root key and full VoLTE payload are embedded inside.
- Fixes outgoing calls: Bypasses VoPS=0, forces 3GPP QoS Preconditions, 
  enables full AMR octet-aligned modesets, and forces immediate SIP UPDATE
  so the recipient phone rings.
- 100% Safe: Does NOT modify your device's read-only NAND (/radio/bin/ims_rrm).
  All patches run safely in dynamic RAM/runtime storage.

REQUIREMENTS:
1. A rooted BlackBerry Passport (SQW100-1).
2. Connected to the same Wi-Fi network as your PC (or over USB).
3. Windows 10 or Windows 11 (OpenSSH is built into Windows).

HOW TO USE:
1. On your BlackBerry Passport:
   - Make sure Wi-Fi or USB IP is reachable.
   - Note your Passport IP address (e.g. 192.168.0.101).
2. On your Windows PC:
   - Double-click "PassportVoLTEPatcher.exe".
   - Type your Passport's IP address (or press Enter if it matches the default).
   - The tool will automatically:
     * Connect to your Passport using the embedded root key.
     * Upload and verify the VoLTE patch files.
     * Register QNX pathtrust permissions.
     * Launch the VoLTE background manager.
     * Check IMS registration (200 OK) and LTE voice status.
3. Once completed, test by making a call!
