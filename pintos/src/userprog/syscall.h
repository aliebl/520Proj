#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
static void copy_in(const char *dest, char *start, int size);
static char *copy_in_string(const char *us);

#endif /* userprog/syscall.h */
