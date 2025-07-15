#ifndef STM32F10_REGISTERS_H
#define STM32F10_REGISTERS_H

#define PERIPH_BASE           ((unsigned int)0x40000000) /*!< Peripheral base address in the alias region */
#define FLASH_BASE            ((unsigned int)0x08000000) /*!< FLASH base address in the alias region */


/*!< Peripheral memory map */
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x20000)

#define GPIOA_BASE            (APB2PERIPH_BASE + 0x0800)
#define GPIOB_BASE            (APB2PERIPH_BASE + 0x0C00)
#define GPIOC_BASE            (APB2PERIPH_BASE + 0x1000)
#define GPIOD_BASE            (APB2PERIPH_BASE + 0x1400)

#define USART1_BASE           (APB2PERIPH_BASE + 0x3800)

#define RCC_BASE              (AHBPERIPH_BASE + 0x1000)
#define FLASH_R_BASE          (AHBPERIPH_BASE + 0x2000) /*!< Flash registers base address */
#define FLASH               ((FLASH_TypeDef *) FLASH_R_BASE)
#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD               ((GPIO_TypeDef *) GPIOD_BASE)
#define USART1              ((USART_TypeDef *) USART1_BASE)


/** 
  * @brief General Purpose I/O
  */

typedef struct
{
  volatile unsigned int CRL;
  volatile unsigned int CRH;
  volatile unsigned int IDR;
  volatile unsigned int ODR;
  volatile unsigned int BSRR;
  volatile unsigned int BRR;
  volatile unsigned int LCKR;
} GPIO_TypeDef;

typedef struct
{
  volatile unsigned int CR;
  volatile unsigned int CFGR;
  volatile unsigned int CIR;
  volatile unsigned int APB2RSTR;
  volatile unsigned int APB1RSTR;
  volatile unsigned int AHBENR;
  volatile unsigned int APB2ENR;
  volatile unsigned int APB1ENR;
  volatile unsigned int BDCR;
  volatile unsigned int CSR;
  unsigned int RESERVED0;
  volatile unsigned int CFGR2;
} RCC_TypeDef;


typedef struct
{
  volatile unsigned int ACR;
  volatile unsigned int KEYR;
  volatile unsigned int OPTKEYR;
  volatile unsigned int SR;
  volatile unsigned int CR;
  volatile unsigned int AR;
  volatile unsigned int RESERVED;
  volatile unsigned int OBR;
  volatile unsigned int WRPR;
} FLASH_TypeDef;

typedef struct
{
  volatile unsigned short int SR;
  unsigned short int  RESERVED0;
  volatile unsigned short int DR;
  unsigned short int  RESERVED1;
  volatile unsigned short int BRR;
  unsigned short int  RESERVED2;
  volatile unsigned short int CR1;
  unsigned short int  RESERVED3;
  volatile unsigned short int CR2;
  unsigned short int  RESERVED4;
  volatile unsigned short int CR3;
  unsigned short int  RESERVED5;
  volatile unsigned short int GTPR;
  unsigned short int  RESERVED6;
} USART_TypeDef;

#define RCC                 ((RCC_TypeDef *) RCC_BASE)

#define  RCC_CFGR_HPRE_DIV1                  ((unsigned int)0x00000000)        /*!< SYSCLK not divided */
#define  RCC_CFGR_HPRE_DIV2                  ((unsigned int)0x00000080)        /*!< SYSCLK divided by 2 */

#define  RCC_CFGR_PPRE2_DIV1                 ((unsigned int)0x00000000)        /*!< HCLK not divided */
#define  RCC_CFGR_PPRE2_DIV2                 ((unsigned int)0x00002000)        /*!< HCLK divided by 2 */

#define  RCC_CFGR_PPRE1_DIV1                 ((unsigned int)0x00000000)        /*!< HCLK not divided */
#define  RCC_CFGR_PPRE1_DIV2                 ((unsigned int)0x00000400)        /*!< HCLK divided by 2 */

#define  RCC_CFGR_PLLSRC                     ((unsigned int)0x00010000)        /*!< PLL entry clock source */

