#define BLYNK_TEMPLATE_ID "TMPL61ADH0bU-"
#define BLYNK_TEMPLATE_NAME "Sistem Keamanan"
#define BLYNK_AUTH_TOKEN "6DSOA6NrQ_Zo2HT7ATAwEFBSqiDUqdc0"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "WARKOP RUMA";
char pass[] = "Ngopiruma123";

// Definisi Pin ESP32 sesuai wiring
const int PIN_PIR       = 27;  // Output sensor PIR
const int PIN_REED      = 14;  // Reed switch 2 kaki (ke GPIO 14 & GND)
const int PIN_BUZZER    = 26;  // Sinyal ke Buzzer / Transistor NPN
const int PIN_LED_MERAH = 25;  // LED Merah + Resistor 220-330 Ohm (Indikator Bahaya)
const int PIN_LED_HIJAU = 33;  // LED Hijau + Resistor 220-330 Ohm (Indikator Aman)

BlynkTimer timer;
bool sedangBahaya = false;
unsigned long waktuNotifTerakhir = 0;
const unsigned long JEDA_MINIMAL = 2000; // Jeda anti-bounce 2 detik

void kirimNotifBahaya(String pesan) {
  if (Blynk.connected()) {
    Blynk.logEvent("motion_detected", pesan);
    Serial.println(">>> Notifikasi Blynk Terkirim: " + pesan);
    waktuNotifTerakhir = millis();
  } else {
    Serial.println(">>> Gagal kirim notif: Blynk belum terhubung!");
  }
}

void cekKeamanan() {
  int statusPIR  = digitalRead(PIN_PIR);
  int statusReed = digitalRead(PIN_REED);

  bool adaGerakan   = (statusPIR == HIGH);
  bool pintuTerbuka = (statusReed == HIGH);

  if (adaGerakan || pintuTerbuka) {
    // Kondisi BAHAYA
    digitalWrite(PIN_BUZZER, HIGH);
    digitalWrite(PIN_LED_MERAH, HIGH); // LED Merah nyala
    digitalWrite(PIN_LED_HIJAU, LOW);  // LED Hijau mati
    Blynk.virtualWrite(V0, 1);

    String pesan = "[BAHAYA] ";
    if (adaGerakan)   pesan += "Gerakan Terdeteksi! ";
    if (pintuTerbuka) pesan += "Pintu/Jendela Terbuka! ";
    Serial.println(pesan);

    // Kirim notif saat baru masuk kondisi bahaya, atau jika terpicu ulang setelah jeda singkat
    if (!sedangBahaya || (millis() - waktuNotifTerakhir >= JEDA_MINIMAL)) {
      kirimNotifBahaya(pesan);
      sedangBahaya = true;
    }
  } else {
    // Kondisi AMAN -> reset status bahaya agar buka pintu berikutnya langsung kirim notif lagi
    sedangBahaya = false;

    digitalWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_LED_MERAH, LOW);  // LED Merah mati
    digitalWrite(PIN_LED_HIJAU, HIGH); // LED Hijau nyala
    Blynk.virtualWrite(V0, 0);

    Serial.println("[AMAN] Tidak ada gerakan & pintu tertutup.");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_REED, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_MERAH, OUTPUT);
  pinMode(PIN_LED_HIJAU, OUTPUT);

  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED_MERAH, LOW);
  digitalWrite(PIN_LED_HIJAU, HIGH);

  Serial.println("=== Sistem Keamanan IoT Dimulai ===");
  Serial.println("Menunggu kalibrasi sensor PIR (5 detik)...");
  delay(5000);

  Serial.println("Menghubungkan ke WiFi & Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(300L, cekKeamanan);
  Serial.println("Sistem Siap!");
}

void loop() {
  Blynk.run();
  timer.run();
}