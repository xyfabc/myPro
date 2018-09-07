#include <linux/jiffies.h>

#include <asm/jzsoc.h>


#define VIDIOC_S_EXPOSURE 			300
#define VIDIOC_S_BRIGHT				301
#define VIDIOC_S_CONTRAS			302
#define VIDIOC_S_CONTRAS_CENTER	    303
#define VIDIOC_RESET_SENOSER		304

/*ZYL,2011.11.08*/
#define VIDIOC_READ_ALLSENOSER		305
#define VIDIOC_INIT_ALLSENOSER		306
#define VIDIOC_G_EXPOSURE		307


#define I2C_READ	1
#define I2C_WRITE	0

//#define TIMEOUT         1000

#define golf_cim_clear_state() \
	do{\
		REG_CIM_STATE = 0x0;\
	}while(0)
#define golf_cim_reset_rxfifo()\
	do{\
		REG_CIM_CTRL |= CIM_CTRL_DMA_EN;\
		REG_CIM_CTRL |= CIM_CTRL_RXF_RST;\
		REG_CIM_CTRL &= ~CIM_CTRL_DMA_EN;\
	}while(0)

#define golf_cim_unreset_rxfifo()\
	do{\
		REG_CIM_CTRL |= CIM_CTRL_DMA_EN;\
		REG_CIM_CTRL &= ~CIM_CTRL_RXF_RST;\
		REG_CIM_CTRL &= ~CIM_CTRL_DMA_EN;\
	}while(0)
	
#define golf_cim_enable_dma()\
	do{\
		REG_CIM_CTRL |= CIM_CTRL_DMA_EN;\
		REG_CIM_CTRL &= ~CIM_CTRL_RXF_RST;\
		REG_CIM_CTRL |= CIM_CTRL_DMA_EN;\
	}while(0)

#define PIN_CIM_GPF5	GPE(9)	// (32 * 5 + 5)	-> GPE(9) YJT, 20120602

#define golf_pin_cim_rgb()\
do{\
	__gpio_set_pin(PIN_CIM_GPF5);  /* near relay */ \
}while(0)

#define golf_pin_cim_yuv()\
do{\
	__gpio_clear_pin(PIN_CIM_GPF5);  /* far to relay */ \
}while(0)


#define CIM_SELECT_CTR_GPIO_INIT()\
do{\
	__gpio_as_output(PIN_CIM_GPF5);\
	__gpio_clear_pin(PIN_CIM_GPF5);  \
}while(0)


/*
 *	ioctl commands
 */
#define IOCTL_SET_ADDR            0 /* set i2c address */
#define IOCTL_SET_CLK             1 /* set i2c clock */
#define IOCTL_WRITE_REG           2 /* write sensor register */
#define IOCTL_READ_REG            3 /* read sensor register */
#define IOCTL_SET_SEL             4 /* set sensor select */
/*
 *	i2c related
 */
//static unsigned int i2c_addr = 0x42;
//static unsigned int i2c_clk = 100*1024;

struct video_device *jz_sensor;


#ifdef i2c_read_reg
#endif

struct i2c_regvals {
  unsigned char reg;
  unsigned char val;
};

#if 1 //7725
static struct i2c_regvals v7725YUVreg[] =// 500 10 58   old   540 10 48    230 1000  40  40
{