#define  RCC_CFGR_PLLXTPRE                   ((unsigned int)0x00020000)        /*!< HSE divider for PLL entry */

/*!< PLLMUL configuration */
#define  RCC_CFGR_PLLMULL                    ((unsigned int)0x003C0000)        /*!< PLLMUL[3:0] bits (PLL multiplication factor) */
#define  RCC_CFGR_PLLMULL_0                  ((unsigned int)0x00040000)        /*!< Bit 0 */
#define  RCC_CFGR_PLLMULL_1                  ((unsigned int)0x00080000)        /*!< Bit 1 */
#define  RCC_CFGR_PLLMULL_2                  ((unsigned int)0x00100000)        /*!< Bit 2 */
#define  RCC_CFGR_PLLMULL_3                  ((unsigned int)0x00200000)        /*!< Bit 3 */

 #define  RCC_CFGR_PLLSRC_HSI_Div2           ((unsigned int)0x00000000)        /*!< HSI clock divided by 2 selected as PLL entry clock source */
 #define  RCC_CFGR_PLLSRC_HSE                ((unsigned int)0x00010000)        /*!< HSE clock selected as PLL entry clock source */

 #define  RCC_CFGR_PLLXTPRE_HSE              ((unsigned int)0x00000000)        /*!< HSE clock not divided for PLL entry */
 #define  RCC_CFGR_PLLXTPRE_HSE_Div2         ((unsigned int)0x00020000)        /*!< HSE clock divided by 2 for PLL entry */

 #define  RCC_CFGR_PLLMULL2                  ((unsigned int)0x00000000)        /*!< PLL input clock*2 */
 #define  RCC_CFGR_PLLMULL3                  ((unsigned int)0x00040000)        /*!< PLL input clock*3 */
 #define  RCC_CFGR_PLLMULL4                  ((unsigned int)0x00080000)        /*!< PLL input clock*4 */
 #define  RCC_CFGR_PLLMULL5                  ((unsigned int)0x000C0000)        /*!< PLL input clock*5 */
 #define  RCC_CFGR_PLLMULL6                  ((unsigned int)0x00100000)        /*!< PLL input clock*6 */
 #define  RCC_CFGR_PLLMULL7                  ((unsigned int)0x00140000)        /*!< PLL input clock*7 */
 #define  RCC_CFGR_PLLMULL8                  ((unsigned int)0x00180000)        /*!< PLL input clock*8 */
 #define  RCC_CFGR_PLLMULL9                  ((unsigned int)0x001C0000)        /*!< PLL input clock*9 */
 #define  RCC_CFGR_PLLMULL10                 ((unsigned int)0x00200000)        /*!< PLL input clock10 */
 #define  RCC_CFGR_PLLMULL11                 ((unsigned int)0x00240000)        /*!< PLL input clock*11 */
 #define  RCC_CFGR_PLLMULL12                 ((unsigned int)0x00280000)        /*!< PLL input clock*12 */
 #define  RCC_CFGR_PLLMULL13                 ((unsigned int)0x002C0000)        /*!< PLL input clock*13 */
 #define  RCC_CFGR_PLLMULL14                 ((unsigned int)0x00300000)        /*!< PLL input clock*14 */
 #define  RCC_CFGR_PLLMULL15                 ((unsigned int)0x00340000)        /*!< PLL input clock*15 */
 #define  RCC_CFGR_PLLMULL16                 ((unsigned int)0x00380000)        /*!< PLL input clock*16 */

