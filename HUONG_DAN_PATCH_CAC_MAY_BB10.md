# HƯỚNG DẪN CÀI ĐẶT & PATCH VoLTE CHO CÁC DÒNG BLACKBERRY 10

Bộ công cụ hồi sinh VoLTE này hoạt động tương thích với **tất cả các dòng máy BlackBerry 10 hỗ trợ 4G LTE** chạy phiên bản hệ điều hành **10.3.3.3216**.

---

## 1. Danh Sách Các Thiết Bị Được Hỗ Trợ
- **BlackBerry Passport / Passport Silver** (`SQW100-1`, `SQW100-2`, `SQW100-3`, `SQW100-4`)
- **BlackBerry Classic (Q20)** (`SQC100-1`, `SQC100-2`, `SQC100-3`, `SQC100-4`, `SQC100-5`)
- **BlackBerry Q10** (`SQN100-1`, `SQN100-2`, `SQN100-3`, `SQN100-4`, `SQN100-5`)
- **BlackBerry Z30** (`STA100-1`, `STA100-2`, `STA100-3`, `STA100-4`, `STA100-5`)
- **BlackBerry Leap** (`STR100-1`, `STR100-2`)
- **BlackBerry Z10 (Bản 4G LTE)** (`STL100-2`, `STL100-3`, `STL100-4`)

> **Yêu cầu bắt buộc**: Máy phải chạy hệ điều hành **10.3.3.3216** và đã được root bằng **AutoRoot V4** (có SSH root qua cổng 22 hoặc USB IP `169.254.0.1`).

---

## 2. Các Tính Năng Đã Được Tối Ưu Hóa & Sửa Lỗi Hoàn Chỉnh
1. **Full Codec Thoại AMR-WB (16kHz HD Voice) & AMR-NB (8kHz)**:
   - Đảm bảo tương thích hoàn hảo với mạng **Viettel**, **VinaPhone**, **MobiFone**.
   - Khắc phục triệt để lỗi tổng đài từ chối cuộc gọi `488 Not Acceptable Here` / `Incompatible media format` / `Cause 111`.
2. **Chuyển đổi Loa Ngoài (Speaker) & Loa Trong (Handset) Trực Tiếp**:
   - Tích hợp cầu nối Audio Bridge thời gian thực (50ms).
   - Nút **Audio** trên màn hình gọi điện hoạt động bình thường, cho phép bấm để chuyển đổi qua lại giữa loa trong và loa ngoài ngay trong cuộc gọi.
3. **Bảo toàn BBM Video & Ứng dụng gốc**:
   - Hoàn toàn **không tắt hay vô hiệu hóa** dịch vụ `ebmservice` của BBM Video. BBM Video vẫn sử dụng bình thường, đồng thời luồng gọi di động được cố định trên chuẩn 3GPP IR.92 VoLTE.
4. **Tự Động Chạy Khi Khởi Động Máy (Auto-Start on Boot)**:
   - Tích hợp sẵn vào thư mục khởi động `/var/addons/q10-unicom-volte/init/start.sh` của AutoRoot V4. Máy khởi động lại là có sóng VoLTE tự động, không cần chạy lại script.

---

## 3. Cách Cài Đặt Lên Thiết Bị Mới

### Cách A: Cài đặt qua Linux / macOS / WSL (1 câu lệnh)

1. Kết nối máy BlackBerry 10 với PC qua Wi-Fi hoặc cáp USB (bật Chế độ phát triển / Development Mode hoặc SSH).
2. Mở terminal và chạy lệnh:
   ```bash
   # Nếu kết nối qua Wi-Fi (thay IP bằng IP máy BlackBerry):
   ./deploy_to_bb10.sh 192.168.1.50

   # Nếu kết nối qua cáp USB:
   ./deploy_to_bb10.sh 169.254.0.1
   ```
3. Script sẽ tự động:
   - Kết nối SSH bằng khóa `bb10-volte/private/id_rsa`.
   - Tự nhận diện nhà mạng của SIM đang cắm (Viettel / VinaPhone / MobiFone).
   - Đẩy toàn bộ module runtime, patch và cấp quyền `pathtrust` trên QNX.
   - Kích hoạt dịch vụ VoLTE và xác nhận trạng thái `200 OK`.

---

### Cách B: Cài đặt trên Windows (1-Click)

1. Cắm BlackBerry 10 vào máy tính bằng cáp micro-USB.
2. Nhấp đúp chuột vào file **`deploy_to_bb10.bat`**.
3. Nhập địa chỉ IP của máy (mặc định bấm Enter sẽ lấy IP USB `169.254.0.1`).
4. Chương trình sẽ tự động thực hiện mọi bước triển khai và báo kết quả thành công.

---

## 4. Cách Sử Dụng File Patch Nhị Phân Offline (`ims_rrm`)
Nếu bạn muốn tự patch thủ công file `/radio/bin/ims_rrm` lấy từ bất kỳ máy BB10 nào:
```bash
python3 apply_volte_patch.py <đường_dẫn_file_ims_rrm_gốc> <file_đầu_ra>
```
Ví dụ:
```bash
python3 apply_volte_patch.py ims_rrm ims_rrm_patched
```
Script sẽ tự động áp dụng toàn bộ 10 điểm patch nhị phân đã được reverse-engineering và đối chiếu kiểm tra mã băm SHA256 an toàn.
