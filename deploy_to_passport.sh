#!/usr/bin/env bash
set -e

# Deploy VoLTE Revival to BlackBerry 10 devices (Passport, Q10, Q20)
# Usage: ./deploy_to_passport.sh <PHONE_IP> [SSH_KEY_PATH] [CARRIER]
# CARRIER options: auto (default), viettel, mobifone, vinaphone, vietnam-all

PASSPORT_IP="${1}"
SSH_KEY="${2:-bb10-volte/private/id_rsa}"
CARRIER="${3:-auto}"

if [ -z "$PASSPORT_IP" ]; then
    echo "Usage: $0 <PHONE_IP> [SSH_KEY_PATH] [CARRIER]"
    echo "Example: $0 192.168.0.100 bb10-volte/private/id_rsa auto"
    echo "Carriers: auto (detect from SIM), viettel, mobifone, vinaphone, vietnam-all"
    exit 1
fi

SSH_OPTS="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o KexAlgorithms=diffie-hellman-group14-sha1 -o Ciphers=aes128-ctr,aes256-ctr -o MACs=hmac-sha1 -o HostKeyAlgorithms=+ssh-rsa -o PubkeyAcceptedKeyTypes=+ssh-rsa"
if [ -f "$SSH_KEY" ]; then
    SSH_OPTS="$SSH_OPTS -i $SSH_KEY"
fi

echo "==> Connecting to root@$PASSPORT_IP..."
ssh $SSH_OPTS root@$PASSPORT_IP "uname -a"

echo "==> Creating remote directories..."
ssh $SSH_OPTS root@$PASSPORT_IP "mkdir -p /var/rootdata/q10_volte/device /var/rootdata/q10_volte/runtime /var/rootdata/q10_volte/driver /var/rootdata/q10_volte/profiles /var/run/q10_volte /var/addons/q10-unicom-volte/init"

echo "==> Uploading VoLTE runtime files..."
scp $SSH_OPTS bb10-volte/device/*.py bb10-volte/device/*.sh root@$PASSPORT_IP:/var/rootdata/q10_volte/device/
scp $SSH_OPTS bb10-volte/materials/payload-manifest.json root@$PASSPORT_IP:/var/rootdata/q10_volte/device/payload-manifest.json
scp $SSH_OPTS bb10-volte/driver/devnp-volte_swcrypto_probe.so root@$PASSPORT_IP:/var/rootdata/q10_volte/driver/ 2>/dev/null || true
scp $SSH_OPTS bb10-volte/profiles/*.json root@$PASSPORT_IP:/var/rootdata/q10_volte/profiles/

echo "==> Configuring Auto-Start on Boot (AutoRoot V4 addon)..."
scp $SSH_OPTS bb10-volte/device/addon-start.sh root@$PASSPORT_IP:/var/addons/q10-unicom-volte/init/start.sh

echo "==> Configuring carrier profile ($CARRIER)..."
ssh $SSH_OPTS root@$PASSPORT_IP "
python3.2 -c \"
import sys, json, os
sys.path.insert(0, '/var/rootdata/q10_volte/device')
import native as n
import profiles

target = '$CARRIER'
if not target or target == 'auto':
    try:
        card = n.attrs(n.read('/pps/services/cellular/uicc/card0/status_private'))
        home = n.value(card, 'hplmn', {})
        mcc = str(home.get('mcc', ''))
        mnc = str(home.get('mnc', '')).zfill(2)
        if mcc == '452':
            if mnc == '01':
                target = 'mobifone'
            elif mnc == '02':
                target = 'vinaphone'
            elif mnc == '04':
                target = 'viettel'
            else:
                target = 'vietnam-all'
        else:
            target = 'vietnam-all'
    except Exception:
        target = 'vietnam-all'

p_file = '/var/rootdata/q10_volte/profiles/' + target + '.json'
if not os.path.exists(p_file):
    p_file = '/var/rootdata/q10_volte/profiles/vietnam-all.json'

p = profiles.read_json(p_file)
req = profiles.request(p, allow_experimental=True)
manifest = json.loads(n.read('/var/rootdata/q10_volte/device/payload-manifest.json').decode('ascii'))
manifest['carrier_profile_id'] = req['profile']['id']
manifest['carrier_profile_sha256'] = profiles.fingerprint(req)

with open('/var/rootdata/q10_volte/carrier-profile.json', 'wb') as f:
    f.write(profiles.canonical(req))

with open('/var/rootdata/q10_volte/installation.json', 'w') as f:
    json.dump(manifest, f, indent=2)

print('[+] Active Carrier Profile: ' + req['profile']['id'] + ' (MCC/MNC: ' + req['profile']['carrier']['mcc'] + '-' + ','.join(req['profile']['carrier']['mncs']) + ')')
\"
"

echo "==> Setting executable permissions & QNX pathtrust..."
ssh $SSH_OPTS root@$PASSPORT_IP "
  chmod 755 /var/rootdata/q10_volte/device/run.sh
  chmod 755 /var/rootdata/q10_volte/device/addon-start.sh
  chmod 755 /var/addons/q10-unicom-volte/init/start.sh
  chmod 755 /var/rootdata/q10_volte/driver/devnp-volte_swcrypto_probe.so

  for name in run.sh service.py native.py ims_image.py radio_api.py install.py compatibility.py profiles.py device_info.py; do
      /proc/boot/pathtrust \"!/var/rootdata/q10_volte/device/\$name\" || exit 1
  done
  /proc/boot/pathtrust '!/var/rootdata/q10_volte/driver/devnp-volte_swcrypto_probe.so'
  /proc/boot/pathtrust '!/var/addons/q10-unicom-volte/init/start.sh'
  touch /var/rootdata/q10_volte/enabled
"

echo "==> Launching VoLTE service..."
ssh $SSH_OPTS root@$PASSPORT_IP "
  slay -f python3.2 ims_rrm 2>/dev/null || true
  rm -f /var/rootdata/q10_volte/runtime/ims_rrm
  /bin/sh /var/rootdata/q10_volte/device/run.sh start
"

echo "==> Waiting 10s for IMS registration..."
sleep 10
ssh $SSH_OPTS root@$PASSPORT_IP "/bin/sh /var/rootdata/q10_volte/device/run.sh status"

echo "==> Deployment Complete! Auto-start on reboot is active."