/********************  Bit definition for RCC_CR register  ********************/
#define  RCC_CR_HSION                        ((unsigned int)0x00000001)        /*!< Internal High Speed clock enable */
#define  RCC_CR_HSIRDY                       ((unsigned int)0x00000002)        /*!< Internal High Speed clock ready flag */
#define  RCC_CR_HSITRIM                      ((unsigned int)0x000000F8)        /*!< Internal High Speed clock trimming */
#define  RCC_CR_HSICAL                       ((unsigned int)0x0000FF00)        /*!< Internal High Speed clock Calibration */
#define  RCC_CR_HSEON                        ((unsigned int)0x00010000)        /*!< External High Speed clock enable */
#define  RCC_CR_HSERDY                       ((unsigned int)0x00020000)        /*!< External High Speed clock ready flag */
#define  RCC_CR_HSEBYP                       ((unsigned int)0x00040000)        /*!< External High Speed clock Bypass */
#define  RCC_CR_CSSON                        ((unsigned int)0x00080000)        /*!< Clock Security System enable */
#define  RCC_CR_PLLON                        ((unsigned int)0x01000000)        /*!< PLL enable */
#define  RCC_CR_PLLRDY                       ((unsigned int)0x02000000)        /*!< PLL clock ready flag */

/*******************  Bit definition for RCC_CFGR register  *******************/
/*!< SW configuration */
#define  RCC_CFGR_SW                         ((unsigned int)0x00000003)        /*!< SW[1:0] bits (System clock Switch) */
#define  RCC_CFGR_SW_0                       ((unsigned int)0x00000001)        /*!< Bit 0 */
#define  RCC_CFGR_SW_1                       ((unsigned int)0x00000002)        /*!< Bit 1 */

#define  RCC_CFGR_SW_HSI                     ((unsigned int)0x00000000)        /*!< HSI selected as system clock */
#define  RCC_CFGR_SW_HSE                     ((unsigned int)0x00000001)        /*!< HSE selected as system clock */
#define  RCC_CFGR_SW_PLL                     ((unsigned int)0x00000002)        /*!< PLL selected as system clock */

/*!< SWS configuration */
#define  RCC_CFGR_SWS                        ((unsigned int)0x0000000C)        /*!< SWS[1:0] bits (System Clock Switch Status) */
#define  RCC_CFGR_SWS_0                      ((unsigned int)0x00000004)        /*!< Bit 0 */
#define  RCC_CFGR_SWS_1                      ((unsigned int)0x00000008)        /*!< Bit 1 */

#define  RCC_CFGR_SWS_HSI                    ((unsigned int)0x00000000)        /*!< HSI oscillator used as system clock */
#define  RCC_CFGR_SWS_HSE                    ((unsigned int)0x00000004)        /*!< HSE oscillator used as system clock */
#define  RCC_CFGR_SWS_PLL                    ((unsigned int)0x00000008)        /*!< PLL used as system clock */

/*******************  Bit definition for FLASH_ACR register  ******************/
#define  FLASH_ACR_LATENCY                   ((unsigned char)0x07)               /*!< LATENCY[2:0] bits (Latency) */
#define  FLASH_ACR_LATENCY_0                 ((unsigned char)0x00)               /*!< Bit 0 */
#define  FLASH_ACR_LATENCY_1                 ((unsigned char)0x01)               /*!< Bit 0 */
#define  FLASH_ACR_LATENCY_2                 ((unsigned char)0x02)               /*!< Bit 1 */

#define  FLASH_ACR_HLFCYA                    ((unsigned char)0x08)               /*!< Flash Half Cycle Access Enable */
#define  FLASH_ACR_PRFTBE                    ((unsigned char)0x10)               /*!< Prefetch Buffer Enable */
#define  FLASH_ACR_PRFTBS                    ((unsigned char)0x20)               /*!< Prefetch Buffer Status */

/******************  Bit definition for RCC_APB2ENR register  *****************/
#define  RCC_APB2ENR_AFIOEN                  ((unsigned int)0x00000001)         /*!< Alternate Function I/O clock enable */
#define  RCC_APB2ENR_IOPAEN                  ((unsigned int)0x00000004)         /*!< I/O port A clock enable */
#define  RCC_APB2ENR_IOPBEN                  ((unsigned int)0x00000008)         /*!< I/O port B clock enable */
#define  RCC_APB2ENR_IOPCEN                  ((unsigned int)0x00000010)         /*!< I/O port C clock enable */
#define  RCC_APB2ENR_IOPDEN                  ((unsigned int)0x00000020)         /*!< I/O port D clock enable */
#define  RCC_APB2ENR_ADC1EN                  ((unsigned int)0x00000200)         /*!< ADC 1 interface clock enable */

