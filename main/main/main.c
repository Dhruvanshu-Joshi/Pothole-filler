// Components
#include "sra_board.h"
//#define debug
#include "ultrasonic.h"
// C Headers
#include <stdio.h>
#include <math.h>

#define MAX_PWM (80.0f)
#define MIN_PWM (60.0f)

#define TAG "COMMS_WEBSOCKET_SERVER"

#include "comms_websocket_server.h"

#define MAX_DISTANCE_CM 500 // 5m max

#if defined(CONFIG_IDF_TARGET_ESP8266)
#define TRIGGER_GPIO 4
#define ECHO_GPIO 5
#else
#define TRIGGER_GPIO 23
#define ECHO_GPIO 22
#endif

static servo_config servo_a = {
    .servo_pin = SERVO_A,
    .min_pulse_width = CONFIG_SERVO_A_MIN_PULSEWIDTH,
    .max_pulse_width = CONFIG_SERVO_A_MAX_PULSEWIDTH,
    .max_degree = CONFIG_SERVO_A_MAX_DEGREE,
    .mcpwm_num = MCPWM_UNIT_0,
    .timer_num = MCPWM_TIMER_0,
    .gen = MCPWM_OPR_A,
};

servo_config servo_b = {
    .servo_pin = SERVO_B,
    .min_pulse_width = CONFIG_SERVO_B_MIN_PULSEWIDTH,
    .max_pulse_width = CONFIG_SERVO_B_MAX_PULSEWIDTH,
    .max_degree = CONFIG_SERVO_B_MAX_DEGREE,
    .mcpwm_num = MCPWM_UNIT_0,
    .timer_num = MCPWM_TIMER_0,
    .gen = MCPWM_OPR_B,
};
void ultrasonic_test()
{
    ultrasonic_sensor_t sensor = {
        .trigger_pin = TRIGGER_GPIO,
        .echo_pin = ECHO_GPIO};
    int angle;
    ultrasonic_init(&sensor);

    float distance;
    esp_err_t res = ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &distance);
    if (res != ESP_OK)
    {
        printf("Error %d: ", res);
        switch (res)
        {
        case ESP_ERR_ULTRASONIC_PING:
            printf("Cannot ping (device is in invalid state)\n");
            break;
        case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
            printf("Ping timeout (no device found)\n");
            break;
        case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
            printf("Echo timeout (i.e. distance too big)\n");
            break;
        default:
            printf("%s\n", esp_err_to_name(res));
        }
    }
    else
    {
        enable_servo();
        printf("Distance: %0.04f m\n", distance);
        angle = (distance - 0.08) * 600;
        if (angle >= 90)
            angle = 90;
        else
            angle = angle;
        // for(int i=0;i<45;i++)
        // {
        //     set_angle_servo(&servo_b, i);
        //     vTaskDelay(3);
        // }
        for (int i = 0; i < angle; i++)
        {
            set_angle_servo(&servo_a, i);
        }

        for (int i = angle; i > 0; i--)
        {
            set_angle_servo(&servo_a, i);
        }
        // set_angle_servo(&servo_b, 0);
        // for(int i=45;i<0;i++)
        // {
        //     set_angle_servo(&servo_b, i);
        //     vTaskDelay(3);
        // }

        vTaskDelay(50);
    }
}

void follow_commands_task(void *arg)
{
    int b_curr_ang = 0, p = 0;
    ESP_ERROR_CHECK(enable_motor_driver(a, NORMAL_MODE));
    while (true)
    {
        if (read_comms().val_changed)
        {
            float speed = read_comms().speed;
            // print to console
            ESP_LOGI(TAG, "speed: %f", speed);
            float motor_pwm = bound((speed), MIN_PWM, MAX_PWM);

            if (read_comms().left_servo)
            {
                ESP_LOGI(TAG, "left_servo");

                for (int i = b_curr_ang; i < b_curr_ang + 5; i++)
                {
                    set_angle_servo(&servo_b, i);
                    vTaskDelay(5);
                }
                vTaskDelay(20);
                if (b_curr_ang < 90)
                {
                    b_curr_ang += 5;
                }
                // reset_val_changed_coms();
            }

            if (read_comms().right_servo)
            {
                ESP_LOGI(TAG, "right_servo");
                for (int i = b_curr_ang; i > b_curr_ang - 5; i--)
                {
                    set_angle_servo(&servo_b, i);
                    vTaskDelay(5);
                }
                vTaskDelay(20);
                if (b_curr_ang > 0)
                {
                    b_curr_ang -= 5;
                }
                // reset_val_changed_coms();
            }

            if (read_comms().front)
            {
                ESP_LOGI(TAG, "forward");
                ESP_ERROR_CHECK(set_motor_speed(MOTOR_A_0, MOTOR_FORWARD, motor_pwm));
                ESP_ERROR_CHECK(set_motor_speed(MOTOR_A_1, MOTOR_FORWARD, motor_pwm));
                reset_val_changed_coms();
                // printf("Going Forward");
            }
            else if (read_comms().back)
            {
                ESP_LOGI(TAG, "backward");
                ESP_ERROR_CHECK(set_motor_speed(MOTOR_A_0, MOTOR_BACKWARD, motor_pwm));
                ESP_ERROR_CHECK(set_motor_speed(MOTOR_A_1, MOTOR_BACKWARD, motor_pwm));
                reset_val_changed_coms();
                // printf("Going Backwards");
            }
            else if (read_comms().right_turn)
            {
                ESP_LOGI(TAG, "right turn");
                ESP_ERROR_CHECK(set_motor_speed(MOTOR_A_0, MOTOR_FORWARD, motor_pwm));
                ESP_ERROR_CHECK(set_motor_speed(MOTOR_A_1, MOTOR_BACKWARD, motor_pwm));
                reset_val_changed_coms();
                // printf("Going right");
            }
            else if (read_comms().left_turn)
            {
                ESP_LOGI(TAG, "left turn");
                ESP_ERROR_CHECK(set_motor_speed(MOTOR_A_0, MOTOR_BACKWARD, motor_pwm));
                ESP_ERROR_CHECK(set_motor_speed(MOTOR_A_1, MOTOR_FORWARD, motor_pwm));
                reset_val_changed_coms();
                // printf("Going left");
            }

            else
            {
                ESP_LOGI(TAG, "stop");
                ESP_ERROR_CHECK(set_motor_speed(MOTOR_A_0, MOTOR_STOP, 0));
                ESP_ERROR_CHECK(set_motor_speed(MOTOR_A_1, MOTOR_STOP, 0));
                reset_val_changed_coms();
                // printf("Imm'a stop");
            }

            // p = b_curr_ang;
            // while (read_comms().left_servo)
            // {
            // 	ESP_LOGI(TAG, "left_servo");
            // 	ESP_ERROR_CHECK(set_angle_servo(&servo_b, p));
            // 	vTaskDelay(10);

            // 	p++;
            // }

            // b_curr_ang = p;
            // p = b_curr_ang;
            // while (read_comms().right_servo)
            // {
            // 	ESP_LOGI(TAG, "right_servo");
            // 	ESP_ERROR_CHECK(set_angle_servo(&servo_b, p));
            // 	vTaskDelay(10);

            // 	p--;
            // }

            reset_val_changed_coms();
        }
        // vTaskDelay(10);
        ultrasonic_test();
    }
    vTaskDelete(NULL);
}

void app_main()
{
    start_websocket_server();

    xTaskCreate(&follow_commands_task, "follow commands task", 4096, NULL, 1, NULL);
}

// void app_main()
// {
//     xTaskCreate(ultrasonic_test, "ultrasonic_test", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
// }