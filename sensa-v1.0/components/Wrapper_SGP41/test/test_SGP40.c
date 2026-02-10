#include "unity.h"
#include "sgp40_i2c.h"
#include "sensirion_i2c_hal.h"
#include "sensirion_common.h"
#include "sensirion_config.h"

void setUp(void) {
   sensirion_i2c_hal_init();
}

void tearDown(void) {
   sensirion_i2c_hal_free();
}


TEST_CASE("SGP40_Test_measure_raw_signal", "[SGP40]")
{
   int16_t error;
   uint16_t relative_humidity = 0;
   uint16_t temperature = 0;
   uint16_t sraw_voc;
   error = sgp40_measure_raw_signal(relative_humidity, temperature, &sraw_voc);
   TEST_ASSERT_EQUAL(error, 0);
   printf("Sraw voc: %u\n", sraw_voc);
}

TEST_CASE("SGP40_Test_execute_self_test", "[SGP40]")
{
   int16_t error;
   uint16_t test_result;
   error = sgp40_execute_self_test(&test_result);
   TEST_ASSERT_EQUAL(error, 0);
   printf("Test result: %u\n", test_result);
}

TEST_CASE("SGP40_Test_turn_heater_off", "[SGP40]") {
   int16_t error;
   error = sgp40_turn_heater_off();
   TEST_ASSERT_EQUAL(error, 0);
}

TEST_CASE("SGP40_Test_get_serial_number", "[SGP40]") {
   int16_t error;
   uint16_t serial_number[42];
   uint8_t serial_number_size = 42;
   error = sgp40_get_serial_number(&serial_number[0], serial_number_size);
   TEST_ASSERT_EQUAL(error, 0);

   printf("Serial number: ");
   for (size_t i = 0; i < serial_number_size; i++) {
       printf("%u, ", serial_number[i]);
   }
   printf("\n");
}