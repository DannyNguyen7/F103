/* USER CODE BEGIN Header */
/**
  * @brief  Giả lập tín hiệu cảm biến Inductive (VR) cho CKP & CMP
  *         Dùng DAC + Timer ngắt (hoặc DMA)
  *         CKP: 36-3, 16 điểm/răng → 576 điểm/vòng
  *         CMP: 32 điểm/răng → 72 răng x 32 = 2304 điểm/vòng cam = 2 vòng khuỷu
  *         RPM điều chỉnh bằng biến trở (PA0)
  */
/* USER CODE END Header */

#include "main.h"
#include "dac.h"
#include "adc.h"
#include "tim.h"

#define CKP_TEETH_TOTAL       36
#define CKP_MISSING           3
#define CKP_POINTS_PER_TOOTH  16
#define CKP_TOTAL_POINTS      (CKP_TEETH_TOTAL * CKP_POINTS_PER_TOOTH)  // 576

#define CAM_POINTS_PER_TOOTH  32
#define CAM_ARRAY_SIZE        72
#define CAM_TOTAL_POINTS      (CAM_ARRAY_SIZE * CAM_POINTS_PER_TOOTH)   // 2304

// Biến toàn cục
float current_rpm = 1000.0f;
uint32_t ckp_index = 0;
uint32_t cam_index = 0;

// Mảng lưu giá trị DAC (12-bit: 0-4095)
uint16_t CKP_Sine[CKP_POINTS_PER_TOOTH];
uint16_t CAM_Sine[CAM_POINTS_PER_TOOTH];

// Mảng pattern răng (giống Hall)
const uint8_t CKP_Pattern[CKP_TEETH_TOTAL] = {
    0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    // 3 khuyết + 33 răng
};

// Tính toán bảng sin trước (chạy 1 lần)
void Generate_Sine_Tables(void)
{
    float crk_offset = 0.0f;   // Điện áp offset (V) - chỉnh bằng biến trở ngoài nếu cần
    float cam_offset = 0.0f;
    float crk_amplitude = 2.5f;   // Biên độ CKP ~2.5V
    float cam_amplitude = 5.0f;   // Biên độ CMP gấp đôi ~5V

    for (int i = 0; i < CKP_POINTS_PER_TOOTH; i++)
    {
        float angle = 2.0f * 3.1415926535f * i / CKP_POINTS_PER_TOOTH;
        float volt = 2.048f + crk_offset + crk_amplitude * sinf(angle);
        CKP_Sine[i] = (uint16_t)(volt * 4095.0f / 3.3f);
    }

    for (int i = 0; i < CAM_POINTS_PER_TOOTH; i++)
    {
        float angle = 2.0f * 3.1415926535f * i / CAM_POINTS_PER_TOOTH;
        float volt = 2.048f + cam_offset + cam_amplitude * sinf(angle);
        CAM_Sine[i] = (uint16_t)(volt * 4095.0f / 3.3f);
    }
}

// Cập nhật tần số timer theo RPM
void Update_Timer_Period(float rpm)
{
    if (rpm < 100) rpm = 100;
    if (rpm > 8000) rpm = 8000;

    // Tần số xuất mẫu = (RPM / 60) × số điểm mỗi vòng khuỷu
    float samples_per_second = (rpm / 60.0f) * CKP_TOTAL_POINTS;
    float timer_freq = samples_per_second;

    uint32_t arr = (uint32_t)(84000000.0f / 84.0f / timer_freq) - 1;  // TIM clock 1MHz
    if (arr == 0) arr = 1;
    if (arr > 65535) arr = 65535;

    __HAL_TIM_SET_AUTORELOAD(&htim6, arr);  // Dùng TIM6 làm base timer
}

// Đọc biến trở → cập nhật RPM
void Update_RPM_From_Pot(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint32_t adc = HAL_ADC_GetValue(&hadc1);

    float new_rpm = 600.0f + (adc / 4095.0f) * (8000.0f - 600.0f);

    if (__fabsf(new_rpm - current_rpm) > 15.0f)
    {
        current_rpm = new_rpm;
        Update_Timer_Period(current_rpm);
    }
}

// Ngắt timer chính – xuất DAC mỗi lần ngắt
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        uint16_t dac_val_ckp = 2048;  // Mức 0V (giữa)
        uint16_t dac_val_cam = 2048;

        // === XỬ LÝ TRỤC KHUỶU ===
        uint32_t tooth = ckp_index / CKP_POINTS_PER_TOOTH;
        uint32_t point_in_tooth = ckp_index % CKP_POINTS_PER_TOOTH;

        if (CKP_Pattern[tooth] == 1)
        {
            dac_val_ckp = CKP_Sine[point_in_tooth];
        }
        // else: vùng khuyết → giữ 0V (2048)

        // === XỬ LÝ TRỤC CAM ===
        uint32_t cam_point = cam_index % CAM_TOTAL_POINTS;
        uint32_t cam_tooth = cam_point / CAM_POINTS_PER_TOOTH;
        uint32_t point_in_cam = cam_point % CAM_POINTS_PER_TOOTH;

        // Dùng mảng pattern cam giống Hall
        if (CAM_I[cam_tooth] == 1)
        {
            dac_val_cam = CAM_Sine[point_in_cam];
        }

        // Xuất DAC
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_val_ckp);
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dac_val_cam);

        // Tăng chỉ số
        ckp_index = (ckp_index + 1) % CKP_TOTAL_POINTS;
        cam_index++;
    }

    // Đọc biến trở mỗi 10ms
    if (htim->Instance == TIM7)
    {
        Update_RPM_From_Pot();
    }
}