#ifndef __PROC_H__
#define __PROC_H__

#include <Eina.h>
#include <stdint.h>
#include <unistd.h>

#if !defined(PID_MAX)
# define PID_MAX     99999
#endif

typedef struct _Proc_Info
{
   pid_t       pid;
   pid_t       ppid;
   uid_t       uid;
   int8_t      nice;
   int8_t      priority;
   int         cpu_id;
   int32_t     numthreads;
   int64_t     cpu_time;
   double      cpu_usage;
   int64_t     start;

   uint64_t    mem_size;
   uint64_t    mem_virt;
   uint64_t    mem_rss;
   uint64_t    mem_shared;

   char       *command;
   char       *arguments;
   const char *state;

   int         tid;
   char       *thread_name;

   Eina_List  *threads;
   Eina_List  *children;
} Proc_Info;

Eina_List *
proc_info_all_get(void);

Proc_Info *
proc_info_by_pid(int pid);

void
proc_info_free(Proc_Info *proc);

void
proc_info_kthreads_show_set(Eina_Bool enabled);

Eina_Bool
proc_info_kthreads_show_get(void);

Eina_List *
proc_info_all_children_get(void);

Eina_List *
proc_info_pid_children_get(pid_t pid);

void
proc_info_pid_children_free(Proc_Info *procs);

#endif
