#include <Wire.h>
#include <SPI.h>
//#include <SD.h>
#include <FS.h>
#include "SD_MMC.h"
#include "Adafruit_LSM6DSL.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



#define BLUE_LED       5 //   

#define IMU_SAMPLING_INTERVAL       10 // 10ms =100HZ 

Adafruit_LSM6DSL lsm6ds;
sensors_event_t accel, gyro, temp;

File file;
String baseName = "/Data";
String fileName;


struct DataItem {
    unsigned long timestamp;
    float acc_x;
    float acc_y;
    float acc_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
};
/* Data buffer for transfering data from IMU thread to SD thread
 *we assume the buffer never full, so buffer remain space is never checked
 */
volatile bool dataReady = false;
#define BUFFER_SIZE               100
#define BULK_WRITE_SIZE           50
DataItem data_list[BUFFER_SIZE];
volatile int head_point = 0;
volatile int tail_point = 0;  
int buf_len = 0;  
uint8_t sd_buffer[BULK_WRITE_SIZE*sizeof(DataItem)+50];
SemaphoreHandle_t bufMutex;

unsigned long currentTime;
unsigned long nextSampleTime;


void readIMUTask(void *parameter);
void writeSDTask(void *parameter);
void onSensorDataReady();
String padStart(String str, unsigned int targetLength, char padChar);
void setup() {
  Serial.begin(115200);
  Wire.begin();
  //Wire.setClock(100000); 
  if (!lsm6ds.begin_I2C(0x6B)) {
    Serial.println("Failed to find LSM6DSL chip");
    while (1) { delay(10); }
  }
  Serial.println("LSM6DSL found!");

//  Serial.println("Initializing SD card...");
//  if (!SD.begin()) {
//    Serial.println("Initialization failed!");
//    while (1) { delay(10); }
//  }
//  Serial.println("SD card initialized.");

  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.println("Initializing SD card...");

  for (unsigned int i = 0; i < 10000; i++) {
    fileName = baseName + padStart(String(i),4, '0') + ".bin";
    if (!SD_MMC.exists(fileName.c_str())) {
      // 找到了一个不存在的文件名，现在创建并写入数据
      file = SD_MMC.open(fileName.c_str(), FILE_WRITE);
      if (file) {
        Serial.println("Data written to " + fileName);
      } else {
        Serial.println("Error creating " + fileName);
        return;
      }
      break; //
    }
  }
  
  lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  lsm6ds.setAccelDataRate(LSM6DS_RATE_104_HZ); // 设置加速度计的数据速率为100Hz左右
  lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  lsm6ds.setGyroDataRate(LSM6DS_RATE_104_HZ); // 设置陀螺仪的数据速率为100Hz左右
  lsm6ds.configInt1(false, false, true);

  Serial.println("Initialization done.");

  pinMode(BLUE_LED,OUTPUT);
  
  lsm6ds.getEvent(&accel, &gyro, &temp);

  bufMutex = xSemaphoreCreateMutex();
    
  // 确保互斥量创建成功
  if (bufMutex == NULL) {
      Serial.println("Mutex creation failed.");
      while(1);
  }
  nextSampleTime = millis()+IMU_SAMPLING_INTERVAL;
  xTaskCreatePinnedToCore(readIMUTask, "Read Task", 2048, NULL, 3, NULL,0);
  xTaskCreatePinnedToCore(writeSDTask, "Write Task", 2048, NULL, 2, NULL,1);
}


void readIMUTask(void *parameter) {
  
  while(true){
    currentTime = millis();
    if(currentTime>=nextSampleTime){
      nextSampleTime = nextSampleTime+IMU_SAMPLING_INTERVAL;
      if (xSemaphoreTake(bufMutex, portMAX_DELAY) == pdTRUE) {
        while(!lsm6ds.getEvent(&accel, &gyro, &temp)){
          Serial.println("Retry I2C reading");
        }
        data_list[head_point].timestamp = accel.timestamp;
        data_list[head_point].acc_x = accel.acceleration.x;
        data_list[head_point].acc_y = accel.acceleration.y;
        data_list[head_point].acc_z = accel.acceleration.z;
        data_list[head_point].gyro_x = gyro.gyro.x;
        data_list[head_point].gyro_y = gyro.gyro.y;
        data_list[head_point].gyro_z = gyro.gyro.z;

        head_point++;
        if(head_point== BUFFER_SIZE ){
          head_point =0;
        }
        
        xSemaphoreGive(bufMutex);
      }
      
//      Serial.println(currentTime);
    }else{
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
  }
  
}

void writeSDTask(void *parameter){
  size_t copy_counter;
  size_t written;
  int item_size = sizeof(DataItem);
  while(true){
    if (xSemaphoreTake(bufMutex, portMAX_DELAY) == pdTRUE) {
      buf_len = head_point - tail_point;
     
      if(buf_len<0){
        buf_len += BUFFER_SIZE;
      }
      if(buf_len >=(BULK_WRITE_SIZE+1)){
        copy_counter = 0;
        for(int i=0;i<BULK_WRITE_SIZE;i++){
          memcpy(sd_buffer+copy_counter, (uint8_t *)(data_list+tail_point),item_size);
          copy_counter+=item_size;
          tail_point++;
          if(tail_point ==BUFFER_SIZE){
            tail_point =0;
          }
        }
        xSemaphoreGive(bufMutex); 
        written = file.write(sd_buffer, copy_counter);
        if(written ==copy_counter){
          file.flush();
        }else{
          Serial.println("SD card write failed!");
          while (1) { delay(10); }
        }
        
        digitalWrite(BLUE_LED, !digitalRead(BLUE_LED));
        Serial.print("write ");Serial.println(millis());
        
      }else{
        xSemaphoreGive(bufMutex); 
      }
      
    }
    vTaskDelay(1 / portTICK_PERIOD_MS); /*MUST give other thread some time*/ 
    
    
  }
}



String padStart(String str, unsigned int targetLength, char padChar) {
  while (str.length() < targetLength) {
    str = String(padChar) + str;
  }
  return str;
}

void loop() {

}
