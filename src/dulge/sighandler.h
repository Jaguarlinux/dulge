/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#ifndef PM_SIGHANDLER_H
#define PM_SIGHANDLER_H

void install_segv_handler(void);
void install_winch_handler(void);
void install_soft_interrupt_handler(void);
void remove_soft_interrupt_handler(void);

#endif /* PM_SIGHANDLER_H */
