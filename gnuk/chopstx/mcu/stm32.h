#define PERIPH_BASE	0x40000000
#define APB1PERIPH_BASE PERIPH_BASE
#define APB2PERIPH_BASE	(PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE	(PERIPH_BASE + 0x20000)
#define AHB2PERIPH_BASE	(PERIPH_BASE + 0x08000000)

struct RCC {
  volatile uint32_t CR;
  volatile uint32_t CFGR;
  volatile uint32_t CIR;
  volatile uint32_t APB2RSTR;
  volatile uint32_t APB1RSTR;
  volatile uint32_t AHBENR;
  volatile uint32_t APB2ENR;
  volatile uint32_t APB1ENR;
  volatile uint32_t BDCR;
  volatile uint32_t CSR;
#if defined(MCU_STM32F0)
  volatile uint32_t AHBRSTR;
  volatile uint32_t CFGR2;
  volatile uint32_t CFGR3;
  volatile uint32_t CR2;
#endif
};

#define RCC_BASE		(AHBPERIPH_BASE + 0x1000)
static struct RCC *const RCC = (struct RCC *)RCC_BASE;

#define RCC_APB1ENR_USBEN	0x00800000
#define RCC_APB1RSTR_USBRST	0x00800000

#define RCC_CR_HSION		0x00000001
#define RCC_CR_HSIRDY		0x00000002
#define RCC_CR_HSITRIM		0x000000F8
#define RCC_CR_HSEON		0x00010000
#define RCC_CR_HSERDY		0x00020000
#define RCC_CR_PLLON		0x01000000
#define RCC_CR_PLLRDY		0x02000000

#define RCC_CFGR_SWS		0x0000000C
#define RCC_CFGR_SWS_HSI	0x00000000
#define RCC_CFGR_SW_HSI		(0 << 0)
#define RCC_CFGR_SW_HSE		(1 << 0)
#define RCC_CFGR_SW_PLL		(2 << 0)
#define RCC_CFGR_SW_MASK	(3 << 0)

#define RCC_AHBENR_DMA1EN       0x00000001
#define RCC_AHBENR_CRCEN        0x00000040

#if defined(MCU_STM32F0)
#define RCC_AHBRSTR_IOPARST	0x00020000
#define RCC_AHBRSTR_IOPBRST	0x00040000
#define RCC_AHBRSTR_IOPCRST	0x00080000
#define RCC_AHBRSTR_IOPDRST	0x00100000
#define RCC_AHBRSTR_IOPFRST	0x00400000

#define RCC_AHBENR_IOPAEN	0x00020000
#define RCC_AHBENR_IOPBEN	0x00040000
#define RCC_AHBENR_IOPCEN	0x00080000
#define RCC_AHBENR_IOPDEN	0x00100000
#define RCC_AHBENR_IOPFEN	0x00400000

#define RCC_APB2RSTR_SYSCFGRST	0x00000001
#define RCC_APB2ENR_SYSCFGEN	0x00000001
#else
#define RCC_APB2ENR_ADC1EN      0x00000200
#define RCC_APB2ENR_ADC2EN      0x00000400
#define RCC_APB2ENR_TIM1EN      0x00000800
#define RCC_APB1ENR_TIM2EN      0x00000001
#define RCC_APB1ENR_TIM3EN      0x00000002
#define RCC_APB1ENR_TIM4EN      0x00000004

#define RCC_APB2RSTR_ADC1RST    0x00000200
#define RCC_APB2RSTR_ADC2RST    0x00000400
#define RCC_APB2RSTR_TIM1RST    0x00000800
#define RCC_APB1RSTR_TIM2RST    0x00000001
#define RCC_APB1RSTR_TIM3RST    0x00000002
#define RCC_APB1RSTR_TIM4RST    0x00000004

#define RCC_APB2RSTR_AFIORST	0x00000001
#define RCC_APB2RSTR_IOPARST	0x00000004
#define RCC_APB2RSTR_IOPBRST	0x00000008
#define RCC_APB2RSTR_IOPCRST	0x00000010
#define RCC_APB2RSTR_IOPDRST	0x00000020
#define RCC_APB2RSTR_IOPERST	0x00000040
#define RCC_APB2RSTR_IOPFRST	0x00000080
#define RCC_APB2RSTR_IOPGRST	0x00000100

#define RCC_APB2ENR_AFIOEN	0x00000001
#define RCC_APB2ENR_IOPAEN	0x00000004
#define RCC_APB2ENR_IOPBEN	0x00000008
#define RCC_APB2ENR_IOPCEN	0x00000010
#define RCC_APB2ENR_IOPDEN	0x00000020
#define RCC_APB2ENR_IOPEEN	0x00000040
#define RCC_APB2ENR_IOPFEN	0x00000080
#define RCC_APB2ENR_IOPGEN	0x00000100
#endif


/* Clock setting values.
 * Due to historical reason, it has the prefix of STM32_.
 */
#define STM32_SW_HSI		(0 << 0)
#define STM32_SW_HSE		(1 << 0)
#define STM32_SW_PLL		(2 << 0)

