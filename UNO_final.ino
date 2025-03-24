#include <Adafruit_NeoPixel.h>

// WS2812B 灯条相关
#define TOUCH_PIN_1 13      // 第一个触控传感器的信号引脚连接到 GPIO 13
#define TOUCH_PIN_2 10      // 第二个触控传感器的信号引脚连接到 GPIO 10
#define LED_PIN 4           // WS2812B 数据引脚连接到 GPIO 4
#define LED_COUNT 60        // 灯条中 LED 的数量

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800); // 创建灯条对象
bool ledState = true; // 初始灯条状态为打开

void setup() {
    // 初始化 WS2812B 灯条
    pinMode(TOUCH_PIN_1, INPUT);
    pinMode(TOUCH_PIN_2, INPUT);
    strip.begin();
    strip.show(); // 确保灯条初始为关闭状态

    // 设置串口调试
    Serial.begin(115200);
    Serial.println("Arduino Uno ready to control LED strip with touch sensors");

    // 初始设置灯条为亮
    controlLED(true);
}

void loop() {
    // 处理触控传感器状态和灯条控制
    handleTouchControl();

    // 短延时
    delay(50);
}

// 处理触控输入并控制灯条
void handleTouchControl() {
    int touchState1 = digitalRead(TOUCH_PIN_1); // 读取第一个触控传感器的状态
    int touchState2 = digitalRead(TOUCH_PIN_2); // 读取第二个触控传感器的状态

    // 打印触控状态，用于调试
    Serial.print("Touch State 1: ");
    Serial.print(touchState1);
    Serial.print("  Touch State 2: ");
    Serial.println(touchState2);

    // 同时触碰两个触控板时熄灭灯条
    if (touchState1 == LOW && touchState2 == LOW) {
        if (ledState) { // 确保仅在灯条打开时才触发关闭
            ledState = false;
            controlLED(false); // 熄灭灯条
            Serial.println("Both touch sensors active, LED OFF");
        }
    } else if (touchState1 == HIGH && touchState2 == HIGH) { // 两个触控板都未触碰
        if (!ledState) { // 确保仅在灯条关闭时才触发打开
            ledState = true;
            controlLED(true); // 打开灯条
            Serial.println("No touch sensors active, LED ON");
        }
    } else {
        // 如果部分触控板触发，不执行任何操作
        Serial.println("One touch sensor active, no action");
    }
}

// 控制灯条亮灭
void controlLED(bool state) {
    if (state) {
        // 设置所有 LED 为半亮白光
        for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, strip.Color(64, 64, 64)); // 设置半亮白光
        }
        strip.show();
        Serial.println("LED Strip ON");
    } else {
        // 关闭灯条
        for (int i = 0; i < LED_COUNT; i++) {
            strip.setPixelColor(i, strip.Color(0, 0, 0)); // 设置为关闭
        }
        strip.show();
        Serial.println("LED Strip OFF");
    }
}