#define  RCC_APB2ENR_TIM1EN                  ((unsigned int)0x00000800)         /*!< TIM1 Timer clock enable */
#define  RCC_APB2ENR_SPI1EN                  ((unsigned int)0x00001000)         /*!< SPI 1 clock enable */
#define  RCC_APB2ENR_USART1EN                ((unsigned int)0x00004000)         /*!< USART1 clock enable */

/*******************  Bit definition for GPIO_CRH register  *******************/
#define  GPIO_CRH_MODE                       ((unsigned int)0x33333333)        /*!< Port x mode bits */

#define  GPIO_CRH_MODE8                      ((unsigned int)0x00000003)        /*!< MODE8[1:0] bits (Port x mode bits, pin 8) */
#define  GPIO_CRH_MODE8_0                    ((unsigned int)0x00000001)        /*!< Bit 0 */
#define  GPIO_CRH_MODE8_1                    ((unsigned int)0x00000002)        /*!< Bit 1 */

#define  GPIO_CRH_MODE9                      ((unsigned int)0x00000030)        /*!< MODE9[1:0] bits (Port x mode bits, pin 9) */
#define  GPIO_CRH_MODE9_0                    ((unsigned int)0x00000010)        /*!< Bit 0 */
#define  GPIO_CRH_MODE9_1                    ((unsigned int)0x00000020)        /*!< Bit 1 */

#define  GPIO_CRH_MODE10                     ((unsigned int)0x00000300)        /*!< MODE10[1:0] bits (Port x mode bits, pin 10) */
#define  GPIO_CRH_MODE10_0                   ((unsigned int)0x00000100)        /*!< Bit 0 */
#define  GPIO_CRH_MODE10_1                   ((unsigned int)0x00000200)        /*!< Bit 1 */

#define  GPIO_CRH_MODE11                     ((unsigned int)0x00003000)        /*!< MODE11[1:0] bits (Port x mode bits, pin 11) */
#define  GPIO_CRH_MODE11_0                   ((unsigned int)0x00001000)        /*!< Bit 0 */
#define  GPIO_CRH_MODE11_1                   ((unsigned int)0x00002000)        /*!< Bit 1 */

#define  GPIO_CRH_MODE12                     ((unsigned int)0x00030000)        /*!< MODE12[1:0] bits (Port x mode bits, pin 12) */
#define  GPIO_CRH_MODE12_0                   ((unsigned int)0x00010000)        /*!< Bit 0 */
#define  GPIO_CRH_MODE12_1                   ((unsigned int)0x00020000)        /*!< Bit 1 */

#define  GPIO_CRH_MODE13                     ((unsigned int)0x00300000)        /*!< MODE13[1:0] bits (Port x mode bits, pin 13) */
#define  GPIO_CRH_MODE13_0                   ((unsigned int)0x00100000)        /*!< Bit 0 */
#define  GPIO_CRH_MODE13_1                   ((unsigned int)0x00200000)        /*!< Bit 1 */

#define  GPIO_CRH_MODE14                     ((unsigned int)0x03000000)        /*!< MODE14[1:0] bits (Port x mode bits, pin 14) */
#define  GPIO_CRH_MODE14_0                   ((unsigned int)0x01000000)        /*!< Bit 0 */
#define  GPIO_CRH_MODE14_1                   ((unsigned int)0x02000000)        /*!< Bit 1 */

#define  GPIO_CRH_MODE15                     ((unsigned int)0x30000000)        /*!< MODE15[1:0] bits (Port x mode bits, pin 15) */
#define  GPIO_CRH_MODE15_0                   ((unsigned int)0x10000000)        /*!< Bit 0 */
#define  GPIO_CRH_MODE15_1                   ((unsigned int)0x20000000)        /*!< Bit 1 */

#define  GPIO_CRH_CNF                        ((unsigned int)0xCCCCCCCC)        /*!< Port x configuration bits */

