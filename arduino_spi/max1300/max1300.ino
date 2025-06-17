#include <SPI.h>

// #define DEBUG

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
    #ifdef DEBUG
    Serial.print(command, BIN);
    #endif
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(command);
    SPI.transfer(0);
    digitalWrite(CS_PIN, HIGH);
}

void adc_configure_channel(byte channel, ChannelMode mode, InputRange range) {
    byte command = (1 << 7) | ((channel & 0b111) << 4) | (byte(mode) << 3) | byte(range);
    #ifdef DEBUG
    Serial.print(command, BIN);
    #endif
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(command);
    SPI.transfer(0);
    digitalWrite(CS_PIN, HIGH);
}

uint16_t adc_read_channel(byte channel) {
    // byte command = (1 << 7) | ((channel & 0b111) << 4);
    byte command = (1 << 7) | ((channel & 0b111) << 4);
    #ifdef DEBUG
    Serial.print(command, BIN);
    #endif
    digitalWrite(CS_PIN, LOW);

    #if 1

    // SPI.transfer16(command << 8);
    // uint16_t rv = SPI.transfer16(0);

    Serial.println();
    uint16_t rv = SPI.transfer16(command << 8);
    Serial.println(rv, BIN);
    rv = SPI.transfer16(command << 8);
    Serial.println(rv, BIN);
    rv = SPI.transfer16(command << 8);
    Serial.println(rv, BIN);
    rv = SPI.transfer16(0);
    Serial.println(rv, BIN);

    // SPI.transfer(command);
    // SPI.transfer(0);
    // byte high = SPI.transfer(0);
    // byte low  = SPI.transfer(command);
    //
    // uint16_t rv = (high << 8) | low;
    // Serial.println(rv, BIN);
    //
    // SPI.transfer(0);
    // SPI.transfer(0);
    // high = SPI.transfer(0);
    // low  = SPI.transfer(command);
    // rv = (high << 8) | low;
    // Serial.println(rv, BIN);

    // Serial.println();
    // SPI.transfer(command);
    // SPI.transfer(0);
    // byte high = SPI.transfer(0);
    // byte low  = SPI.transfer(0);

    // uint16_t rv = (high << 8) | low;
    // Serial.println(rv, BIN);

    digitalWrite(CS_PIN, HIGH);
    return rv;

    #else

    // byte high = SPI.transfer(command);
    // byte low  = SPI.transfer(0);

    SPI.transfer(command);
    SPI.transfer(0);
    byte high = SPI.transfer(0);
    byte low  = SPI.transfer(0);

    digitalWrite(CS_PIN, HIGH);
    return (high << 8) | low;
    #endif

}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    pinMode(A0,     INPUT);
    pinMode(CS_PIN, OUTPUT);
    pinMode(MISO,   INPUT);
    pinMode(MOSI,   OUTPUT);
    pinMode(SCK,    OUTPUT);

    // digitalWrite(CS_PIN, LOW);
    digitalWrite(CS_PIN, HIGH);

    // Serial.println(SPCR, BIN);
    SPI.begin();
    // Serial.println(SPCR, BIN);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    // Serial.println(SPCR, BIN);

    // delay(10000);

    #if 0
    adc_set_mode(ADCMode::Reset);

    #else

    // delay(10);
    SPI.transfer16(1 << 15);
    adc_set_mode(ADCMode::ExternalClock);
    for (size_t i = 0; i < 8; i++)
        adc_configure_channel(i, ChannelMode::SingleEnded, InputRange::Positive3HalfVref);
    #endif
}


void loop() {
    // adc_set_mode(ADCMode::Reset);
    // Serial.println();
    // adc_set_mode(ADCMode::ExternalClock);
    // Serial.println();

    #if 0
    unsigned long start_time = micros();
    for (size_t i = 0; i < 10000; i++) {
        uint16_t spi_data = adc_read_channel(0);
        if (spi_data < 28000) {
            Serial.println("Transmission Error");
        }
        // Serial.print(spi_data);
        // Serial.print(' ');
    }
    unsigned long end_time = micros();
    char buffer[64];
    // sprintf(buffer, "%.6e", double(end_time - start_time) / 10000);
    Serial.println(double(end_time - start_time) / 10000);
    // delay(2000);
    // Serial.println();

    #elif 1
    uint16_t spi_data = adc_read_channel(0);
    Serial.print("AnalogPin: ");
    Serial.print(analogRead(A0) * 3.3 / 1023);
    Serial.print("; MAX1300: ");
    Serial.print(float(spi_data) * 4.096 / 65536);
    // Serial.print(" | ");
    // Serial.println(spi_data, BIN);
    Serial.println();
    delay(100);
    #else
    for (size_t i = 0; i < 8; i++) {
        // adc_configure_channel(i, ChannelMode::SingleEnded, InputRange::Positive3HalfVref);
        // Serial.print(" | ");
        uint16_t spi_data = adc_read_channel(i);
        // uint16_t spi_data = SPI.transfer16(1 << 15);
        Serial.print(" | ");

        Serial.print(analogRead(A0));
        Serial.print(" | ");
        Serial.print(float(spi_data) * 4.096 / 65536);
        Serial.print(" | ");
        Serial.println(spi_data, BIN);
    }

    // delay(100);
    Serial.println();
    #endif
}
