#include <SPI.h>

// #define SPI_DEBUG

// Arduino Giga
#if 0
#define CS_PIN 10
#define Serial SerialUSB
#define SPI SPI1

// Arduino Mega
#else
#define CS_PIN 53
#endif


enum class ADCMode: byte {
    ExternalClock       = 0b000,
    ExternalAcquisition = 0b001,
    InternalClock       = 0b010,
    Reset               = 0b100,
    PartialPowerDown    = 0b110,
    FullPowerDown       = 0b111,
};

enum class ChannelMode: byte {
    SingleEnded  = 0,
    Differential = 1,
};

enum class InputRange: byte {
    Centered3HalfVref = 0b001,
    Negative3HalfVref = 0b010,
    Positive3HalfVref = 0b011,
    Centered3Vref     = 0b100,
    Negative3Vref     = 0b101,
    Positive3Vref     = 0b110,
    Centered6Vref     = 0b111,
};


void adc_set_mode(ADCMode mode) {
    byte command = (1 << 7) | (byte(mode) << 4) | (1 << 3);
    #ifdef SPI_DEBUG
    Serial.print(command, BIN);
    #endif
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(command);
    SPI.transfer(0);
    digitalWrite(CS_PIN, HIGH);
}

void adc_configure_channel(byte channel, ChannelMode mode, InputRange range) {
    byte command = (1 << 7) | ((channel & 0b111) << 4) | (byte(mode) << 3) | byte(range);
    #ifdef SPI_DEBUG
    Serial.print(command, BIN);
    #endif
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(command);
    SPI.transfer(0);
    digitalWrite(CS_PIN, HIGH);
}

uint16_t adc_read_channel(byte channel) {
    byte command = (1 << 7) | ((channel & 0b111) << 4);
    #ifdef SPI_DEBUG
    Serial.print(command, BIN);
    #endif
    digitalWrite(CS_PIN, LOW);
    SPI.transfer16(command << 8);
    uint16_t rv = SPI.transfer16(0);
    digitalWrite(CS_PIN, HIGH);
    return rv;
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    pinMode(A0,     INPUT);
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);

    SPI.begin();
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    adc_set_mode(ADCMode::Reset);
    adc_set_mode(ADCMode::ExternalClock);
    for (size_t i = 0; i < 8; i++)
        adc_configure_channel(i, ChannelMode::SingleEnded, InputRange::Positive3HalfVref);
}


void loop() {

    // Read channel every 100ms, compare to A0 pin
    #if 1
    uint16_t spi_data = adc_read_channel(0);
    Serial.print("AnalogPin: ");
    Serial.print(analogRead(A0) * 3.3 / 1023);
    Serial.print("; MAX1300: ");
    Serial.print(float(spi_data) * 4.096 / 65536);
    Serial.println();
    delay(100);

    // Benchmark
    #else

    const size_t ITER_COUNT = 10000;
    unsigned long start_time = micros();
    for (size_t i = 0; i < ITER_COUNT; i++) {
        uint16_t spi_data = adc_read_channel(0);
        if (spi_data < 28000) {
            Serial.println("Transmission Error");
        }
    }
    unsigned long end_time = micros();
    char buffer[64];
    // sprintf(buffer, "%.6e", double(end_time - start_time) / 10000);
    Serial.println(double(end_time - start_time) / ITER_COUNT);

    #endif
}
