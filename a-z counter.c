#include "stm32f4xx.h"
#include <stdint.h>

// MAX7219 registers
#define REG_NOOP        0x00
#define REG_DIGIT0      0x01
#define REG_DIGIT1      0x02
#define REG_DIGIT2      0x03
#define REG_DIGIT3      0x04
#define REG_DIGIT4      0x05
#define REG_DIGIT5      0x06
#define REG_DIGIT6      0x07
#define REG_DIGIT7      0x08
#define REG_DECODE_MODE 0x09
#define REG_INTENSITY   0x0A
#define REG_SCAN_LIMIT  0x0B
#define REG_SHUTDOWN    0x0C
#define REG_DISPLAY_TEST 0x0F

// Pin definitions based on the schematic
#define DIN_PIN     0  // PA0 connected to DIN
#define CS_PIN      1  // PA1 connected to LOAD/CS
#define CLK_PIN     2  // PA2 connected to CLK

// Function prototypes
void delay(uint32_t ms);
void send_byte(uint8_t data);
void send_cmd(uint8_t reg, uint8_t data);
void init_max7219(void);
void clear_display(void);
void display_letter(uint8_t letter_idx);
void SystemClock_Config(void);

// Delay function
void delay(uint32_t ms) {
    // Çok daha düşük kalibrasyon değeri - gerçek 1ms zamanlamayı elde etmek için
    volatile uint32_t cycles = ms * 4000; // Çok daha düşük değer kullanıyoruz
    while(cycles--) {
        __NOP();
    }
}

// Send a byte to MAX7219
void send_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        // Clear clock
        GPIOA->BSRR = (1 << (CLK_PIN + 16)); // Reset CLK
        
        // Set data bit
        if (data & 0x80)
            GPIOA->BSRR = (1 << DIN_PIN); // Set DIN
        else
            GPIOA->BSRR = (1 << (DIN_PIN + 16)); // Reset DIN
        
        // Toggle clock
        GPIOA->BSRR = (1 << CLK_PIN); // Set CLK
        
        // Shift to next bit
        data <<= 1;
    }
}

// Send command to MAX7219
void send_cmd(uint8_t reg, uint8_t data) {
    // Select the device (CS low)
    GPIOA->BSRR = (1 << (CS_PIN + 16)); // Reset CS
    
    // Send register and data
    send_byte(reg);
    send_byte(data);
    
    // Deselect the device (CS high)
    GPIOA->BSRR = (1 << CS_PIN); // Set CS
}

// Initialize MAX7219
void init_max7219(void) {
    // Set decode mode: no decode for digits 0-7
    send_cmd(REG_DECODE_MODE, 0x00);
    
    // Set scan limit: all digits (0-7) enabled
    send_cmd(REG_SCAN_LIMIT, 0x07);
    
    // Set intensity (0x00 to 0x0F)
    send_cmd(REG_INTENSITY, 0x08); // Parlaklığı biraz düşürelim
    
    // Exit shutdown mode
    send_cmd(REG_SHUTDOWN, 0x01);
    
    // Exit display test
    send_cmd(REG_DISPLAY_TEST, 0x00);
    
    // Clear display
    clear_display();
}

// Clear the display
void clear_display(void) {
    for (int i = 1; i <= 8; i++) {
        send_cmd(i, 0x00);
    }
}