	{0x12,0x80}, 
	{0x12,0x00}, 
	{0x13,0xf0},//zyl,2012.3.30
	{0x00,0x00}, 
	{0x3D,0x03}, 
	{0x0c,0x10}, // 0x10 => 0x11
	{0x15,0x00}, 
	{0x16,0x00}, 
	{0x17,0x22}, 
	{0x18,0xa4}, 
	{0x19,0x07}, 
	{0x1a,0xf0}, 
	{0x1b,0x35},
	{0x32,0x00}, 
	{0x3f,0x1f}, 
	{0x29,0xa0}, 
	{0x2c,0xf2}, 
	{0x2a,0x00}, 
	{0x11,0x03}, 
	{0x42,0x7f}, 
	{0x4d,0x09}, 
	{0x63,0xe0},
	{0x64,0xff}, 
	{0x55,0xf4}, 
	{0x65,0x20}, 
	{0x66,0x00}, // 0x00 => 0x20
	{0x67,0x48}, 
	{0x13,0xf0}, 
	{0x0d,0x51}, 
	{0x0f,0xc5}, 
	{0x14,0x11},
	{0x22,0x52}, 
	{0x23,0x07}, 
	{0x24,0x48}, 
	{0x25,0x3c},
	{0x26,0x92}, 
	{0x2b,0x00}, 
	{0x6b,0xaa}, 
	{0x13,0xF0},
	{0x90,0x05},
	{0x91,0x01}, 
	{0x92,0x03}, 
	{0x93,0x00}, 
	{0x94,0x37}, 
	{0x95,0x2d}, 
	{0x96,0x09}, 
	{0x97,0x17}, 
	{0x98,0x29}, 
	{0x99,0x40}, 
	{0x9a,0x9e}, 
	{0x9b,0x20},
	{0x9e,0x81}, 
	{0xa6,0x04}, 
	{0x7e,0x04}, 
	{0x7f,0x08}, 
	{0x80,0x10}, 
	{0x81,0x20}, 
	{0x82,0x28}, 
	{0x83,0x30}, 
	{0x84,0x38}, 
	{0x85,0x40}, 
	{0x86,0x48}, 
	{0x87,0x50}, 
	{0x88,0x60}, 
	{0x89,0x70}, 
	{0x8a,0x90}, 
	{0x8b,0xb0}, 
	{0x8c,0xd0},
	{0x8d,0x40}, 
	{0x0e,0x65}, 
	{0x46,0x01}, 
	{0x47,0x00}, 
	{0x48,0x00}, 
	{0x49,0x04}, 
	{0x4a,0x08}, 
	{0x4b,0x04}, 
	{0x4c,0x06}, 
	{0x22,0x4c}, 
	{0x23,0x07}, 
	{0x33,0x66},
	{0x34,0x00}, 
	{0x9c,0x20}, 
	{0x08,0x01},
	{0x10,0x60},


};	  

#if 0
//���Զ��ع�
static struct i2c_regvals reg_init_yuv_test[] =
{0x12,0x80, 
0x12,0x00, 
0x00,0x00, 
0x3D,0x03, 
0x0c,0x10,
0x15,0x00, 
0x16,0x00, 
0x17,0x22, 
0x18,0xa4, 
0x19,0x07, 
0x1a,0xf0, 

0x1b,0x35,

0x32,0x00, 
0x3f,0x1f, 
0x29,0xa0, 
0x2c,0xf2, 
0x2a,0x00, 
0x11,0x03, 
0x42,0x7f, 
0x4d,0x09, 
0x63,0xe0,
0x64,0xff, 
0x55,0xf4, 
0x65,0x20, 
0x66,0x00,
0x67,0x44, 
0x13,0xff, 
0x0d,0x51, 
0x0f,0xc5, 
0x14,0x41,
0x22,0x52, 
0x23,0x07, 
0x24,0x40,
0x25,0x3c,
0x26,0x92, 
0x2b,0x00, 
0x6b,0xaa, 
0x13,0xFf,
0x90,0x00,
0x91,0x01, 
0x92,0x03, 
0x93,0x00, 
0x94,0x37, 
0x95,0x2d, 
0x96,0x09, 
0x97,0x17, 
0x98,0x29, 
0x99,0x40, 
0x9a,0x9e, 
0x9b,0x20,
0xab,0x06,

0x9e,0x81, 
0xa6,0x26, 
0x60,0x80,
0x61,0x80,
0xa7,0x80,
0xa8,0x80,

0x7e,0x04, 
0x7f,0x08, 
0x80,0x10, 
0x81,0x20, 
0x82,0x28, 
0x83,0x30, 
0x84,0x38, 
0x85,0x40, 
0x86,0x48, 
0x87,0x50, 
0x88,0x60, 
0x89,0x70, 
0x8a,0x90, 
0x8b,0xb0, 
0x8c,0xd0,
0x8d,0x40, 

0x0e,0x65, 
0x46,0x01, 
0x47,0x00, 
0x48,0x00, 
0x49,0x04, 
0x4a,0x08, 
0x4b,0x04, 
0x4c,0x06, 
0x22,0x4c, 
0x23,0x07, 
0x33,0x66,
0x34,0x00, 
0x9c,0x20, 
0x08,0x00,
0x10,0xF0
	};	  


