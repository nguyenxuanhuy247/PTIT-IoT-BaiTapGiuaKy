#include <WiFi.h>
#include <DHT.h>
#include "ThingSpeak.h"

// --- 1. CẤU HÌNH WI-FI VÀ THINGSPEAK (THAY THÔNG TIN CỦA BẠN VÀO ĐÂY) ---
char ssid[] = "Mạnh Đan";     // Tên mạng Wi-Fi nhà bạn
char pass[] = "Manhdan14021997@#";        // Mật khẩu Wi-Fi nhà bạn
WiFiClient client;

unsigned long myChannelNumber = 3496518;         // THAY SỐ CHANNEL ID CỦA BẠN VÀO ĐÂY (Ví dụ: 2456789)
const char * myWriteAPIKey = "2N8Q5PIPZFDRULGC";    // THAY WRITE API KEY CỦA BẠN VÀO ĐÂY

// --- 2. CẤU HÌNH CHÂN CẢM BIẾN ---
#define DHTPIN 4       // Chân OUT của DHT11 nối với chân D4 trên ESP32
#define DHTTYPE DHT11  // Khai báo loại cảm biến là DHT11

#define LDR_PIN 34     // Chân DO của cảm biến ánh sáng nối với chân D34 trên ESP32

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200); // Khởi động Serial với baudrate 115200
  Serial.println("Đang khởi động hệ thống cảm biến...");

  dht.begin();             // Khởi động cảm biến DHT11
  pinMode(LDR_PIN, INPUT); // Cài đặt chân LDR là chân đọc dữ liệu vào

  // Kết nối Wi-Fi
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Khởi tạo ThingSpeak
}

void loop() {
  // Kiểm tra và kết nối lại Wi-Fi nếu bị rớt mạng
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Đang kết nối lại Wi-Fi: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
      delay(5000);
    }
    Serial.println("\nĐã kết nối Wi-Fi!");
  }

  delay(2000); // Đợi 2 giây giữa mỗi lần đọc cảm biến

  // Đọc độ ẩm và nhiệt độ từ DHT11
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Đọc nhiệt độ theo độ C

  // Đọc giá trị từ cảm biến ánh sáng
  int ldrState = digitalRead(LDR_PIN); 

  // Kiểm tra xem dữ liệu đọc từ DHT11 có hợp lệ không
  if (isnan(h) || isnan(t)) {
    Serial.println("Lỗi: Không thể đọc dữ liệu từ cảm biến DHT11!");
    return;
  }

  // In kết quả ra màn hình Serial Monitor (để kiểm tra tại chỗ)
  Serial.print("Nhiệt độ: ");
  Serial.print(t);
  Serial.print(" °C  |  ");
  
  Serial.print("Độ ẩm: ");
  Serial.print(h);
  Serial.print(" %  |  ");

  Serial.print("Trạng thái ánh sáng (DO): ");
  if (ldrState == HIGH) {
    Serial.println("Có ánh sáng / Sáng");
  } else {
    Serial.println("Không có ánh sáng / Tối");
  }

  // --- 3. ĐẨY DỮ LIỆU LÊN THINGSPEAK ---
  ThingSpeak.setField(1, t);         // Field 1: Nhiệt độ
  ThingSpeak.setField(2, h);         // Field 2: Độ ẩm
  ThingSpeak.setField(3, ldrState);  // Field 3: Ánh sáng

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
  if (x == 200) { // <--- Sửa thành số 200 chuẩn xác
    Serial.println("==> Cập nhật dữ liệu lên ThingSpeak thành công!");
  } else {
    Serial.print("==> Lỗi cập nhật dữ liệu. Mã lỗi HTTP: ");
    Serial.println(x);
  }
  
  Serial.println("-----------------------------------");
  
  // ThingSpeak bản miễn phí yêu cầu thời gian giữa 2 lần gửi tối thiểu khoảng 15-20 giây
  delay(15000); 
}