// Letter patterns for modified Turkish alphabet (23 letters: a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, r, s, t, u, v, y, z)
const uint8_t letters[23][8] = {
    // a - index 0
    {
        0b00000000,
        0b00000000,
        0b00111000,
        0b00000100,
        0b00111100,
        0b01000100,
        0b01000100,
        0b00111100
    },
    // b - index 1
    {
        0b01000000,
        0b01000000,
        0b01111000,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01111000
    },
    // c - index 2
    {
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00111100
    },
    // d - index 3
    {
        0b00000100,
        0b00000100,
        0b00111100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b00111100
    },
    // e - index 4
    {
        0b00000000,
        0b00000000,
        0b00111000,
        0b01000100,
        0b01111100,
        0b01000000,
        0b01000000,
        0b00111100
    },
    // f - index 5
    {
        0b00011100,
        0b00100000,
        0b00100000,
        0b01111000,
        0b00100000,
        0b00100000,
        0b00100000,
        0b00100000
    },
    // g - index 6
    {
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000100,
        0b01000100,
        0b00111100,
        0b00000100,
        0b00111000
    },
    // h - index 7
    {
        0b01000000,
        0b01000000,
        0b01000000,
        0b01111000,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100
    },
    // i - index 8
    {
        0b00010000,
        0b00000000,
        0b00110000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00111000
    },
    // j - index 9
    {
        0b00000100,
        0b00000000,
        0b00001100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b01000100,
        0b00111000
    },
    // k - index 10
    {
        0b01000000,
        0b01000000,
        0b01000100,
        0b01001000,
        0b01110000,
        0b01001000,
        0b01000100,
        0b01000100
    },
    // l - index 11
    {
        0b00110000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00111000
    },
    // m - index 12
    {
        0b00000000,
        0b00000000,
        0b01101000,
        0b01010100,
        0b01010100,
        0b01010100,
        0b01010100,
        0b01010100
    },
    // n - index 13
    {
        0b00000000,
        0b00000000,
        0b01111000,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100
    },
    // o - index 14
    {
        0b00000000,
        0b00000000,
        0b00111000,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b00111000
    },
    // p - index 15
    {
        0b00000000,
        0b00000000,
        0b01111000,
        0b01000100,
        0b01000100,
        0b01111000,
        0b01000000,
        0b01000000
    },
    // r - index 16
    {
        0b00000000,
        0b00000000,
        0b01011100,
        0b01100000,
        0b01000000,
        0b01000000,
        0b01000000,
        0b01000000
    },
    // s - index 17
    {
        0b00000000,
        0b00000000,
        0b00111100,
        0b01000000,
        0b00111000,
        0b00000100,
        0b00000100,
        0b01111000
    },
    // t - index 18
    {
        0b00010000,
        0b00010000,
        0b00111000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00010000,
        0b00001100
    },
    // u - index 19
    {
        0b00000000,
        0b00000000,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b00111100
    },
    // v - index 20
    {
        0b00000000,
        0b00000000,
        0b01000100,
        0b01000100,
        0b01000100,
        0b01000100,
        0b00101000,
        0b00010000
    },
    // y - index 21
    {
        0b00000000,
        0b00000000,
        0b01000100,
        0b01000100,
        0b01000100,
        0b00111100,
        0b00000100,
        0b00111000
    },
    // z - index 22
    {
        0b00000000,
        0b00000000,
        0b01111100,
        0b00000100,
        0b00001000,
        0b00010000,
        0b00100000,
        0b01111100
    }
};

// Display a letter (a-z) on the matrix
void display_letter(uint8_t letter_idx) {
    // Safety check - ensure letter_idx is in bounds
    if (letter_idx >= 23) {
        letter_idx = 0;
    }
    
    // Direkt harfi göster - ekranı önceden temizleme yok
    for (int row = 0; row < 8; row++) {
        send_cmd(row + 1, letters[letter_idx][row]);
    }
}

// System clock configuration to 84MHz using PLL
void SystemClock_Config(void) {
    // Enable HSI
    RCC->CR |= RCC_CR_HSION;
    while(!(RCC->CR & RCC_CR_HSIRDY));
    
    // Configure PLL (HSI/16 * 336 / 4 = 84 MHz)
    RCC->PLLCFGR = (16 << RCC_PLLCFGR_PLLN_Pos) | 
                   (4 << RCC_PLLCFGR_PLLM_Pos) | 
                   (7 << RCC_PLLCFGR_PLLQ_Pos) | 
                   RCC_PLLCFGR_PLLSRC_HSI;
    
    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));
    
    // Configure Flash latency
    FLASH->ACR = FLASH_ACR_LATENCY_2WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
    
    // Select PLL as system clock
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
    
    // Update SystemCoreClock variable
    SystemCoreClockUpdate();
}

int main(void) {
    // Configure system clock
    SystemClock_Config();
    
    // Enable clock for GPIOA
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    
    // Configure PA0 (DIN), PA1 (CS), PA2 (CLK) as output
    GPIOA->MODER |= (GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0);
    
    // Set output speed
    GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR0_0 | GPIO_OSPEEDER_OSPEEDR1_0 | GPIO_OSPEEDER_OSPEEDR2_0);
    
    // Initial state: CS high, CLK low, DIN low
    GPIOA->BSRR = (1 << CS_PIN);              // CS high
    GPIOA->BSRR = (1 << (CLK_PIN + 16));      // CLK low
    GPIOA->BSRR = (1 << (DIN_PIN + 16));      // DIN low
    
    // Initialize MAX7219
    init_max7219();
    
    // Ekranı bir kez temizle - başlangıçta
    clear_display();
    
    uint8_t letter_idx = 0;  // 'a' ile başla
    
    while (1) {
        // Geçerli harfi göster
        display_letter(letter_idx);
        
        // Tam 1 saniye bekle - yeni kalibrasyon değerine göre ayarlandı
        delay(1000);
        
        // Sonraki harfe ilerle
        letter_idx++;
        
        // Sınırları kontrol et
        if (letter_idx >= 23) {
            letter_idx = 0; // Başa dön
        }
    }
}
