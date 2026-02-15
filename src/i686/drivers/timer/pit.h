#pragma once

#define PIT_CHANNEL_0_REGISTER 0x40
#define PIT_CHANNEL_1_REGISTER 0x41
#define PIT_CHANNEL_2_REGISTER 0x42
#define PIT_COMMAND_REGISTER 0x43

void pit_init();

// TODO: Implement functionality to configure the mode and delays