static struct i2c_regvals v7670reg_320x240[] =   //
{ 0x12,0x80, 
0x3d, 0x03,
0x12, 0x06,
0x17, 0x22,
0x18, 0xa4,
0x19, 0x07,
0x1a, 0xf0,
0x32, 0x00,
0x29, 0x50,
0x2c, 0x78,
0x2a, 0x00,
0x11, 0x03,// ;00/01/03/07 for 60/30/15/7.5fps
//
0x42, 0x7f,
0x4d, 0x09,
0x63, 0xe0,
0x64, 0xff,
0x65, 0x2f,
0x66, 0x00,
0x67, 0x48,
//
0x13, 0xf0,
0x0d, 0x41, //;51/61/71 for different AEC/AGC window
0x0f, 0xc5,
0x14, 0x11,
0x22, 0x3f,// ;ff/7f/3f/1f for 60/30/15/7.5fps
0x23, 0x07,// ;01/03/07/0f for 60/30/15/7.5fps
0x24, 0x40,
0x25, 0x30,
0x26, 0xa1,
0x2b, 0x00,// ;00/9e for 60/50Hz
0x6b, 0xaa,
0x13, 0xff,
//,
0x90, 0x05,
0x91, 0x01,
0x92, 0x03,
0x93, 0x00,
0x94, 0xb0,
0x95, 0x9d,
0x96, 0x13,
0x97, 0x16,
0x98, 0x7b,
0x99, 0x91,
0x9a, 0x1e,
0x9b, 0x08,
0x9c, 0x20,
0x9e, 0x81,
0xa6, 0x04,
//
0x7e, 0x0c,
0x7f, 0x16,
0x80, 0x2a,
0x81, 0x4e,
0x82, 0x61,
0x83, 0x6f,
0x84, 0x7b,
0x85, 0x86,
0x86, 0x8e,
0x87, 0x97,
0x88, 0xa4,
0x89, 0xaf,
0x8a, 0xc5,
0x8b, 0xd7,
0x8c, 0xe8,
0x8d, 0x20,
};

#endif

