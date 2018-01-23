const char xdata WaveData[128] =     
{                                      
// Wave 0 
/* LenBr */ 0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x02,     0x02,     0x02,     0x02,     0x02,     0x02,     0x02,     0x02,
/* Output*/ 0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,
/* LFun  */ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 1 
/* LenBr */ 0x04,     0x04,     0x01,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x22,     0x02,     0x02,     0x02,     0x02,     0x02,     0x02,     0x02,
/* Output*/ 0x00,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,
/* LFun  */ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 2 
/* LenBr */ 0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x02,     0x02,     0x02,     0x02,     0x02,     0x02,     0x02,     0x02,
/* Output*/ 0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,
/* LFun  */ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 3 
/* LenBr */ 0x81,     0x03,     0x03,     0xB8,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x03,     0x06,     0x02,     0x03,     0x02,     0x02,     0x02,     0x02,
/* Output*/ 0x01,     0x00,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,
/* LFun  */ 0x36,     0x00,     0x00,     0x2D,     0x00,     0x00,     0x00,     0x3F,
};
const char xdata FlowStates[36] =   
{                                      
/* Wave 0 FlowStates */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* Wave 1 FlowStates */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* Wave 2 FlowStates */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* Wave 3 FlowStates */ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
const char xdata InitData[7] =                                   
{                                                                
/* Regs  */ 0xE0,0x10,0x00,0x01,0xCE,0x4E,0x00     
};
