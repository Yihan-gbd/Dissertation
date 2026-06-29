#include <Pololu3piPlus32U4.h>
#include <Wire.h>
#include "SwarmB2.h"          // 把 SwarmB2.h, SwarmB2.cpp, ircomm_i2c.h 放在 .ino 同目录

using namespace Pololu3piPlus32U4;

Motors      motors;
LineSensors lineSensors;
ButtonA     buttonA;
OLED        display;
SwarmB2_c   SwarmB2;

// ===== 通信消息结构（必须 ≤ 32 字节）=====
typedef struct __attribute__((packed)) {
  uint8_t  robot_id;       // 机器人编号（区分自己和邻居）
  uint16_t obs_safe;       // 白格观测数
  uint16_t obs_unsafe;     // 黑格观测数
} comm_msg_t;              // 共 5 字节，远小于 32 上限

// ===== 参数 =====
const uint8_t  MY_ID          = 1;       // 每台机器人烧录前改这个：1 或 2
const uint16_t DRIVE_SPEED    = 100;
const uint16_t TURN_SPEED     = 120;
const uint16_t WHITE_MAX      = 400;
const uint16_t GRAY_MIN       = 500;
const uint16_t GRAY_MAX       = 1200;
const uint16_t BLACK_MIN      = 1400;
const unsigned long SAMPLE_MS = 100;
const float MEAN_STRAIGHT_MS  = 2000;
const unsigned long BROADCAST_MS = 200;  // 每 200ms 广播一次

// ===== 状态 =====
enum State { DRIVING, TURNING };
State state = DRIVING;
unsigned long stateStart = 0, straightDuration = 0, turnDuration = 0;
int turnDir = 1;

uint16_t obsSafe = 0, obsUnsafe = 0;
uint16_t nbSafe  = 0, nbUnsafe  = 0;     // 邻居的观测（收到后存这里）
bool     nbPresent = false;               // 是否检测到邻居
unsigned long lastSample = 0, lastBroadcast = 0;
uint16_t lineSensorValues[5];

// ---- 工具函数 ----
float randf() { return random(0, 10000) / 10000.0f; }
unsigned long expRandom(float mean) {
  float u = randf();
  if (u < 1e-4f) u = 1e-4f;
  return (unsigned long)(-mean * log(u));
}

enum Ground { WHITE, BLACK, GRAY, UNKNOWN };
Ground classifyGround(uint16_t r) {
  if (r <= WHITE_MAX) return WHITE;
  if (r >= BLACK_MIN) return BLACK;
  if (r >= GRAY_MIN && r <= GRAY_MAX) return GRAY;
  return UNKNOWN;
}

// ---- 地面采集 ----
void collectInformation() {
  lineSensors.read(lineSensorValues);
  uint16_t center = (lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) / 3;
  Ground g = classifyGround(center);
  if (g == WHITE) obsSafe++;
  else if (g == BLACK) obsUnsafe++;
}

// ---- 广播自己的观测 ----
void broadcastMessage() {
  comm_msg_t msg;
  msg.robot_id   = MY_ID;
  msg.obs_safe   = obsSafe;
  msg.obs_unsafe = obsUnsafe;
  SwarmB2.setIRMessage((uint8_t*)&msg, sizeof(msg));
}

// ---- 接收邻居的观测 ----
void receiveMessages() {
  comm_msg_t buf;
  nbPresent = false;
  for (int i = 0; i < 4; i++) {
    int len = SwarmB2.getIRMessage((uint8_t*)&buf, i);
    if (len == sizeof(comm_msg_t) && buf.robot_id != MY_ID) {
      nbSafe    = buf.obs_safe;
      nbUnsafe  = buf.obs_unsafe;
      nbPresent = true;
    }
  }
}

// ---- 灰色边界避障 ----
bool avoidBorder() {
  lineSensors.read(lineSensorValues);
  Ground left  = classifyGround(lineSensorValues[0]);
  Ground right = classifyGround(lineSensorValues[4]);
  if (left == GRAY && right == GRAY) {
    motors.setSpeeds(-DRIVE_SPEED, -DRIVE_SPEED); delay(200);
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);    delay(400);
    startDriving(); return true;
  }
  if (left == GRAY) {
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED); delay(300);
    startDriving(); return true;
  }
  if (right == GRAY) {
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED); delay(300);
    startDriving(); return true;
  }
  return false;
}

void startDriving() {
  state = DRIVING; stateStart = millis();
  straightDuration = expRandom(MEAN_STRAIGHT_MS) + 300;
  motors.setSpeeds(DRIVE_SPEED, DRIVE_SPEED);
}
void startTurning() {
  state = TURNING; stateStart = millis();
  unsigned long dur = 200 + (unsigned long)(randf() * 1200);
  turnDir = (randf() < 0.5f) ? 1 : -1;
  motors.setSpeeds(turnDir * TURN_SPEED, -turnDir * TURN_SPEED);
  turnDuration = dur;
}

// ---- 显示 ----
void updateDisplay() {
  display.clear();
  display.gotoXY(0, 0); display.print("ID:"); display.print(MY_ID);
  display.gotoXY(0, 1); display.print("W:"); display.print(obsSafe);
    display.print(" B:"); display.print(obsUnsafe);
  display.gotoXY(0, 2);
  if (nbPresent) {
    display.print("NB W:"); display.print(nbSafe);
    display.gotoXY(0, 3);
    display.print("NB B:"); display.print(nbUnsafe);
  } else {
    display.print("No neighbor");
  }
}

// ===== SETUP =====
void setup() {
  Wire.begin();
  Wire.setClock(400000);
  SwarmB2.init();

  display.clear();
  display.gotoXY(0, 0); display.print("Robot #"); display.print(MY_ID);
  display.gotoXY(0, 1); display.print("Press A");
  buttonA.waitForButton();
  randomSeed(micros());
  lastSample = millis();
  lastBroadcast = millis();
  startDriving();
}

// ===== LOOP =====
void loop() {
  unsigned long now = millis();

  // 1) 定时采样地面
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;
    collectInformation();
  }

  // 2) 定时广播 + 接收
  if (now - lastBroadcast >= BROADCAST_MS) {
    lastBroadcast = now;
    broadcastMessage();
    receiveMessages();
    updateDisplay();
  }

  // 3) 灰色边界避障
  if (avoidBorder()) return;

  // 4) 随机游走
  switch (state) {
    case DRIVING:
      if (now - stateStart >= straightDuration) startTurning();
      break;
    case TURNING:
      if (now - stateStart >= turnDuration) startDriving();
      break;
  }
}
