#include "DFRobotDFPlayerMini.h"
#include <Wire.h>

// DFPlayer Mini 设置
DFRobotDFPlayerMini myDFPlayer;

// Heart rate sensor 设置
#define SENSOR_PIN A0        // SIG 引脚接到 Leonardo 的 A0 (ADC)
#define VIBRATION_PIN 9      // 振动控制引脚
#define SLAVE_ADDRESS 0x08   // Leonardo I2C 地址

int smoothedValue = 0;       // 平滑后的信号值
int previousValue = 0;       // 前一个信号值
int peakThreshold = 500;     // 阈值，用于检测心跳峰值
int peakCount = 0;           // 检测到的峰值计数
bool musicStarted = false;   // 用于标记是否已开始播放音乐
unsigned long lastHeartbeatTime = 0; // 记录上次心跳信号变化的时间
unsigned long musicStopDelay = 1000;  // 如果 4 秒内没有心跳信号变化，则停止音乐

void setup() {
  // 初始化串口
  Serial.begin(115200);     
  Serial.println("Heart rate monitoring and vibration control started...");

  // 设置引脚模式
  pinMode(SENSOR_PIN, INPUT); 
  pinMode(VIBRATION_PIN, OUTPUT);

  // 初始化 I2C 从设备
  Wire.begin(SLAVE_ADDRESS); 
  Wire.onRequest(sendData);  // 当主设备请求时发送数据

  // 初始化DFPlayer Mini
  Serial1.begin(9600); 
  if (!myDFPlayer.begin(Serial1)) {  
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }

  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(0);  // 初始化音量为 0
  delay(100);
  myDFPlayer.volume(15);  // 设置音量
}

void loop() {
  // 读取心率信号
  int rawValue = analogRead(SENSOR_PIN);
  smoothedValue = (smoothedValue * 19 + rawValue) / 20; // 平滑信号

  // 检测峰值并触发振动
  if (smoothedValue > peakThreshold && previousValue <= peakThreshold) {
    peakCount++; // 增加峰值计数
    if (peakCount >= 3 && !musicStarted) {
      // 检测到三次峰值后播放音乐
      myDFPlayer.play(1);  // 播放第一首音乐
      myDFPlayer.loop(1);  // 循环播放音乐
      musicStarted = true; // 设置标志位，表示音乐已经开始播放
      Serial.println(F("Heartbeat detected. Music started."));
    }
    triggerVibration(smoothedValue); // 振动控制
    lastHeartbeatTime = millis();  // 更新最后一次心跳信号变化的时间
  }

  // 如果没有心跳信号变化超过 4 秒，则停止音乐
  if (musicStarted && millis() - lastHeartbeatTime > musicStopDelay) {
    myDFPlayer.stop();  // 停止音乐
    musicStarted = false;  // 重置标志
    Serial.println(F("No heartbeat detected for 4 seconds. Music stopped."));
  }

  // 输出到串口，供调试使用
  Serial.println(smoothedValue);

  // 更新前一个信号值
  previousValue = smoothedValue;

  // 延时控制采样频率
  delay(10);
}

// 触发振动的函数
void triggerVibration(int signal) {
  int frequency = map(signal, 0, 1023, 36, 100); // 信号映射为频率
  int dutyCycle = map(signal, 0, 1023, 25, 100); // 信号映射为占空比
  int duration = 1000;  // 振动持续时间，1 秒

  for (int i = 0; i < duration; i += (1000 / frequency)) {
    analogWrite(VIBRATION_PIN, 255 * dutyCycle / 100); // 输出 PWM 信号
    delay(1000 / frequency / 2); // 半周期延时
    analogWrite(VIBRATION_PIN, 0); // 停止振动
    delay(1000 / frequency / 2); // 半周期延时
  }
}

// I2C 数据请求回调
void sendData() {
  // 发送平滑后的心率信号值（16 位，分两字节发送）
  Wire.write((uint8_t)(smoothedValue & 0xFF));         // 低字节
  Wire.write((uint8_t)((smoothedValue >> 8) & 0xFF));  // 高字节
}