#define STM32_PLLSRC_HSI	(0 << 16)
#define STM32_PLLSRC_HSE	(1 << 16)

#define STM32_PLLXTPRE_DIV1	(0 << 17)
#define STM32_PLLXTPRE_DIV2	(1 << 17)

#define STM32_HPRE_DIV1		(0 << 4)
#define STM32_HPRE_DIV8		(10 << 4)
#define STM32_HPRE_DIV16	(11 << 4)

#define STM32_PPRE1_DIV1	(0 << 8)
#define STM32_PPRE1_DIV2	(4 << 8)
#define STM32_PPRE1_DIV16	(7 << 8)

#define STM32_PPRE2_DIV1        (0 << 11)
#define STM32_PPRE2_DIV2	(4 << 11)
#define STM32_PPRE2_DIV16	(7 << 11)

#define STM32_ADCPRE_DIV4	(1 << 14)
#define STM32_ADCPRE_DIV6       (2 << 14)
#define STM32_ADCPRE_DIV8	(3 << 14)

#define STM32_USBPRE_DIV1P5     (0 << 22)
#define STM32_USBPRE_DIV2       (3 << 22) /* Not for STM32, but GD32F103 */

#define STM32_MCO_NOCLOCK	(0 << 24)

struct PWR
{
  volatile uint32_t CR;
  volatile uint32_t CSR;
};
static struct PWR *const PWR = ((struct PWR *)0x40007000);
#define PWR_CR_LPDS 0x0001	/* Low-power deepsleep  */
#define PWR_CR_PDDS 0x0002	/* Power down deepsleep */
#define PWR_CR_CWUF 0x0004	/* Clear wakeup flag    */


#if defined(MCU_STM32F0)
struct GPIO {
  volatile uint32_t MODER;
  volatile uint16_t OTYPER;
  uint16_t dummy0;
  volatile uint32_t OSPEEDR;
  volatile uint32_t PUPDR;
  volatile uint16_t IDR;
  uint16_t dummy1;
  volatile uint16_t ODR;
  uint16_t dummy2;
  volatile uint16_t BSRR;
  uint16_t dummy3;
  volatile uint32_t LCKR;
  volatile uint32_t AFR[2];
  volatile uint16_t BRR;
  uint16_t dummy4;
};

#define GPIOA_BASE	(AHB2PERIPH_BASE + 0x0000)
#define GPIOA		((struct GPIO *) GPIOA_BASE)
#define GPIOB_BASE	(AHB2PERIPH_BASE + 0x0400)
#define GPIOB		((struct GPIO *) GPIOB_BASE)
#define GPIOC_BASE	(AHB2PERIPH_BASE + 0x0800)
#define GPIOC		((struct GPIO *) GPIOC_BASE)
#define GPIOD_BASE	(AHB2PERIPH_BASE + 0x0C00)
#define GPIOD		((struct GPIO *) GPIOD_BASE)
#define GPIOF_BASE	(AHB2PERIPH_BASE + 0x1400)
#define GPIOF		((struct GPIO *) GPIOF_BASE)
#else
struct GPIO {
  volatile uint32_t CRL;
  volatile uint32_t CRH;
  volatile uint32_t IDR;
  volatile uint32_t ODR;
  volatile uint32_t BSRR;
  volatile uint32_t BRR;
  volatile uint32_t LCKR;
};

#define GPIOA_BASE	(APB2PERIPH_BASE + 0x0800)
#define GPIOA		((struct GPIO *) GPIOA_BASE)
#define GPIOB_BASE	(APB2PERIPH_BASE + 0x0C00)
#define GPIOB		((struct GPIO *) GPIOB_BASE)
#define GPIOC_BASE	(APB2PERIPH_BASE + 0x1000)
#define GPIOC		((struct GPIO *) GPIOC_BASE)
#define GPIOD_BASE	(APB2PERIPH_BASE + 0x1400)
#define GPIOD		((struct GPIO *) GPIOD_BASE)
#define GPIOE_BASE	(APB2PERIPH_BASE + 0x1800)
#define GPIOE		((struct GPIO *) GPIOE_BASE)
#endif


#if defined(MCU_STM32F0)
struct SYSCFG {
  volatile uint32_t CFGR1;
  uint32_t dummy0;
  volatile uint32_t EXTICR[4];
  volatile uint32_t CFGR2;
};
#define SYSCFG_CFGR1_MEM_MODE 0x03

#define SYSCFG_BASE	(APB1PERIPH_BASE + 0x00010000)
static struct SYSCFG *const SYSCFG = (struct SYSCFG *)SYSCFG_BASE;
#endif

struct FLASH {
  volatile uint32_t ACR;
  volatile uint32_t KEYR;
  volatile uint32_t OPTKEYR;
  volatile uint32_t SR;
  volatile uint32_t CR;
  volatile uint32_t AR;
  volatile uint32_t RESERVED;
  volatile uint32_t OBR;
  volatile uint32_t WRPR;
};

#define FLASH_R_BASE	(AHBPERIPH_BASE + 0x2000)
static struct FLASH *const FLASH = (struct FLASH *)FLASH_R_BASE;