#define  GPIO_CRH_CNF8                       ((unsigned int)0x0000000C)        /*!< CNF8[1:0] bits (Port x configuration bits, pin 8) */
#define  GPIO_CRH_CNF8_0                     ((unsigned int)0x00000004)        /*!< Bit 0 */
#define  GPIO_CRH_CNF8_1                     ((unsigned int)0x00000008)        /*!< Bit 1 */

#define  GPIO_CRH_CNF9                       ((unsigned int)0x000000C0)        /*!< CNF9[1:0] bits (Port x configuration bits, pin 9) */
#define  GPIO_CRH_CNF9_0                     ((unsigned int)0x00000040)        /*!< Bit 0 */
#define  GPIO_CRH_CNF9_1                     ((unsigned int)0x00000080)        /*!< Bit 1 */

#define  GPIO_CRH_CNF10                      ((unsigned int)0x00000C00)        /*!< CNF10[1:0] bits (Port x configuration bits, pin 10) */
#define  GPIO_CRH_CNF10_0                    ((unsigned int)0x00000400)        /*!< Bit 0 */
#define  GPIO_CRH_CNF10_1                    ((unsigned int)0x00000800)        /*!< Bit 1 */

#define  GPIO_CRH_CNF11                      ((unsigned int)0x0000C000)        /*!< CNF11[1:0] bits (Port x configuration bits, pin 11) */
#define  GPIO_CRH_CNF11_0                    ((unsigned int)0x00004000)        /*!< Bit 0 */
#define  GPIO_CRH_CNF11_1                    ((unsigned int)0x00008000)        /*!< Bit 1 */

#define  GPIO_CRH_CNF12                      ((unsigned int)0x000C0000)        /*!< CNF12[1:0] bits (Port x configuration bits, pin 12) */
#define  GPIO_CRH_CNF12_0                    ((unsigned int)0x00040000)        /*!< Bit 0 */
#define  GPIO_CRH_CNF12_1                    ((unsigned int)0x00080000)        /*!< Bit 1 */

#define  GPIO_CRH_CNF13                      ((unsigned int)0x00C00000)        /*!< CNF13[1:0] bits (Port x configuration bits, pin 13) */
#define  GPIO_CRH_CNF13_0                    ((unsigned int)0x00400000)        /*!< Bit 0 */
#define  GPIO_CRH_CNF13_1                    ((unsigned int)0x00800000)        /*!< Bit 1 */

#define  GPIO_CRH_CNF14                      ((unsigned int)0x0C000000)        /*!< CNF14[1:0] bits (Port x configuration bits, pin 14) */
#define  GPIO_CRH_CNF14_0                    ((unsigned int)0x04000000)        /*!< Bit 0 */
#define  GPIO_CRH_CNF14_1                    ((unsigned int)0x08000000)        /*!< Bit 1 */

#define  GPIO_CRH_CNF15                      ((unsigned int)0xC0000000)        /*!< CNF15[1:0] bits (Port x configuration bits, pin 15) */
#define  GPIO_CRH_CNF15_0                    ((unsigned int)0x40000000)        /*!< Bit 0 */
#define  GPIO_CRH_CNF15_1                    ((unsigned int)0x80000000)        /*!< Bit 1 */


