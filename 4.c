/*
 * Программа для ATmega328PB, реализующая отображение символов на 4-разрядном 7-сегментном индикаторе
 * с использованием таймера и прерываний. Без функции delay.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

// Символы: [a,b,c,d,e,f,g]
const uint8_t symbols[8][7] = {
  {1, 0, 0, 1, 0, 0, 1},
  {0, 0, 0, 1, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 1, 1, 1},
  {0, 1, 1, 0, 1, 1, 1},
  {0, 0, 1, 1, 1, 0, 1},
  {0, 1, 1, 0, 0, 0, 0},
  {1, 1, 1, 1, 0, 0, 1}
};

volatile int displayBuffer[4] = {-1, -1, -1, -1};
volatile uint8_t currentDigit = 0;
volatile uint16_t tickCounter = 0;
volatile uint8_t currentSymbol = 0;

void setup_ports() {
    // Сегменты: PD2–PD7 (A–F), PB0 (G), PB1 (DP)
    DDRD |= 0b11111100; // PD2–PD7 на выход (segA–segF)
    DDRB |= (1 << PB0) | (1 << PB1); // PB0 = G, PB1 = DP

    // Разряды: PC0–PC3
    DDRC |= 0b00001111; // digit1–digit4

    // Отключить все разряды и точку
    PORTC |= 0b00001111;
    PORTB &= ~(1 << PB1); // DP = 0
}

void set_segments(uint8_t symbol) {
    const uint8_t *seg = symbols[symbol];
    PORTD = (PORTD & 0b00000011) | // оставить PD0, PD1 нетронутыми
            (seg[0] << PD2) |
            (seg[6] << PD3) |
            (seg[2] << PD4) |
            (seg[3] << PD5) |
            (seg[4] << PD6) |
            (seg[5] << PD7);

    if (seg[1]) PORTB |= (1 << PB0); else PORTB &= ~(1 << PB0); // segB на PB0
}

void show_digit(uint8_t digitIndex) {
    PORTC |= 0b00001111; // выключить все разряды
    if (displayBuffer[digitIndex] != -1) {
        set_segments(displayBuffer[digitIndex]);
        PORTC &= ~(1 << digitIndex); // включить текущий разряд
    }
}

void setup_timer() {
    TCCR0A = 0; // Нормальный режим
    TCCR0B = (1 << CS01); // Предделитель 8 → 1 тик = 0.5 мкс
    TIMSK0 = (1 << TOIE0); // Разрешить прерывание по переполнению
    TCNT0 = 0;
}

ISR(TIMER0_OVF_vect) {
    static uint8_t localCounter = 0;

    show_digit(currentDigit);
    currentDigit = (currentDigit + 1) % 4;

    tickCounter++;
    if (tickCounter >= 2170) { // примерно 2.17 секунды (по факту зависит от частоты)
        tickCounter = 0;

        for (int i = 0; i < 3; i++) displayBuffer[i] = displayBuffer[i+1];
        displayBuffer[3] = currentSymbol;
        currentSymbol = (currentSymbol + 1) % 8;
    }
}

int main(void) {
    setup_ports();
    setup_timer();
    sei(); // Глобальное разрешение прерываний

    while (1) {
        // Основной цикл пустой — всё работает через прерывания
    }

    return 0;
}