// new move to here
static struct i2c_regvals v7725reg[] = //_640x480
{
	{0x12,0x80}, 
	{0x12,0x06}, 
	{0x00,0x00}, 
	{0x3D,0x03}, 
	{0x0c,0x10},  // 0x10 -> 0x11 // 	TEST COLOR BAR
	{0x15,0x00}, 
	{0x16,0x00}, 
	{0x17,0x22}, 
	{0x18,0xa4}, 
	{0x19,0x07}, 
	{0x1a,0xf0}, 
	{0x1b,0x35},
	{0x32,0x00}, 
	{0x3f,0x1f}, 
	{0x29,0xa0}, /* 50 */ //0x29,0x50, //0x29,0xa0, 
	{0x2c,0xf2}, /* 78 */ //0x2c,0x78, //0x2c,0xf2, 
	{0x2a,0x00}, //
	{0x11,0x03}, //
	{0x42,0x7f}, 
	{0x4d,0x09}, 
	{0x63,0xe0},
	{0x64,0xff}, 
	{0x55,0xf4}, 
	{0x65,0x20}, /* 2f */
	{0x66,0x00},  // 0x00 -> 0x20 // 	TEST COLOR BAR
	{0x67,0x48}, 
	{0x13,0xf0}, /* 0xff */
	{0x0d,0x51}, //
	{0x0f,0xc5}, 
	{0x14,0x11},
	{0x22,0x47}, //0x22,0x59, //0x22,0x58, //0x22,0x4c, //0x22,0x52, 
	{0x23,0x08}, //0x23,0x06, //0x23,0x07, 
	{0x24,0x48}, 
	{0x25,0x3c},
	{0x26,0x92}, 
	{0x2b,0x00}, //
	{0x6b,0xaa}, 
	{0x13,0xFf},
	{0x90,0x05},
	{0x91,0x01}, 
	{0x92,0x03}, 
	{0x93,0x00}, 
	{0x94,0x37}, 
	{0x95,0x2d}, 
	{0x96,0x09}, 
	{0x97,0x17}, 
	{0x98,0x29}, 
	{0x99,0x40}, 
	{0x9a,0x9e}, 
	{0x9b,0x30},//zyl,2012.4.1  0x9b,0x50,  // bright ����̫�ߣ�����ͼ�񷢻ҷ���
	{0x9e,0x81}, 
	{0xa6,0x04}, 
	{0x7e,0x04}, 
	{0x7f,0x08}, 
	{0x80,0x10}, 
	{0x81,0x20}, 
	{0x82,0x28}, 
	{0x83,0x30}, 
	{0x84,0x38}, 
	{0x85,0x40}, 
	{0x86,0x48}, 
	{0x87,0x50}, 
	{0x88,0x60}, 
	{0x89,0x70}, 
	{0x8a,0x90}, 
	{0x8b,0xb0}, 
	{0x8c,0xd0},
	{0x8d,0x40}, 
	{0x0e,0x65}, 
	{0x46,0x01}, 
	{0x47,0x00}, 
	{0x48,0x00}, 
	{0x49,0x04}, 
	{0x4a,0x08}, 
	{0x4b,0x04}, 
	{0x4c,0x06}, 
	{0x22,0x8f}, //zyl,2012.4.1      0x22,0x4c, 
	{0x23,0x03}, //zyl,2012.4.1      0x23,0x07, 
	{0x33,0x66},//zyl,2012.4.1       0x33,0x86,
	{0x34,0x00}, //
	{0x9c,0x30}, 
	{0x08,0x01},  // expose
	{0x10,0x60},  // expose
};

#if 0
static struct i2c_regvals v7670regold[] = //_640x480
{
0x12,0x80, 
0x12,0x06, 
0x00,0x00, 
0x3D,0x03, 
0x0c,0x10,
0x15,0x00, 
0x16,0x00, 
0x17,0x22, 
0x18,0xa4,  // h640
//0x18,0x5e,  // h320
0x19,0x07, 
0x1a,0xf0,  // v480
//0x1a,0x78/2,  // v240

0x1b,0x35,

0x32,0x00, 
0x3f,0x1f, 
0x29,0x50,//0x29,0x50, //0x29,0xa0, 
0x2c,0x78,//0x2c,0x78, //0x2c,0xf2, 
0x2a,0x00, //
0x11,0x03, //
0x42,0x7f, 
0x4d,0x09, 
0x63,0xe0,
0x64,0xff, 
0x55,0xf4, 
0x65,0x2f, 
0x66,0x00,
0x67,0x48, 
0x13,0xff, 
0x0d,0x51, //
0x0f,0xc5, 
0x14,0x11,
0x22,0x47, //0x22,0x59, //0x22,0x58, //0x22,0x4c, //0x22,0x52, 
0x23,0x08, //0x23,0x06, //0x23,0x07, 
0x24,0x48, 
0x25,0x3c,
0x26,0x92, 
0x2b,0x00, //
0x6b,0xaa, 
0x13,0xFf,
0x90,0x05,
0x91,0x01, 
0x92,0x03, 
0x93,0x00, 
0x94,0x37, 
0x95,0x2d, 
0x96,0x09, 
0x97,0x17, 
0x98,0x29, 
0x99,0x40, 
0x9a,0x9e, 
0x9b,0x50,  // bright golf 0218

0x9e,0x81, 
0xa6,0x04, 

0x7e,0x04, 
0x7f,0x08, 
0x80,0x10, 
0x81,0x20, 
0x82,0x28, 
0x83,0x30, 
0x84,0x38, 
0x85,0x40, 
0x86,0x48, 
0x87,0x50, 
0x88,0x60, 
0x89,0x70, 
0x8a,0x90, 
0x8b,0xb0, 
0x8c,0xd0,
0x8d,0x40, 

0x0e,0x65, 
0x46,0x01, 
0x47,0x00, 
0x48,0x00, 
0x49,0x04, 
0x4a,0x08, 
0x4b,0x04, 
0x4c,0x06, 
0x22,0x4c, 
0x23,0x07, 
0x33,0x86, //0x33,0x66, //0x33,0x73, //0x33,0x66, // golf
0x34,0x00, //
0x9c,0x30, 
0x08,0x01,  // expose
0x10,0x60,  // expose
	0xff,0xff
};


