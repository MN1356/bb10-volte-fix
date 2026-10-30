@echo off
setlocal enabledelayedexpansion

:: Chuyen thu muc lam viec ve dung thu muc chua file bat
cd /d "%~dp0"

echo ==========================================================
echo  BlackBerry 10 Universal VoLTE Deployment Tool (Windows)
echo ==========================================================
echo Supported: Passport, Classic Q20, Q10, Z30, Leap, Z10 LTE
echo.

set PHONE_IP=%1
set SSH_KEY=%2
set CARRIER=%3

if "%PHONE_IP%"=="" (
    set /p PHONE_IP="Nhap dia chi IP cua BlackBerry [mac dinh 169.254.0.1 cho USB]: "
)
if "%PHONE_IP%"=="" set PHONE_IP=169.254.0.1

if "%SSH_KEY%"=="" (
    if exist "%~dp0bb10-volte\private\id_rsa" (
        set SSH_KEY=%~dp0bb10-volte\private\id_rsa
    ) else if exist "%~dp0id_rsa" (
        set SSH_KEY=%~dp0id_rsa
    ) else (
        set SSH_KEY=bb10-volte\private\id_rsa
    )
)

if "%CARRIER%"=="" set CARRIER=auto

if not exist "%SSH_KEY%" (
    echo [!] LOI: Khong tim thay khoa SSH tai: %SSH_KEY%
    echo     Ban can "GIAI NEN TOAN BO" file ZIP truoc khi chay,
    echo     khong mo truc tiep file .bat ben trong trinh xem file nen!
    echo.
    pause
    exit /b 1
)

:: Phan quyen chat che cho private key tren Windows de OpenSSH khong tu choi
icacls "%SSH_KEY%" /inheritance:r /grant:r "%USERNAME%:R" >nul 2>&1

set SSH_OPTS=-i "%SSH_KEY%" -o StrictHostKeyChecking=no -o UserKnownHostsFile=NUL -o KexAlgorithms=+diffie-hellman-group14-sha1,diffie-hellman-group-exchange-sha256 -o Ciphers=+aes128-ctr,aes192-ctr,aes256-ctr,aes128-cbc,3des-cbc,aes256-cbc -o MACs=+hmac-sha1 -o HostKeyAlgorithms=+ssh-rsa -o PubkeyAcceptedKeyTypes=+ssh-rsa -o ConnectTimeout=12

echo [*] Ket noi toi BlackBerry (%PHONE_IP%)...
ssh %SSH_OPTS% root@%PHONE_IP% "uname -a"
if errorlevel 1 (
    echo.
    echo [!] LOI KET NOI SSH TOI root@%PHONE_IP%!
    echo ========================================================================
    echo NGUYEN NHAN PHO BIEN:
    echo  1. May BlackBerry CHUA DUOC ROOT bang AutoRoot V4:
    echo     - Chuc nang "Che do phat trien" (Development Mode) mac dinh chi
    echo       cho phep tai khoan 'devuser', KHONG cho phep dang nhap 'root'.
    echo     - May bat buoc phai duoc root truoc qua AutoRoot V4 de nap SSH root.
    echo  2. Neu ban chay bang quyen Administrator lam sai duong dan thu muc,
    echo     hay giai nen toan bo thu muc va chay truc tiep file .bat.
    echo  3. Kiem tra cap USB hoac thu ket noi qua dia chi Wi-Fi cua BlackBerry.
    echo ========================================================================
    pause
    exit /b 1
)

echo [*] Tao thu muc he thong tren may...
ssh %SSH_OPTS% root@%PHONE_IP% "mkdir -p /var/rootdata/q10_volte/device /var/rootdata/q10_volte/runtime /var/rootdata/q10_volte/runtime/carrier /var/rootdata/q10_volte/driver /var/rootdata/q10_volte/profiles /var/run/q10_volte /var/addons/q10-unicom-volte/init"

echo [*] Chep cac tep thuc thi VoLTE vao may...
scp %SSH_OPTS% bb10-volte\device\*.py bb10-volte\device\*.sh root@%PHONE_IP%:/var/rootdata/q10_volte/device/
scp %SSH_OPTS% bb10-volte\materials\payload-manifest.json root@%PHONE_IP%:/var/rootdata/q10_volte/device/payload-manifest.json
scp %SSH_OPTS% bb10-volte\driver\devnp-volte_swcrypto_probe.so root@%PHONE_IP%:/var/rootdata/q10_volte/driver/ 2>nul
scp %SSH_OPTS% bb10-volte\profiles\*.json root@%PHONE_IP%:/var/rootdata/q10_volte/profiles/

echo [*] Thiet lap khoi dong tu dong khi bat may (AutoRoot V4 addon)...
scp %SSH_OPTS% bb10-volte\device\addon-start.sh root@%PHONE_IP%:/var/addons/q10-unicom-volte/init/start.sh

echo [*] Phan quyen thuc thi & cap quyen QNX pathtrust...
ssh %SSH_OPTS% root@%PHONE_IP% "chmod 755 /var/rootdata/q10_volte/device/run.sh; chmod 755 /var/rootdata/q10_volte/device/addon-start.sh; chmod 755 /var/addons/q10-unicom-volte/init/start.sh; for name in run.sh service.py native.py ims_image.py radio_api.py install.py compatibility.py profiles.py device_info.py configure_carrier.py; do /proc/boot/pathtrust \"!/var/rootdata/q10_volte/device/$name\" || exit 1; done; /proc/boot/pathtrust '!/var/addons/q10-unicom-volte/init/start.sh' 2>/dev/null; touch /var/rootdata/q10_volte/enabled"

echo [*] Ap dung profile nha mang (%CARRIER%) va khoi chay tien trinh VoLTE...
ssh %SSH_OPTS% root@%PHONE_IP% "export LD_LIBRARY_PATH=/base/usr/lib:/base/lib:/base/lib/dll:/radio/lib:/radio/lib/dll:/radio/usr/lib:/proc/boot; export PYTHONHOME=/usr; export PYTHONPATH=/usr/lib/python32.zip:/usr/lib/python3.2/lib-dynload; /usr/bin/python3.2 /var/rootdata/q10_volte/device/configure_carrier.py '%CARRIER%'; slay -f python3.2 ims_rrm 2>/dev/null; sleep 1; /var/rootdata/q10_volte/device/run.sh start"

echo [*] Doi 10 giay de dong bo dang ky IMS...
timeout /t 10 /nobreak >nul
ssh %SSH_OPTS% root@%PHONE_IP% "/var/rootdata/q10_volte/device/run.sh status"

echo.
echo ==========================================================
echo  [+] HOAN TAT CAI DAT! VoLTE DA DUOC KICH HOAT THANH CONG.
echo      Dich vu se tu dong khoi chay moi khi khoi dong lai may.
echo ==========================================================
pause
