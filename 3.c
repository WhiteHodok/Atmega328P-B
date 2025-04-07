#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

// Определение выводов (DIG4 - катод, как требуется в задании)
#define DIG4 PD2  // DIG4 управляется PD2 (выбран по условию: (19 mod 4) + 1 = 4)

// Определение сегментов
#define SEG_A PD4
#define SEG_F PD5
#define SEG_B PD6
#define SEG_G PD7
#define SEG_DP PB0
#define SEG_C PB1
#define SEG_E PB2
#define SEG_D PB3

// Массив символов для отображения (0-7, как в задании)
const uint8_t symbols[] = {
    (1 << SEG_G) | (1 << SEG_D) | (1 << SEG_A),                   // 0
    (1 << SEG_F) | (1 << SEG_E) | (1 << SEG_D),                   // 1
    (1 << SEG_F) | (1 << SEG_E) | (1 << SEG_G) | (1 << SEG_C) | (1 << SEG_D), // 2
    (1 << SEG_A) | (1 << SEG_F) | (1 << SEG_B) | (1 << SEG_C) | (1 << SEG_G) | (1 << SEG_E), // 3
    (1 << SEG_F) | (1 << SEG_E) | (1 << SEG_B) | (1 << SEG_C) | (1 << SEG_G),  // 4
    (1 << SEG_G) | (1 << SEG_E) | (1 << SEG_D) | (1 << SEG_C),    // 5
    (1 << SEG_B) | (1 << SEG_C),                                  // 6
    (1 << SEG_A) | (1 << SEG_B) | (1 << SEG_C) | (1 << SEG_D) | (1 << SEG_G)   // 7
};
#define NUM_SYMBOLS (sizeof(symbols)/sizeof(symbols[0]))

volatile uint8_t symbol_index = 0;  // Текущий символ из массива symbols

// Инициализация таймера 1 (режим переполнения, делитель 1024)
void timer1_init() {
    cli();                          // Шаг 5: выключение прерываний
    TCNT1 = 31630;                  // Шаг 6: начальное значение для 2.17 с (65536 - 16000000/1024*2.17)
    TCCR1B |= (1 << CS12) | (1 << CS10); // Шаг 7: делитель 1024 (CS12=1, CS10=1)
    TIMSK1 |= (1 << TOIE1);         // Разрешение прерывания по переполнению
    sei();                          // Шаг 8: включение глобальных прерываний
}

// Обработчик прерывания таймера 1 (переполнение)
ISR(TIMER1_OVF_vect) {             // Шаг 10: обработчик прерывания
    TCNT1 = 31630;                  // Шаг 14: перезагрузка счетчика для 2.17 с

    // Шаг 9: вывод символа на DIG4
    PORTD &= ~(1 << DIG4);          // Отключаем текущий катод
    PORTD = (PORTD & 0x03) | (symbols[symbol_index] << 4); // Устанавливаем сегменты (PD4-PD7)
    PORTB = (symbols[symbol_index] >> 4) & 0x0F;           // Устанавливаем сегменты (PB0-PB3)
    PORTD |= (1 << DIG4);           // Включаем DIG4

    symbol_index = (symbol_index + 1) % NUM_SYMBOLS; // Шаг 11: увеличение счетчика
    if (symbol_index == 0) {        // Шаги 12-13: сброс при достижении конца массива
        // Дополнительные действия (если нужны)
    }
}

int main(void) {
    // Настройка выводов
    DDRD |= (1 << DIG4) | (1 << SEG_A) | (1 << SEG_F) | (1 << SEG_B) | (1 << SEG_G);
    DDRB |= (1 << SEG_DP) | (1 << SEG_C) | (1 << SEG_E) | (1 << SEG_D);

    // Инициализация таймера
    timer1_init();

    while (1) {}                    // Бесконечный цикл
}
