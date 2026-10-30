====================================================================
BlackBerry 10 Universal VoLTE Revival Tool v1.2.0 (Windows)
Ho tro: Passport, Classic (Q20), Q10, Z30, Leap, Z10 (4G LTE)
Nha mang: Viettel (452-04), MobiFone (452-01), VinaPhone (452-02)
Target OS: BlackBerry 10 OS 10.3.3.3216 (May da root qua AutoRoot V4)
====================================================================

[TIENG VIET]

1. GIOI THIEU:
   Day la cong cu Standalone (Tat ca trong mot) 1-click tu dong nap ban va
   kich hoat VoLTE (Voice over LTE) cho toan bo cac dong may BlackBerry 10 
   chay 4G LTE da duoc Root, giup nghe goi tren song 4G sau khi cac nha mang
   tat hoan toan song 2G/3G.

2. TINH NANG NOI BAT (v1.2.0):
   - Standalone 1-File: Toan bo ma nguon, khoa SSH Root va bo Patch VoLTE da duoc
     dong goi truc tiep vao trong file .exe. Chi can nhap dup chuot la chay ngay.
   - Ho tro toan bo codec AMR-WB (16kHz HD Voice) va AMR-NB (8kHz), khac phuc triet de
     loi "Call failure: the call could not be connected" / "488 Not Acceptable Here".
   - Tich hop cau noi Audio Bridge: Chuyen doi loa ngoai (Speaker) va loa trong
     (Handset) ngay trong cuoc goi bang nut Audio tren man hinh.
   - Tu dong an duoi SIP "@xxx" khi co cuoc goi den (khop danh ba chuan).
   - Bao toan BBM Video & ung dung goc.
   - Tu khoi dong cung may: Tich hop san AutoRoot V4 addon, may khoi dong lai
     la tu co song VoLTE.

3. DIEU KIEN BAT BUOC:
   - May phai chay BB10 phien ban 10.3.3.3216 va DA DUOC ROOT bang AutoRoot V4.
     (Luu y: "Che do phat trien" / Development Mode mac dinh chi cho tai khoan 'devuser',
      phai co AutoRoot V4 thi may moi mo quyen 'root' de chay patch).
   - May tinh Windows 10 hoac Windows 11 (co san OpenSSH).

4. HUONG DAN SU DUNG:
   Buoc 1: Cam cap USB ket noi BlackBerry voi may tinh.
           (Trong Cai dat -> Luu tru & Truy cap: dam bao chon 'Ket noi voi Windows').
   Buoc 2: Chay file "BB10VoLTEPatcher.exe".
   Buoc 3: Nhan Enter de lay dia chi IP USB mac dinh (169.254.0.1).
           (Neu dung Wi-Fi: nhap dia chi IP Wi-Fi hien trong Che do phat trien).
   Buoc 4: Chon nha mang:
           - Nhan 1: Tu dong nhan dien nha mang theo SIM (Khuyen dung).
           - Nhan 2: Viettel | 3: MobiFone | 4: VinaPhone | 5: Universal Vietnam.
   Buoc 5: Tool se tu dong ket noi SSH, nap ban va va khoi chay tien trinh VoLTE.
   Buoc 6: Kiem tra bieu tuong 4G LTE va thuc hien cuoc goi thu nghiem!

--------------------------------------------------------------------

[ENGLISH]

1. OVERVIEW:
   Standalone 1-click utility to automatically patch and deploy VoLTE on rooted
   BlackBerry 10 devices (Passport, Classic, Q10, Z30, Leap, Z10 LTE) running
   BB10 OS 10.3.3.3216.

2. FEATURES:
   - All-in-one standalone executable: Embedded root private key and full VoLTE payload.
   - Dual codec AMR-WB & AMR-NB: Eliminates carrier call rejection (488 Not Acceptable).
   - In-call Speaker/Handset switching via on-screen Audio button.
   - Automatic SIP '@xxx' caller ID suffix stripping.
   - BBM Video preserved. Auto-start on boot via AutoRoot V4 addon.

3. REQUIREMENTS:
   - Rooted BB10 device via AutoRoot V4 (OS 10.3.3.3216).
   - Windows 10/11 with OpenSSH enabled. USB connection set to 'Connect to Windows'.
