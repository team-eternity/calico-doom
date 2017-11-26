/*
  CALICO

  Jaguar gamepad key constants; separated out of doomdef.h
*/

#ifndef JAGPAD_H__
#define JAGPAD_H__

#define JP_NUM    0x00000001
#define JP_9      0x00000002
#define JP_6      0x00000004
#define JP_3      0x00000008
#define JP_0      0x00000010
#define JP_8      0x00000020
#define JP_5      0x00000040
#define JP_2      0x00000080
#define JP_STRAFE 0x00000100 // CALICO: dedicated strafe-on input
#define JP_OPTION 0x00000200
#define JP_SLEFT  0x00000400 // CALICO: dedicated strafe left input
#define JP_SRIGHT 0x00000800 // CALICO: dedicated strafe right input
#define JP_USE    0x00001000 // CALICO: dedicated use input
#define JP_C      0x00002000
#define JP_PWEAPN 0x00004000 // CALICO: previous weapon input
#define JP_NWEAPN 0x00008000 // CALICO: next weapon input
#define JP_STAR   0x00010000
#define JP_7      0x00020000
#define JP_4      0x00040000
#define JP_1      0x00080000
#define JP_UP     0x00100000
#define JP_DOWN   0x00200000
#define JP_LEFT   0x00400000
#define JP_RIGHT  0x00800000
#define JP_ATTACK 0x01000000 // CALICO: dedicated attack input
#define JP_B      0x02000000
#define JP_SPEED  0x04000000 // CALICO: dedicated speed input

#define JP_PAUSE  0x10000000
#define JP_A      0x20000000

#define BT_RIGHT  JP_RIGHT
#define BT_LEFT   JP_LEFT
#define BT_UP     JP_UP
#define BT_DOWN   JP_DOWN
#define BT_A      JP_A
#define BT_B      JP_B
#define BT_C      JP_C
#define BT_OPTION JP_OPTION
#define BT_PAUSE  JP_PAUSE
#define BT_STAR   JP_STAR
#define BT_HASH   JP_NUM
#define BT_1      JP_1
#define BT_2      JP_2
#define BT_3      JP_3
#define BT_4      JP_4
#define BT_5      JP_5
#define BT_6      JP_6
#define BT_7      JP_7
#define BT_8      JP_8
#define BT_9      JP_9
#define BT_0      JP_0

#endif

// EOF

