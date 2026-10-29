====================================================================
BlackBerry 10 (Passport / Q10 / Q20) VoLTE Revival Tool (Windows)
Nha mang ho tro: Viettel (452-04), MobiFone (452-01), VinaPhone (452-02)
Target OS: BlackBerry 10 OS 10.3.3.3216 (May da root)
====================================================================

[TIENG VIET]

1. GIOI THIEU:
   Day la cong cu 1-click tu dong nap ban va VoLTE (Voice over LTE)
   cho BlackBerry Passport, Q10 va Classic (Q20) da root, giup nghe goi 4G khi
   tat ca mang 2G/3G bi tat song.

2. TINH NANG:
   - Tat ca trong mot: Da tich hop san SSH Root Key va bo Patch VoLTE 5 diem
     (Bypass VoPS=0, Ep Precondition Policy volte, Vuot whitelist nha mang,
     Sua loi "Call failure: the call could not be connected" tren mang Viettel/Mobi/Vina,
     Gui lenh SIP UPDATE tuc thi de chuong reo ngay).
   - Ho tro da nha mang: Viettel, MobiFone, VinaPhone va che do Tu dong nhan dien SIM (Auto-detect).
   - An toan 100%: Khong can thiep vao bo nho he thong goc (/radio/bin/ims_rrm).
     Moi thu chay tren RAM/runtime ao. Khoi dong lai van an toan tuyet doi.
   - Tu khoi dong cung may: Tu dong chay lai sau moi lan khoi dong may qua AutoRoot V4.

3. YEU CAU:
   - BlackBerry Passport (SQW100), Q10 (SQN100) hoac Classic Q20 (SQC100) da duoc root (AutoRoot V4).
   - May tinh Windows 10 hoac Windows 11 (co san OpenSSH).
   - Ket noi dien thoai voi PC qua USB hoac chung mang Wi-Fi.

4. HUONG DAN SU DUNG:
   Buoc 1: Cam cap USB ket noi dien thoai voi may tinh (hoac bat Development Mode tren BB10).
   Buoc 2: Chay file "BB10VoLTEPatcher.exe" (hoac "PassportVoLTEPatcher.exe").
   Buoc 3: Nhap dia chi IP cua dien thoai (vi du: 169.254.0.1 neu cam USB, hoac IP Wi-Fi) roi Enter.
   Buoc 4: Chon nha mang (An 1 de tu dong nhan dien SIM qua UICC, hoac chon 2-Viettel, 3-MobiFone, 4-VinaPhone).
   Buoc 5: Cong cu se tu dong ket noi, nap ban va, dang ky pathtrust va kich hoat VoLTE.
   Buoc 6: Kiem tra man hinh va thuc hien cuoc goi 4G thu nghiem!

--------------------------------------------------------------------

[ENGLISH]

1. OVERVIEW:
   One-click utility to automatically patch and deploy VoLTE on rooted
   BlackBerry Passport, Q10, and Classic (Q20) devices running BB10 OS 10.3.3.3216.

2. FEATURES:
   - Self-contained single executable: Embedded root private key and full 5-patch VoLTE stack.
   - Multi-carrier support: Viettel, MobiFone, VinaPhone with automatic SIM detection.
   - Fixes call drop/connection failures across all IR.92 IMS networks.
   - 100% non-destructive RAM runtime execution with AutoRoot boot addon integration.