/*******************  Bit definition for GPIO_ODR register  *******************/
#define GPIO_ODR_ODR0                        ((uint16_t)0x0001)            /*!< Port output data, bit 0 */
#define GPIO_ODR_ODR1                        ((uint16_t)0x0002)            /*!< Port output data, bit 1 */
#define GPIO_ODR_ODR2                        ((uint16_t)0x0004)            /*!< Port output data, bit 2 */
#define GPIO_ODR_ODR3                        ((uint16_t)0x0008)            /*!< Port output data, bit 3 */
#define GPIO_ODR_ODR4                        ((uint16_t)0x0010)            /*!< Port output data, bit 4 */
#define GPIO_ODR_ODR5                        ((uint16_t)0x0020)            /*!< Port output data, bit 5 */
#define GPIO_ODR_ODR6                        ((uint16_t)0x0040)            /*!< Port output data, bit 6 */
#define GPIO_ODR_ODR7                        ((uint16_t)0x0080)            /*!< Port output data, bit 7 */
#define GPIO_ODR_ODR8                        ((uint16_t)0x0100)            /*!< Port output data, bit 8 */
#define GPIO_ODR_ODR9                        ((uint16_t)0x0200)            /*!< Port output data, bit 9 */
#define GPIO_ODR_ODR10                       ((uint16_t)0x0400)            /*!< Port output data, bit 10 */
#define GPIO_ODR_ODR11                       ((uint16_t)0x0800)            /*!< Port output data, bit 11 */
#define GPIO_ODR_ODR12                       ((uint16_t)0x1000)            /*!< Port output data, bit 12 */
#define GPIO_ODR_ODR13                       ((uint16_t)0x2000)            /*!< Port output data, bit 13 */
#define GPIO_ODR_ODR14                       ((uint16_t)0x4000)            /*!< Port output data, bit 14 */
#define GPIO_ODR_ODR15                       ((uint16_t)0x8000)            /*!< Port output data, bit 15 */

/*******************  Bit definition for USART_SR register  *******************/
#define  USART_SR_PE                         ((unsigned short int)0x0001)            /*!< Parity Error */
#define  USART_SR_FE                         ((unsigned short int)0x0002)            /*!< Framing Error */
#define  USART_SR_NE                         ((unsigned short int)0x0004)            /*!< Noise Error Flag */
#define  USART_SR_ORE                        ((unsigned short int)0x0008)            /*!< OverRun Error */
#define  USART_SR_IDLE                       ((unsigned short int)0x0010)            /*!< IDLE line detected */
#define  USART_SR_RXNE                       ((unsigned short int)0x0020)            /*!< Read Data Register Not Empty */
#define  USART_SR_TC                         ((unsigned short int)0x0040)            /*!< Transmission Complete */
#define  USART_SR_TXE                        ((unsigned short int)0x0080)            /*!< Transmit Data Register Empty */
#define  USART_SR_LBD                        ((unsigned short int)0x0100)            /*!< LIN Break Detection Flag */
#define  USART_SR_CTS                        ((unsigned short int)0x0200)            /*!< CTS Flag */

/******************  Bit definition for USART_CR1 register  *******************/
#define  USART_CR1_SBK                       ((unsigned short int)0x0001)            /*!< Send Break */
#define  USART_CR1_RWU                       ((unsigned short int)0x0002)            /*!< Receiver wakeup */
#define  USART_CR1_RE                        ((unsigned short int)0x0004)            /*!< Receiver Enable */
#define  USART_CR1_TE                        ((unsigned short int)0x0008)            /*!< Transmitter Enable */
#define  USART_CR1_IDLEIE                    ((unsigned short int)0x0010)            /*!< IDLE Interrupt Enable */
#define  USART_CR1_RXNEIE                    ((unsigned short int)0x0020)            /*!< RXNE Interrupt Enable */
#define  USART_CR1_TCIE                      ((unsigned short int)0x0040)            /*!< Transmission Complete Interrupt Enable */
#define  USART_CR1_TXEIE                     ((unsigned short int)0x0080)            /*!< PE Interrupt Enable */
#define  USART_CR1_PEIE                      ((unsigned short int)0x0100)            /*!< PE Interrupt Enable */
#define  USART_CR1_PS                        ((unsigned short int)0x0200)            /*!< Parity Selection */
#define  USART_CR1_PCE                       ((unsigned short int)0x0400)            /*!< Parity Control Enable */
#define  USART_CR1_WAKE                      ((unsigned short int)0x0800)            /*!< Wakeup method */
#define  USART_CR1_M                         ((unsigned short int)0x1000)            /*!< Word length */
#define  USART_CR1_UE                        ((unsigned short int)0x2000)            /*!< USART Enable */
#define  USART_CR1_OVER8                     ((unsigned short int)0x8000)            /*!< USART Oversampling 8-bits */



#endif /* STM32F10_REGISTERS_H */