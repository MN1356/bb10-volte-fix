#!/usr/bin/env bash
set -e

# ==============================================================================
# Universal BlackBerry 10 VoLTE Deployment Tool
# Supported Devices:
#   - BlackBerry Passport (SQW100-1/2/3/4)
#   - BlackBerry Classic Q20 (SQC100-1/2/3/4/5)
#   - BlackBerry Q10 (SQN100-1/2/3/4/5)
#   - BlackBerry Z30 (STA100-1/2/3/4/5)
#   - BlackBerry Leap (STR100-1/2)
#   - BlackBerry Z10 LTE (STL100-2/3/4)
# OS Requirement: 10.3.3.3216 (AutoRoot V4 installed)
#
# Usage:
#   ./deploy_to_bb10.sh <PHONE_IP> [SSH_KEY_PATH] [CARRIER]
#
# Examples:
#   ./deploy_to_bb10.sh 192.168.0.100                                 # WiFi, auto-detect carrier
#   ./deploy_to_bb10.sh 169.254.0.1 bb10-volte/private/id_rsa viettel # USB, Viettel
# ==============================================================================

PHONE_IP="${1}"
SSH_KEY="${2:-bb10-volte/private/id_rsa}"
CARRIER="${3:-auto}"

if [ -z "$PHONE_IP" ]; then
    echo "=========================================================="
    echo " BlackBerry 10 Universal VoLTE Deployment Tool"
    echo "=========================================================="
    echo "Usage:   $0 <PHONE_IP> [SSH_KEY_PATH] [CARRIER]"
    echo ""
    echo "Parameters:"
    echo "  PHONE_IP      : IP address of your BlackBerry 10 device"
    echo "                  - Wi-Fi IP: e.g. 192.168.1.50"
    echo "                  - USB IP:   169.254.0.1"
    echo "  SSH_KEY_PATH  : (Optional) Path to root id_rsa private key"
    echo "                  Default: bb10-volte/private/id_rsa"
    echo "  CARRIER       : (Optional) Carrier profile to activate"
    echo "                  Options: auto (default), viettel, vinaphone, mobifone, vietnam-all"
    echo ""
    echo "Examples:"
    echo "  $0 192.168.0.100"
    echo "  $0 169.254.0.1 bb10-volte/private/id_rsa viettel"
    echo "=========================================================="
    exit 1
fi

SSH_OPTS="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o KexAlgorithms=diffie-hellman-group14-sha1 -o Ciphers=aes128-ctr,aes256-ctr -o MACs=hmac-sha1 -o HostKeyAlgorithms=+ssh-rsa -o PubkeyAcceptedKeyTypes=+ssh-rsa"
if [ -f "$SSH_KEY" ]; then
    SSH_OPTS="$SSH_OPTS -i $SSH_KEY"
fi

echo "==> [1/6] Connecting to root@$PHONE_IP..."
ssh $SSH_OPTS root@$PHONE_IP "uname -a"

echo "==> [2/6] Creating remote directories on device..."
ssh $SSH_OPTS root@$PHONE_IP "mkdir -p /var/rootdata/q10_volte/device /var/rootdata/q10_volte/runtime /var/rootdata/q10_volte/runtime/carrier /var/rootdata/q10_volte/driver /var/rootdata/q10_volte/profiles /var/run/q10_volte /var/addons/q10-unicom-volte/init"

echo "==> [3/6] Uploading VoLTE runtime files..."
scp $SSH_OPTS bb10-volte/device/*.py bb10-volte/device/*.sh root@$PHONE_IP:/var/rootdata/q10_volte/device/
scp $SSH_OPTS bb10-volte/materials/payload-manifest.json root@$PHONE_IP:/var/rootdata/q10_volte/device/payload-manifest.json
scp $SSH_OPTS bb10-volte/driver/devnp-volte_swcrypto_probe.so root@$PHONE_IP:/var/rootdata/q10_volte/driver/ 2>/dev/null || true
scp $SSH_OPTS bb10-volte/profiles/*.json root@$PHONE_IP:/var/rootdata/q10_volte/profiles/

echo "==> [4/6] Configuring Auto-Start on Boot (AutoRoot V4 addon)..."
scp $SSH_OPTS bb10-volte/device/addon-start.sh root@$PHONE_IP:/var/addons/q10-unicom-volte/init/start.sh

echo "==> [5/6] Setting executable permissions & QNX pathtrust..."
ssh $SSH_OPTS root@$PHONE_IP "
  chmod 755 /var/rootdata/q10_volte/device/run.sh
  chmod 755 /var/rootdata/q10_volte/device/addon-start.sh
  chmod 755 /var/addons/q10-unicom-volte/init/start.sh
  chmod 755 /var/rootdata/q10_volte/driver/devnp-volte_swcrypto_probe.so 2>/dev/null || true

  for name in run.sh service.py native.py ims_image.py radio_api.py install.py compatibility.py profiles.py device_info.py configure_carrier.py; do
      /proc/boot/pathtrust \"!/var/rootdata/q10_volte/device/\$name\" || exit 1
  done
  /proc/boot/pathtrust '!/var/rootdata/q10_volte/driver/devnp-volte_swcrypto_probe.so' 2>/dev/null || true
  /proc/boot/pathtrust '!/var/addons/q10-unicom-volte/init/start.sh' 2>/dev/null || true
  touch /var/rootdata/q10_volte/enabled
"

echo "==> [6/6] Configuring carrier profile ($CARRIER) & Launching VoLTE..."
ssh $SSH_OPTS root@$PHONE_IP "
  export LD_LIBRARY_PATH=/base/usr/lib:/base/lib:/base/lib/dll:/radio/lib:/radio/lib/dll:/radio/usr/lib:/proc/boot
  export PYTHONHOME=/usr
  export PYTHONPATH=/usr/lib/python32.zip:/usr/lib/python3.2/lib-dynload
  /usr/bin/python3.2 /var/rootdata/q10_volte/device/configure_carrier.py '$CARRIER'
  slay -f python3.2 ims_rrm 2>/dev/null || true
  sleep 1
  /var/rootdata/q10_volte/device/run.sh start
"

echo "==> Waiting 10s for IMS registration..."
sleep 10
ssh $SSH_OPTS root@$PHONE_IP "
  export PYTHONHOME=/usr
  export PYTHONPATH=/usr/lib/python32.zip:/usr/lib/python3.2/lib-dynload
  /usr/bin/python3.2 /var/rootdata/q10_volte/device/service.py status
"

echo "==> VoLTE Deployment Complete! Auto-start on reboot is active."
