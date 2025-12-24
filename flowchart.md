# Sơ đồ lưu đồ giải thuật hệ thống viễn thông

Dựa trên code của hệ thống, đây là sơ đồ lưu đồ giải thuật chính sử dụng state machine.

```mermaid
flowchart TD
    A[Khởi động hệ thống] --> B[STATE1: Nhập password]
    B --> C{Nhấn phím keypad}
    C --> D{Key == '#'?}
    D -->|Có| E{Password đúng?}
    E -->|Có| F[Chuyển STATE2<br>Hiển thị số điện thoại để gọi<br>Set auth timeout 30s]
    E -->|Không| G{Tăng attempt<br>attempt >= 3?}
    G -->|Có| H[Chuyển STATE4<br>Khóa hệ thống 30s]
    G -->|Không| I[Hiển thị sai<br>Reset input<br>Delay 1.5s]
    I --> B
    D -->|Không| J{Key == '*'?}
    J -->|Có| K[Xóa ký tự cuối]
    K --> L[Cập nhật hiển thị]
    L --> B
    J -->|Không| M[Thêm ký tự vào password]
    M --> L
    F --> N[STATE2: Chờ xác thực phone]
    N --> O{Cuộc gọi đến?}
    O -->|Có| P{Số gọi khớp allowed list?}
    P -->|Có| Q[Chuyển STATE3<br>Hiển thị Access Granted<br>Reset auth timeout]
    P -->|Không| R[Thông báo caller không hợp lệ]
    R --> N
    O -->|Không| S{Timeout 30s?}
    S -->|Có| T[Chuyển STATE1<br>Reset password input & attempt]
    S -->|Không| N
    Q --> U[STATE3: Truy cập được cấp]
    U --> V{Sau 5s?}
    V -->|Có| W[Chuyển STATE1<br>Reset]
    V -->|Không| U
    H --> X[STATE4: Hệ thống bị khóa]
    X --> Y{Thời gian khóa hết?}
    Y -->|Có| Z[Chuyển STATE1<br>Reset]
    Z --> B
    Y -->|Không| AA[Hiển thị thời gian còn lại]
    AA --> X
```

## Giải thích các trạng thái

- **STATE1**: Nhập mật khẩu qua keypad. Hỗ trợ xóa (*) và xác nhận (#).
- **STATE2**: Chờ cuộc gọi đến từ số điện thoại được phép trong danh sách allowed.
- **STATE3**: Truy cập được cấp, hiển thị thông báo thành công.
- **STATE4**: Hệ thống bị khóa do nhập sai mật khẩu quá nhiều lần.

## Các thông số chính

- Mật khẩu mặc định: 1234567
- Số lần thử tối đa: 3
- Thời gian khóa: 30 giây
- Thời gian xác thực: 30 giây
- Thời gian hiển thị access granted: 5 giây
