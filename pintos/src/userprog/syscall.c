#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/loader.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "threads/malloc.h"

static void syscall_handler (struct intr_frame *);
static struct lock fs_lock;
void sys_halt(void);
void sys_exit(void);
void sys_exec(void);
void sys_wait(void);
void sys_create(void);
void sys_remove(void);
void sys_open(void);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&fs_lock);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  typedef int syscall_function (int, int, int);

  struct syscall
  {
     size_t arg_cnt;
     syscall_function *func;
  };

  static const struct syscall syscall_table[] =
  {
      {0,(syscall_function *) sys_halt},
      {1,(syscall_function *) sys_exit},
      {1,(syscall_function *) sys_exec},
      {1,(syscall_function *) sys_wait},
      {2,(syscall_function *) sys_create},
      {1,(syscall_function *) sys_remove},
      {1,(syscall_function *) sys_open}
   };

  const struct syscall *sc;
  unsigned call_nr;
  int args[3];
  /* Get the system call. */
  copy_in (&call_nr, f->esp, sizeof call_nr);
  if (call_nr >= sizeof syscall_table / sizeof *syscall_table)
    thread_exit ();
  sc = syscall_table + call_nr;
   /* Get the system call arguments. */
  ASSERT (sc->arg_cnt <= sizeof args / sizeof *args);
  memset (args, 0, sizeof args);
  copy_in (args, (uint32_t *) f->esp + 1, sizeof *args * sc->arg_cnt);
  /* Execute the system call,and set the return value. */
  f->eax = sc->func (args[0], args[1], args[2]);
}

/* Copies a byte from user address USRC to kernel address DST. */
static inline bool
get_user (uint8_t *dst, const uint8_t *usrc)
{
int eax;
asm ("movl $1f, %%eax; movb %2, %%al; movb %%al, %0; 1:"
: "=m" (*dst), "=&a" (eax) : "m" (*usrc));
return eax != 0;
}

/* Writes BYTE to user address UDST. */
static inline bool
put_user (uint8_t *udst, uint8_t byte)
{
int eax;
asm ("movl $1f, %%eax; movb %b2, %0; 1:"
: "=m" (*udst), "=&a" (eax) : "q" (byte));
return eax != 0;
}

sys_open (const char *ufile)
{
  char *kfile = copy_in_string (ufile);
  struct file_descriptor *fd;
  int handle =-1;

  fd = malloc (sizeof fd);
  if (fd != NULL)
  {
    lock_acquire (&fs_lock);
    fd->file = filesys_open (kfile);
    if (fd->file != NULL)
    {
      struct thread *cur = thread_current ();
      handle = fd->handle = cur->next_handle++;
      list_push_front (&cur->fds, &fd->elem);
     }
   else
       free (fd);
   lock_release (&fs_lock);
   }
   palloc_free_page (kfile);
   return handle;
}



static void
copy_in(const char *dest, char *start, int size){
  size_t length;
  if (dest == NULL)
    thread_exit();
  for(length = 0; length < size; length++)
  {
     if (start >= (char *) PHYS_BASE || !get_user (dest + length, start++))
     {
       thread_exit();
     }
   }
   return;
}

/* Creates a copy of user string US in kernel memory and returns it
as a page that must be freed with palloc_free_page().
Truncates the string at PGSIZE bytes in size. */
static char *
copy_in_string (const char *us)
{
  char *ks;
  size_t length;
  ks = palloc_get_page (PAL_ASSERT | PAL_ZERO);
  if (ks == NULL)
    thread_exit ();
  for (length = 0; length < PGSIZE; length++)
  {
    if (us >= (char *) PHYS_BASE || !get_user (ks + length, us++))
    {
      palloc_free_page (ks);
      thread_exit ();
    }
    if (ks[length] == '\0')
      return ks;
  }
  ks[PGSIZE-1] = '\0';
  return ks;
}

sys_halt(void){
 shutdown_power_off();
}

sys_exit(int status){
   struct thread *cur = thread_current();
  printf ("%s: exit(%d)\n", cur->name, status);
  thread_exit(); 
}


sys_exec(void){
return;
}

sys_wait(void){
return;
}

sys_create(void){
return;
}

sys_remove(void){
return;
}

