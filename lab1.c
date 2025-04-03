#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

// Макросы для удобства
#define SET_BIT(PORT, PIN) (PORT |= (1 << PIN))
#define CLR_BIT(PORT, PIN) (PORT &= ~(1 << PIN))

// Переменные состояния
volatile uint8_t current_step = 0;
volatile uint8_t direction = 0; // 0 - even, 1 - odd, 2 - turn off
volatile uint16_t delay_counter = 0;

void init_leds() {
    // Настройка пинов на выход
    DDRB |= (1 << PB1) | (1 << PB2);          // PB1, PB2 - выходы
    DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6);
}

void init_timer1() {
    // Настройка таймера 1 (CTC mode)
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS12); // CTC mode, prescaler 256
    OCR1A = 62500; // Прерывание каждые 1 секунду (16MHz/256/62500 = 1Hz)
    TIMSK1 = (1 << OCIE1A); // Разрешить прерывание по совпадению
}

// Обработчик прерывания таймера 1
ISR(TIMER1_COMPA_vect) {
    if (delay_counter > 0) {
        delay_counter--;
    }
}

void process_led_sequence() {
    if (delay_counter == 0) {
        switch (direction) {
            case 0: // even LEDs
                switch (current_step) {
                    case 0: SET_BIT(PORTB, PB2); break; // LED 1
                    case 1: SET_BIT(PORTD, PD6); break; // LED 3
                    case 2: SET_BIT(PORTD, PD4); break; // LED 5
                    case 3: SET_BIT(PORTD, PD2); break; // LED 7
                    case 4: 
                        direction = 1; // Переключаемся на нечетные
                        current_step = 0;
                        break;
                }
                if (current_step < 4) current_step++;
                break;
                
            case 1: // odd LEDs
                switch (current_step) {
                    case 0: SET_BIT(PORTD, PD3); break; // LED 6
                    case 1: SET_BIT(PORTD, PD5); break; // LED 4
                    case 2: SET_BIT(PORTB, PB1); break; // LED 2
                    case 3: 
                        direction = 2; // Переключаемся на выключение
                        current_step = 0;
                        break;
                }
                if (current_step < 3) current_step++;
                break;
                
            case 2: // turn off LEDs
                switch (current_step) {
                    case 0: CLR_BIT(PORTD, PD2); break; // LED 7
                    case 1: CLR_BIT(PORTD, PD4); break; // LED 5
                    case 2: CLR_BIT(PORTD, PD6); break; // LED 3
                    case 3: CLR_BIT(PORTB, PB2); break; // LED 1
                    case 4: CLR_BIT(PORTD, PD3); break; // LED 6
                    case 5: CLR_BIT(PORTD, PD5); break; // LED 4
                    case 6: CLR_BIT(PORTB, PB1); break; // LED 2
                    case 7: 
                        direction = 0; // Возвращаемся к четным
                        current_step = 0;
                        // Пауза перед началом нового цикла
                        delay_counter = 2; // 2 секунды
                        return;
                }
                current_step++;
                break;
        }
        
        // Устанавливаем задержку 0.5 секунды между шагами
        delay_counter = 1; // 0.5 секунды (прерывание каждые 0.5 сек)
    }
}

int main(void) {
    init_leds();
    init_timer1();
    sei(); // Разрешить прерывания глобально

    while (1) {
        process_led_sequence();
    }
}