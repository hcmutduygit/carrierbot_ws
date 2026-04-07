# HƯỚNG DẪN TẠO VÀ LƯU MAP VỚI CARTOGRAPHER

## 📋 MỤC LỤC
1. [Khởi động Cartographer](#1-khởi-động-cartographer)
2. [Tạo map](#2-tạo-map)
3. [Lưu map](#3-lưu-map)
4. [Vị trí lưu map](#4-vị-trí-lưu-map)
5. [Xem map đã lưu](#5-xem-map-đã-lưu)
6. [Xử lý lỗi](#6-xử-lý-lỗi)

---

## 1. KHỞI ĐỘNG CARTOGRAPHER

### Bước 1: Cắm RPLidar vào laptop
- Cắm cáp USB của RPLidar S2 vào cổng USB laptop
- Kiểm tra kết nối:
```bash
ls /dev/ttyUSB*
```
Phải thấy: `/dev/ttyUSB0`

### Bước 2: Mở terminal và chạy Cartographer
```bash
cd /home/duybuntu/mapping_ws
source install/setup.bash
ros2 launch rplidar_ros cartographer_rplidar_s2_rviz.launch.py
```

### Bước 3: Kiểm tra RViz
- Cửa sổ RViz sẽ mở ra
- Bạn sẽ thấy:
  - Lưới (Grid)
  - Điểm laser màu đỏ
  - Map đang được xây dựng

---

## 2. TẠO MAP

### Cách tạo map:
1. **Đặt laptop + lidar lên xe đẩy**
2. **Đẩy xe từ từ** qua các khu vực muốn map
3. **Quan sát RViz**: Map sẽ được vẽ theo thời gian thực
4. **Lưu ý**:
   - Đẩy **chậm và đều** (khoảng 0.5-1 m/s)
   - Đi qua **tất cả các khu vực** cần map
   - Tránh đẩy quá nhanh (sẽ gây lỗi map)
   - Nên **đóng vòng** (quay lại điểm bắt đầu) để map chính xác hơn

### Thời gian mapping:
- Phòng nhỏ (20m²): 2-3 phút
- Phòng lớn (100m²): 5-10 phút
- Nhiều phòng: 10-20 phút

---

## 3. LƯU MAP

### ⚠️ QUAN TRỌNG: Giữ nguyên terminal đang chạy Cartographer!

### Cách 1: Sử dụng script tự động (KHUYÊN DÙNG)

#### Bước 1: Mở terminal mới
Nhấn `Ctrl+Shift+T` hoặc mở terminal mới

#### Bước 2: Chạy lệnh lưu map
```bash
cd /home/duybuntu/mapping_ws
source install/setup.bash
bash src/rplidar_ros/scripts/save_map.sh -n ten_map
```

**Ví dụ cụ thể:**
```bash
# Lưu map văn phòng
bash src/rplidar_ros/scripts/save_map.sh -n van_phong

# Lưu map tầng 1
bash src/rplidar_ros/scripts/save_map.sh -n tang_1

# Lưu map ngày hôm nay
bash src/rplidar_ros/scripts/save_map.sh -n map_20260206
```

#### Bước 3: Chờ script chạy xong
Script sẽ:
- Kết thúc trajectory
- Lưu file `.pbstream` (dữ liệu Cartographer)
- Lưu file `.pgm` (ảnh map)
- Lưu file `.yaml` (metadata)

---

### Cách 2: Lưu thủ công (nếu script bị lỗi)

Mở terminal mới và chạy từng lệnh:

#### Bước 1: Kết thúc trajectory
```bash
ros2 service call /finish_trajectory cartographer_ros_msgs/srv/FinishTrajectory "{trajectory_id: 0}"
```

#### Bước 2: Lưu Cartographer state
```bash
mkdir -p ~/maps
ros2 service call /write_state cartographer_ros_msgs/srv/WriteState "{filename: '/home/duybuntu/maps/ten_map.pbstream'}"
```

#### Bước 3: Lưu occupancy grid
```bash
cd ~/maps
ros2 run nav2_map_server map_saver_cli -f ten_map
```

---

## 4. VỊ TRÍ LƯU MAP

### Thư mục mặc định:
```
/home/duybuntu/maps/
```

### Cấu trúc file map:
Khi lưu map tên `van_phong`, sẽ có 3 file:

```
/home/duybuntu/maps/
├── van_phong.pbstream    # Dữ liệu Cartographer (có thể tải lại để tiếp tục map)
├── van_phong.pgm         # Ảnh map (màu đen/trắng)
└── van_phong.yaml        # Thông tin map (resolution, origin, etc.)
```

### Xem danh sách map đã lưu:
```bash
ls -lh ~/maps/
```

### Thay đổi thư mục lưu:
```bash
# Lưu vào thư mục tùy chỉnh
bash src/rplidar_ros/scripts/save_map.sh -n my_map -o /home/duybuntu/my_custom_folder
```

---

## 5. XEM MAP ĐÃ LƯU

### Xem file .pgm (ảnh map):
```bash
# Cài đặt image viewer nếu chưa có
sudo apt install eog

# Xem map
eog ~/maps/ten_map.pgm
```

### Xem thông tin map (.yaml):
```bash
cat ~/maps/ten_map.yaml
```

Nội dung file `.yaml`:
```yaml
image: van_phong.pgm
resolution: 0.050000      # Độ phân giải (5cm/pixel)
origin: [-10.0, -10.0, 0.0]  # Gốc tọa độ
negate: 0
occupied_thresh: 0.65
free_thresh: 0.196
```

### Tải lại map đã lưu (để tiếp tục mapping):
```bash
ros2 launch cartographer_ros cartographer_pbstream_reader.launch.py \
  pbstream_filename:=/home/duybuntu/maps/van_phong.pbstream
```

---

## 6. XỬ LÝ LỖI

### Lỗi 1: "waiting for service to become available..."

**Nguyên nhân**: Cartographer chưa được khởi động

**Giải pháp**:
1. Kiểm tra Cartographer có đang chạy:
```bash
ros2 node list | grep cartographer
```
Phải thấy: `/cartographer_node`

2. Nếu không thấy, khởi động lại Cartographer:
```bash
cd /home/duybuntu/mapping_ws
source install/setup.bash
ros2 launch rplidar_ros cartographer_rplidar_s2_rviz.launch.py
```

### Lỗi 2: "No such file or directory"

**Nguyên nhân**: Script path không đúng

**Giải pháp**: Sử dụng đường dẫn đầy đủ:
```bash
bash /home/duybuntu/mapping_ws/src/rplidar_ros/scripts/save_map.sh -n my_map
```

### Lỗi 3: "permission denied"

**Nguyên nhân**: Script không có quyền thực thi

**Giải pháp**:
```bash
chmod +x /home/duybuntu/mapping_ws/src/rplidar_ros/scripts/save_map.sh
```

### Lỗi 4: RPLidar timeout khi khởi động

**Nguyên nhân**: Lidar bị chiếm giữ bởi process khác

**Giải pháp**:
1. Ngắt USB lidar ra
2. Đợi 3 giây
3. Cắm lại USB
4. Chạy lại Cartographer

### Lỗi 5: Map không lưu được (thiếu nav2_map_server)

**Nguyên nhân**: Chưa cài Nav2 Map Server

**Giải pháp**:
```bash
sudo apt install ros-humble-nav2-map-server
```

---

## 📊 QUY TRÌNH HOÀN CHỈNH (TÓM TẮT)

### Terminal 1 - Chạy Cartographer:
```bash
cd /home/duybuntu/mapping_ws
source install/setup.bash
ros2 launch rplidar_ros cartographer_rplidar_s2_rviz.launch.py
```

### Đẩy xe để tạo map (3-10 phút)

### Terminal 2 - Lưu map:
```bash
cd /home/duybuntu/mapping_ws
source install/setup.bash
bash src/rplidar_ros/scripts/save_map.sh -n ten_map
```

### Kiểm tra map đã lưu:
```bash
ls -lh ~/maps/
eog ~/maps/ten_map.pgm
```

---

## 🎯 MẸO VÀ LƯU Ý

### Mẹo tạo map tốt:
1. ✅ **Đẩy chậm và đều**: 0.5-1 m/s
2. ✅ **Đóng vòng**: Quay lại điểm bắt đầu
3. ✅ **Đi qua tất cả khu vực**: Không bỏ sót góc nào
4. ✅ **Tránh khu vực có gương/kính**: Laser phản xạ gây nhiễu
5. ✅ **Giữ lidar ở độ cao cố định**: Không lắc lư

### Lưu ý quan trọng:
- ⚠️ **KHÔNG tắt Cartographer** trước khi lưu map xong
- ⚠️ **Chờ script chạy hết** (khoảng 5-10 giây)
- ⚠️ **Mở terminal mới** để chạy lệnh lưu map
- ⚠️ **Backup map thường xuyên** nếu quan trọng

### Kích thước file map:
- `.pbstream`: 5-50 MB (tùy kích thước map)
- `.pgm`: 100 KB - 5 MB
- `.yaml`: 1 KB

---

## 📞 HỖ TRỢ

### Kiểm tra log nếu có lỗi:
```bash
# Log của Cartographer
cat ~/.ros/log/latest/cartographer_node/stdout.log

# Log của RPLidar
cat ~/.ros/log/latest/rplidar_node/stdout.log
```

### Xem các service có sẵn:
```bash
ros2 service list | grep cartographer
```

### Debug ROS2:
```bash
# Xem topics
ros2 topic list

# Xem topic /scan
ros2 topic echo /scan --once

# Xem nodes
ros2 node list

# Xem TF tree
ros2 run tf2_tools view_frames
```

---

**Tạo bởi**: GitHub Copilot  
**Ngày**: 06/02/2026  
**Phiên bản**: 1.0