static struct i2c_regvals v7670reg1[] = //_640x480
{ 0x12,0x80, 
0x12,0x06, 
0x00,0x00, 
0x3D,0x03, 
0x0c,0x10,
0x15,0x00, 
0x16,0x00, 
0x17,0x22, 
0x18,0xa4, 
0x19,0x07, 
0x1a,0xf0, 

0x1b,0x35,

0x32,0x00, 
0x3f,0x1f, 
//0x29,0xa0, 
//0x2c,0xf2, 
0x29, 0x50,
0x2c, 0x78,
0x2a,0x00, 
0x11,0x03, 
0x42,0x7f, 
0x4d,0x09, 
0x63,0xe0,
0x64,0xff, 
0x55,0xf4, 
0x65, 0x2f, //dsp0x65,0x20, //0x65, 0x2f,
0x66,0x00,
0x67,0x48, 
0x13,0xff,// 0x13,0xf2,
0x0d,0x51, 
0x0f,0xc5, 
0x14,0x11,
/*0x22,0x52,*/
0x22,0x4C,
0x23,0x07, 
0x24,0x48, 
0x25,0x3c,
0x26,0x92, 
0x2b,0x00, 
0x6b,0xaa, 
0x13,0xFF,//0x13,0xF2,
0x90,0x05,
0x91,0x01, 
0x92,0x03, 
0x93,0x00, 
0x94,0x37, 
0x95,0x2d, 
0x96,0x09, 
0x97,0x17, 
0x98,0x29, 
0x99,0x40, 
0x9a,0x9e, 
0x9b,0x30,

0x9e,0x81, 
0xa6,0x04, 


0x7e,0x04, 
0x7f,0x08, 
0x80,0x10, 
0x81,0x20, 
0x82,0x28, 
0x83,0x30, 
0x84,0x38, 
0x85,0x40, 
0x86,0x48, 
0x87,0x50, 
0x88,0x60, 
0x89,0x70, 
0x8a,0x90, 
0x8b,0xb0, 
0x8c,0xd0,
0x8d,0x40, 

0x0e,0x65, 
0x46,0x01, 
0x47,0x00, 
0x48,0x00, 
0x49,0x04, 
0x4a,0x08, 
0x4b,0x04, 
0x4c,0x06, 
0x22,0x4c, 
0x23,0x07, 
0x33,0x66,
0x34,0x00, 
0x9c,0x30, 
0x08,0x01,
0x10,0x60,
	0xff,0xff
};	  
#endif

#endif

#if 0
static int  jz_sensor_init(struct i2c_client *client, int st_num)
{
	int i;
	int count;
	printk("read rgb reg!!!!!!!!!!!!!!!!!!!\n" );

	__gpio_as_output1(GPE(9));
	count = sizeof(v7725reg) / sizeof(struct i2c_regvals);
	unsigned char testi2c;
	unsigned char testi2c_old;

	for(i = st_num; i < count; i++)
	{

		do
			{

			}while(0);

	}

	__gpio_clear_pin(GPE(9));
	count =sizeof(v7725YUVreg) / sizeof(struct i2c_regvals);
	for(i = st_num; i < count; i++)
	{
		do
		{





		}while(0);

	}
	printk("Ingenic CMOS camera sensor driver registered\n");
	__gpio_set_pin(GPE(9));
	printk("read rgb reg end !!!!!!!!!!!!!!!!!!!\n" );


	return 0;
}
#endif


