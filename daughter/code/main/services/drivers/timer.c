#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"

static void periodic_timer_callback(void* arg);
static void oneshot_timer_callback(void* arg);

// #define LEVEL_0								27500
#define LEVEL_0								100
#define LEVEL_1								21450
#define LEVEL_2								18780
#define LEVEL_3								16260
#define LEVEL_4								13790
#define LEVEL_5								11340
#define LEVEL_6								8910
#define LEVEL_7								6130

int pwm_frequency = LEVEL_0;

void set_blower_pwm_frequency (int level) {
	if (level == 0)	pwm_frequency = LEVEL_0;
	if (level == 1)	pwm_frequency = LEVEL_1;
	if (level == 2)	pwm_frequency = LEVEL_2;
	if (level == 3)	pwm_frequency = LEVEL_3;
	if (level == 4)	pwm_frequency = LEVEL_4;
	if (level == 5)	pwm_frequency = LEVEL_5;
	if (level == 6)	pwm_frequency = LEVEL_6;
	if (level == 7)	pwm_frequency = LEVEL_7;

  printf("set_blower_pwm_frequency:\t%d\t%d\n", level, pwm_frequency);
}

bool on = true;
esp_timer_handle_t oneshot_timer;

static void oneshot_timer_callback(void* arg)
{
	mcp23x17_set_level(&mcp_dev, DRIVER_HEATED_SEAT_STATUS, true);
	mcp23x17_set_level(&mcp_dev, DRIVER_HEATED_SEAT_STATUS, true);
	mcp23x17_set_level(&mcp_dev, DRIVER_HEATED_SEAT_STATUS, true);

    // int64_t time_since_boot = esp_timer_get_time();
    // ESP_LOGI(TAG, "One-shot timer called, time since boot: %lld us", time_since_boot);
    // esp_timer_handle_t periodic_timer_handle = (esp_timer_handle_t) arg;
    // /* To start the timer which is running, need to stop it first */
    // ESP_ERROR_CHECK(esp_timer_stop(periodic_timer_handle));
    // ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer_handle, 1000000));
    // time_since_boot = esp_timer_get_time();
    // ESP_LOGI(TAG, "Restarted periodic timer with 1s period, time since boot: %lld us",
    //         time_since_boot);
}

bool val = true;

static void periodic_timer_callback(void* arg)
{
	  mcp23x17_set_level(&mcp_dev, DRIVER_SEAT_TEMP_CONTROL, val);
		mcp23x17_set_level(&mcp_dev, RECIRCULATION_A, val);
		mcp23x17_set_level(&mcp_dev, RECIRCULATION_B, val);

		val = !val;

    // ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer, pwm_frequency / 2));
}


static void timer_example_evt_task(void *arg)
{

  /* Create two timers:
   * 1. a periodic timer which will run every 0.5s, and print a message
   * 2. a one-shot timer which will fire after 5s, and re-start periodic
   *    timer with period of 1s.
   */

  const esp_timer_create_args_t periodic_timer_args = {
          .callback = &periodic_timer_callback,
          /* name is optional, but may help identify the timer when debugging */
          .name = "periodic"
  };

  esp_timer_handle_t periodic_timer;
  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  /* The timer has been created but is not running yet */

  const esp_timer_create_args_t oneshot_timer_args = {
          .callback = &oneshot_timer_callback,
          /* argument specified here will be passed to timer callback function */
          .arg = (void*) periodic_timer,
          .name = "one-shot"
  };

  ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_timer));

  /* Start the timers */
	printf("\n!!\tStarting timers\t!!\n");
  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, pwm_frequency));
  // ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer, 5000000));
  // ESP_LOGI(TAG, "Started timers, time since boot: %lld us", esp_timer_get_time());

  while (1) {

    vTaskDelay(1000 / portTICK_RATE_MS);
    // ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer, 5000000));
  }

  /* Print debugging information about timers to console every 2 seconds */
  for (int i = 0; i < 5; ++i) {
      ESP_ERROR_CHECK(esp_timer_dump(stdout));
      usleep(2000000);
  }

  /* Timekeeping continues in light sleep, and timers are scheduled
   * correctly after light sleep.
   */
  // ESP_LOGI(TAG, "Entering light sleep for 0.5s, time since boot: %lld us",
  //         esp_timer_get_time());

  ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(500000));
  esp_light_sleep_start();

  // ESP_LOGI(TAG, "Woke up from light sleep, time since boot: %lld us",
  //             esp_timer_get_time());

  /* Let the timer run for a little bit more */
  usleep(2000000);

  /* Clean up and finish the example */
  ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
  ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));
  ESP_ERROR_CHECK(esp_timer_delete(oneshot_timer));
  // ESP_LOGI(TAG, "Stopped and deleted timers");
}


void timer_main(void)
{
		printf("\n!!\tStarting timers0\t!!\n");
    xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);
}
