====================================================================
BlackBerry Passport (SQW100-1) VoLTE Revival Tool (Windows)
Nha mang ho tro: Viettel Vietnam (452-04) & Mang 4G ho tro IMS IR.92
Target OS: BlackBerry 10 OS 10.3.3.3216 (May da root)
====================================================================

[TIENG VIET]

1. GIOI THIEU:
   Day la cong cu 1-click tu dong nap ban va VoLTE (Voice over LTE)
   cho BlackBerry Passport da root, giup nghe goi 4G khi mang 2G/3G bi cat song.

2. TINH NANG:
   - Tat ca trong mot: Da tich hop san SSH Root Key va bo Patch VoLTE 5 diem
     (Bypass VoPS=0, Ep Precondition Policy volte, Vuot whitelist nha mang,
     Sua loi "Call failure: the call could not be connected" tren mang Viettel,
     Gui lenh SIP UPDATE tuc thi de chuong reo ngay).
   - An toan 100%: Khong can thiep vao bo nho he thong goc (/radio/bin/ims_rrm).
     Moi thu chay tren RAM/runtime ao. Khoi dong lai van an toan tuyet doi.
   - Tu khoi dong cung may: Tu dong chay lai sau moi lan khoi dong may qua AutoRoot V4.

3. YEU CAU:
   - BlackBerry Passport (SQW100-1) da duoc root (AutoRoot / Term49).
   - May tinh Windows 10 hoac Windows 11 (co san OpenSSH).
   - Ket noi Passport voi PC qua USB hoac chung mang Wi-Fi.

4. HUONG DAN SU DUNG:
   Buoc 1: Cam cap USB ket noi Passport voi may tinh
           (hoac ket noi cung mang Wi-Fi voi PC).
           - Neu cam USB: IP mac dinh thuong la: 169.254.0.1
           - Neu dung Wi-Fi: Xem IP tren Passport vao Settings -> Network Connections -> Wi-Fi -> Advanced.
   Buoc 2: Chay file "PassportVoLTEPatcher.exe".
   Buoc 3: Nhap dia chi IP cua Passport (vi du: 169.254.0.1 neu cam USB, hoac IP Wi-Fi) roi Enter.
   Buoc 4: Cong cu se tu dong:
           - Ket noi SSH vao Passport bang Root Key nhung san.
           - Upload goi du lieu va dang ky quyen thuc thi pathtrust.
           - Khoi dong tien trinh VoLTE va kiem tra dang ky IMS (200 OK).
   Buoc 5: Khi man hinh bao thanh cong, thuc hien cuoc goi thu tren dien thoai!

--------------------------------------------------------------------

[ENGLISH]

1. OVERVIEW:
   One-click utility to automatically patch and deploy VoLTE on rooted
   BlackBerry Passport (SQW100-1) devices running BB10 OS 10.3.3.3216.

2. FEATURES:
   - Self-contained single executable: Embedded root private key and full 5-patch VoLTE stack.
   - Fixes "Call failure: the call could not be connected" on Viettel & IR.92 IMS networks.
   - Forces QoS Preconditions and sends fast SIP UPDATE on 183 Session Progress.
   - 100% Non-destructive: Runs in dynamic RAM/runtime, NAND remains untouched.
   - Persistent: Auto-starts on reboot via AutoRoot V4 addon.

3. HOW TO RUN:
   - Connect Passport to PC via USB cable (IP: 169.254.0.1) or join same Wi-Fi.
   - Double-click "PassportVoLTEPatcher.exe".
   - Enter your Passport IP and press Enter.
   - Wait ~15 seconds for completion and test making a VoLTE call!
====================================================================
