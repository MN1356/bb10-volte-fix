#!/usr/bin/env bash
set -e

# Deploy VoLTE Revival to another rooted BlackBerry Passport
# Usage: ./deploy_to_passport.sh <PASSPORT_IP> [SSH_KEY_PATH]

PASSPORT_IP="${1}"
SSH_KEY="${2:-bb10-volte/private/id_rsa}"

if [ -z "$PASSPORT_IP" ]; then
    echo "Usage: $0 <PASSPORT_IP> [SSH_KEY_PATH]"
    echo "Example: $0 192.168.0.105 bb10-volte/private/id_rsa"
    exit 1
fi

SSH_OPTS="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o KexAlgorithms=+diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha256 -o Ciphers=+aes128-cbc,3des-cbc,aes256-cbc -o HostKeyAlgorithms=+ssh-rsa"
if [ -f "$SSH_KEY" ]; then
    SSH_OPTS="$SSH_OPTS -i $SSH_KEY"
fi

echo "==> Connecting to root@$PASSPORT_IP..."
ssh $SSH_OPTS root@$PASSPORT_IP "uname -a"

echo "==> Creating remote directories..."
ssh $SSH_OPTS root@$PASSPORT_IP "mkdir -p /var/rootdata/q10_volte/device /var/rootdata/q10_volte/runtime /var/run/q10_volte"

echo "==> Uploading VoLTE runtime files..."
scp $SSH_OPTS bb10-volte/device/* root@$PASSPORT_IP:/var/rootdata/q10_volte/device/
scp $SSH_OPTS bb10-volte/materials/payload-manifest.json root@$PASSPORT_IP:/var/rootdata/q10_volte/device/

echo "==> Initializing carrier installation.json..."
scp $SSH_OPTS /tmp/installation.json root@$PASSPORT_IP:/var/rootdata/q10_volte/installation.json

echo "==> Registering scripts with /proc/boot/pathtrust..."
ssh $SSH_OPTS root@$PASSPORT_IP "
  /proc/boot/pathtrust '!/var/rootdata/q10_volte/device/run.sh'
  /proc/boot/pathtrust '!/var/rootdata/q10_volte/device/service.py'
  /proc/boot/pathtrust '!/var/rootdata/q10_volte/device/ims_image.py'
  /proc/boot/pathtrust '!/var/rootdata/q10_volte/device/compatibility.py'
  chmod 755 /var/rootdata/q10_volte/device/run.sh
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

echo "==> Deployment Complete!"
