#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
# include <sys/types.h>
# include <sys/sysctl.h>
# include <sys/user.h>
# include <sys/proc.h>
# include <libgen.h>
#endif

#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
# include <libgen.h>
# include <unistd.h>
# include <fcntl.h>
# include <kvm.h>
# include <limits.h>
# include <sys/proc.h>
# include <sys/param.h>
# include <sys/resource.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>

#include "process.h"
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>

#if defined(__linux__) && !defined(PF_KTHREAD)
# define PF_KTHREAD 0x00200000
#endif

#define U64(n) (uint64_t) n
#define MEMSIZE U64

static Eina_Bool _show_kthreads = EINA_FALSE;

void
proc_info_kthreads_show_set(Eina_Bool enabled)
{
   _show_kthreads = enabled;
}

Eina_Bool
proc_info_kthreads_show_get(void)
{
   return _show_kthreads;
}

static const char *
_process_state_name(char state)
{
   const char *statename = NULL;
#if defined(__linux__)

   switch (state)
     {
      case 'D':
        statename = "dsleep";
        break;

      case 'I':
        statename = "idle";
        break;

      case 'R':
        statename = "run";
        break;

      case 'S':
        statename = "sleep";
        break;

      case 'T':
      case 't':
        statename = "stop";
        break;

      case 'X':
        statename = "dead";
        break;

      case 'Z':
        statename = "zomb";
        break;
     }
#else
   switch (state)
     {
      case SIDL:
        statename = "idle";
        break;

      case SRUN:
        statename = "run";
        break;

      case SSLEEP:
        statename = "sleep";
        break;

      case SSTOP:
        statename = "stop";
        break;

#if !defined(__OpenBSD__)
      case SWAIT:
        statename = "wait";
        break;

      case SLOCK:
        statename = "lock";
        break;

#endif
      case SZOMB:
        statename = "zomb";
        break;

#if defined(__OpenBSD__)
      case SDEAD:
        statename = "dead";
        break;

      case SONPROC:
        statename = "onproc";
        break;
#endif
     }
#endif
   return statename;
}

#if defined(__linux__)

static unsigned long
_parse_line(const char *line)
{
   char *p, *tok;

   p = strchr(line, ':') + 1;
   while (isspace(*p))
     p++;
   tok = strtok(p, " ");

   return atol(tok);
}

static void
_mem_size(Proc_Info *proc)
{
   FILE *f;
   char buf[1024];
   unsigned int dummy, size, shared, resident, data, text;
   static int pagesize = 0;

   if (!pagesize) pagesize = getpagesize();

   f = fopen(eina_slstr_printf("/proc/%d/statm", proc->pid), "r");
   if (!f) return;

   if (fgets(buf, sizeof(buf), f))
     {
        if (sscanf(buf, "%u %u %u %u %u %u %u",
                   &size, &resident, &shared, &text,
                   &dummy, &data, &dummy) == 7)
          {
             proc->mem_rss = MEMSIZE(resident) * MEMSIZE(pagesize);
             proc->mem_shared = MEMSIZE(shared) * MEMSIZE(pagesize);
             proc->mem_size = proc->mem_rss - proc->mem_shared;
             proc->mem_virt = MEMSIZE(size) * MEMSIZE(pagesize);
          }
     }

   fclose(f);
}

static void
_cmd_args(Proc_Info *p, char *name, size_t len)
{
   char line[4096];
   int pid = p->pid;

   char *link = ecore_file_readlink(eina_slstr_printf("/proc/%d/exe", pid));
   if (link)
     {
        snprintf(name, len, "%s", ecore_file_file_get(link));
        free(link);
     }

   FILE *f = fopen(eina_slstr_printf("/proc/%d/cmdline", pid), "r");
   if (f)
     {
        if (fgets(line, sizeof(line), f))
          {
             Eina_Strbuf *buf = eina_strbuf_new();
             const char *n;

             if (ecore_file_exists(line))
               snprintf(name, len, "%s", ecore_file_file_get(line));

             n = line;
             while (*n && (*n + 1))
               {
                  eina_strbuf_append(buf, n);
                  n = strchr(n, '\0') + 1;
                  if (*n && (*n + 1)) eina_strbuf_append(buf, " ");
               }
             p->arguments = eina_strbuf_release(buf);
          }
        fclose(f);
     }

   char *end = strchr(name, ' ');
   if (end) *end = '\0';

   p->command = strdup(name);
}

static int
_uid(int pid)
{
   FILE *f;
   int uid = -1;
   char line[1024];

   f = fopen(eina_slstr_printf("/proc/%d/status", pid), "r");
   if (!f) return -1;

   while ((fgets(line, sizeof(line), f)) != NULL)
     {
        if (!strncmp(line, "Uid:", 4))
          {
             uid = _parse_line(line);
             break;
          }
     }

   fclose(f);

   return uid;
}

static int64_t
_boot_time(void)
{
   FILE *f;
   int64_t boot_time;
   char buf[4096];
   double uptime = 0.0;

   f = fopen("/proc/uptime", "r");
   if (!f) return 0;

   if (fgets(buf, sizeof(buf), f))
     sscanf(buf, "%lf", &uptime);
   else boot_time = 0;

   fclose(f);

   if (uptime > 0.0)
     boot_time = time(NULL) - (time_t) uptime;

   return boot_time;
}

typedef struct {
   int pid, ppid, utime, stime, cutime, cstime;
   int psr, pri, nice, numthreads;
   long long int start_time;
   char state;
   unsigned int mem_rss, flags;
   unsigned long mem_virt;
   char name[1024];
} Stat;

static Eina_Bool
_stat(const char *path, Stat *st)
{
   FILE *f;
   char line[4096];
   int dummy, res = 0;
   static int64_t boot_time = 0;

   if (!boot_time) boot_time = _boot_time();

   memset(st, 0, sizeof(Stat));

   f = fopen(path, "r");
   if (!f) return EINA_FALSE;

   if (fgets(line, sizeof(line), f))
     {
        char *end, *start = strchr(line, '(') + 1;
        end = strchr(line, ')');

        strncpy(st->name, start, end - start);
        st->name[end - start] = '\0';
        res = sscanf(end + 2, "%c %d %d %d %d %d %u %u %u %u %u %d %d %d"
              " %d %d %d %u %u %lld %lu %u %u %u %u %u %u %u %d %d %d %d %u"
              " %d %d %d %d %d %d %d %d %d",
              &st->state, &st->ppid, &dummy, &dummy, &dummy, &dummy, &st->flags,
              &dummy, &dummy, &dummy, &dummy, &st->utime, &st->stime, &st->cutime,
              &st->cstime, &st->pri, &st->nice, &st->numthreads, &dummy, &st->start_time,
              &st->mem_virt, &st->mem_rss, &dummy, &dummy, &dummy, &dummy, &dummy,
              &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy,
              &dummy, &dummy, &st->psr, &dummy, &dummy, &dummy, &dummy, &dummy);
     }
   fclose(f);

   if (res != 42) return EINA_FALSE;

   st->start_time /= sysconf(_SC_CLK_TCK);
   st->start_time += boot_time;

   return EINA_TRUE;
}

static Eina_List *
_process_list_linux_get(void)
{
   Eina_List *files, *list;
   char *n;
   Stat st;

   list = NULL;

   files = ecore_file_ls("/proc");
   EINA_LIST_FREE(files, n)
     {
        int pid = atoi(n);
        free(n);

        if (!pid) continue;

        if (!_stat(eina_slstr_printf("/proc/%d/stat", pid), &st))
          continue;

        if (st.flags & PF_KTHREAD && !proc_info_kthreads_show_get())
          continue;

        Proc_Info *p = calloc(1, sizeof(Proc_Info));
        if (!p) return NULL;

        p->pid = pid;
        p->ppid = st.ppid;
        p->uid = _uid(pid);
        p->cpu_id = st.psr;
        p->start = st.start_time;
        p->state = _process_state_name(st.state);
        p->cpu_time = st.utime + st.stime;
        p->nice = st.nice;
        p->priority = st.pri;
        p->numthreads = st.numthreads;
        _mem_size(p);
        _cmd_args(p, st.name, sizeof(st.name));

        list = eina_list_append(list, p);
     }

   return list;
}

static void
_proc_thread_info(Proc_Info *p)
{
   Eina_List *files;
   char *n;
   Stat st;

   files = ecore_file_ls(eina_slstr_printf("/proc/%d/task", p->pid));
   EINA_LIST_FREE(files, n)
     {
        int tid = atoi(n);
        free(n);
        if (!_stat(eina_slstr_printf("/proc/%d/task/%d/stat", p->pid, tid), &st))
          continue;

        Proc_Info *t = calloc(1, sizeof(Proc_Info));
        if (!t) continue;
        t->cpu_id = st.psr;
        t->state = _process_state_name(st.state);
        t->cpu_time = st.utime + st.stime;
        t->nice = st.nice;
        t->priority = st.pri;
        t->numthreads = st.numthreads;
        t->mem_virt = st.mem_virt;
        t->mem_rss = st.mem_rss;

        t->tid = tid;
        t->thread_name = strdup(st.name);

        p->threads = eina_list_append(p->threads, t);
     }
}

Proc_Info *
proc_info_by_pid(int pid)
{
   Stat st;

   if (!_stat(eina_slstr_printf("/proc/%d/stat", pid), &st))
     return NULL;

   Proc_Info *p = calloc(1, sizeof(Proc_Info));
   if (!p) return NULL;

   p->pid = pid;
   p->ppid = st.ppid;
   p->uid = _uid(pid);
   p->cpu_id = st.psr;
   p->start = st.start_time;
   p->state = _process_state_name(st.state);
   p->cpu_time = st.utime + st.stime;
   p->priority = st.pri;
   p->nice = st.nice;
   p->numthreads = st.numthreads;
   _mem_size(p);
   _cmd_args(p, st.name, sizeof(st.name));

   _proc_thread_info(p);

   return p;
}

#endif

#if defined(__OpenBSD__)

static void
_proc_get(Proc_Info *p, struct kinfo_proc *kp)
{
   static int pagesize = 0;

   if (!pagesize) pagesize = getpagesize();

   p->pid = kp->p_pid;
   p->ppid = kp->p_ppid;
   p->uid = kp->p_uid;
   p->cpu_id = kp->p_cpuid;
   p->start = kp->p_ustart_sec;
   p->state = _process_state_name(kp->p_stat);
   p->cpu_time = kp->p_uticks + kp->p_sticks + kp->p_iticks;
   p->mem_virt = p->mem_size = (MEMSIZE(kp->p_vm_tsize) * MEMSIZE(pagesize)) +
      (MEMSIZE(kp->p_vm_dsize) * MEMSIZE(pagesize)) + (MEMSIZE(kp->p_vm_ssize) * MEMSIZE(pagesize));
   p->mem_rss = MEMSIZE(kp->p_vm_rssize) * MEMSIZE(pagesize);
   p->mem_size = p->mem_rss;
   p->priority = kp->p_priority - PZERO;
   p->nice = kp->p_nice - NZERO;
   p->tid = kp->p_tid;
}

static void
_cmd_get(Proc_Info *p, kvm_t *kern, struct kinfo_proc *kp)
{
   char **args;
   char name[1024];

   if ((args = kvm_getargv(kern, kp, sizeof(name)-1)))
     {
        Eina_Strbuf *buf = eina_strbuf_new();
        for (int i = 0; args[i]; i++)
          {
             eina_strbuf_append(buf, args[i]);
             if (args[i + 1])
               eina_strbuf_append(buf, " ");
          }
        p->arguments = eina_strbuf_string_steal(buf);
        eina_strbuf_free(buf);

        if (args[0] && ecore_file_exists(args[0]))
          p->command = strdup(ecore_file_file_get(args[0]));
     }

   if (!p->command)
     p->command = strdup(kp->p_comm);
}

Proc_Info *
proc_info_by_pid(int pid)
{
   struct kinfo_proc *kp, *kpt;
   kvm_t *kern;
   char errbuf[_POSIX2_LINE_MAX];
   int count, pid_count;

   kern = kvm_openfiles(NULL, NULL, NULL, KVM_NO_FILES, errbuf);
   if (!kern) return NULL;

   kp = kvm_getprocs(kern, KERN_PROC_PID, pid, sizeof(*kp), &count);
   if (!kp) return NULL;

   if (count == 0) return NULL;

   Proc_Info *p = calloc(1, sizeof(Proc_Info));
   if (!p) return NULL;

   _proc_get(p, kp);
   _cmd_get(p, kern, kp);

   kp = kvm_getprocs(kern, KERN_PROC_SHOW_THREADS, 0, sizeof(*kp), &pid_count);

   for (int i = 0; i < pid_count; i++)
     {
        if (kp[i].p_pid != p->pid) continue;

        kpt = &kp[i];

        if (kpt->p_tid <= 0) continue;

        Proc_Info *t = calloc(1, sizeof(Proc_Info));
        if (!t) continue;

        _proc_get(t, kpt);

        t->tid = kpt->p_tid;
        t->thread_name = strdup(kpt->p_comm);

        p->threads = eina_list_append(p->threads, t);
     }

   p->numthreads = eina_list_count(p->threads);

   kvm_close(kern);

   return p;
}

static Eina_List *
_process_list_openbsd_get(void)
{
   struct kinfo_proc *kps, *kp;
   Proc_Info *p;
   char errbuf[4096];
   kvm_t *kern;
   int pid_count;
   Eina_List *list = NULL;

   kern = kvm_openfiles(NULL, NULL, NULL, KVM_NO_FILES, errbuf);
   if (!kern) return NULL;

   kps = kvm_getprocs(kern, KERN_PROC_ALL, 0, sizeof(*kps), &pid_count);
   if (!kps) return NULL;

   for (int i = 0; i < pid_count; i++)
     {
        p = calloc(1, sizeof(Proc_Info));
        if (!p) return NULL;

        kp = &kps[i];

        _proc_get(p, kp);
        _cmd_get(p, kern, kp);

        list = eina_list_append(list, p);
     }

   kvm_close(kern);

   return list;
}

#endif

#if defined(__FreeBSD__) || defined(__DragonFly__)

static int
_pid_max(void)
{
   size_t len;
   static int pid_max = 0;

   if (pid_max != 0) return pid_max;

   len = sizeof(pid_max);
   if (sysctlbyname("kern.pid_max", &pid_max, &len, NULL, 0) == -1)
     {
#if defined(__FreeBSD__)
        pid_max = 99999;
#elif defined(__DragonFly__)
        pid_max = 999999;
#else
        pid_max = PID_MAX;
#endif
     }

   return pid_max;
}

static void
_cmd_get(Proc_Info *p, struct kinfo_proc *kp)
{
   kvm_t * kern;
   char **args;
   char name[1024];
   Eina_Bool have_command = EINA_FALSE;

   kern = kvm_open(NULL, "/dev/null", NULL, O_RDONLY, "kvm_open");
   if (kern)
     {
        if ((args = kvm_getargv(kern, kp, sizeof(name)-1)) && (args[0]))
          {
             char *base = strdup(args[0]);
             if (base)
               {
                  char *spc = strchr(base, ' ');
                  if (spc) *spc = '\0';

                  if (ecore_file_exists(base))
                    {
                       snprintf(name, sizeof(name), "%s", basename(base));
                       have_command = EINA_TRUE;
                    }
                  free(base);
               }
             Eina_Strbuf *buf = eina_strbuf_new();
             for (int i = 0; args[i] != NULL; i++)
               {
                  eina_strbuf_append(buf, args[i]);
                  if (args[i + 1])
                    eina_strbuf_append(buf, " ");
               }
             p->arguments = eina_strbuf_string_steal(buf);
             eina_strbuf_free(buf);
          }
        kvm_close(kern);
     }

   if (!have_command)
     snprintf(name, sizeof(name), "%s", kp->ki_comm);

   p->command = strdup(name);
}

static Proc_Info *
_proc_thread_info(struct kinfo_proc *kp, Eina_Bool is_thread)
{
   struct rusage *usage;
   Proc_Info *p;
   static int pagesize = 0;

   if (!pagesize) pagesize = getpagesize();

   p = calloc(1, sizeof(Proc_Info));
   if (!p) return NULL;

   p->pid = kp->ki_pid;
   p->ppid = kp->ki_ppid;
   p->uid = kp->ki_uid;

   if (!is_thread)
     _cmd_get(p, kp);

   p->cpu_id = kp->ki_oncpu;
   if (p->cpu_id == -1)
     p->cpu_id = kp->ki_lastcpu;

   usage = &kp->ki_rusage;

   p->cpu_time = (usage->ru_utime.tv_sec * 1000000) + usage->ru_utime.tv_usec +
       (usage->ru_stime.tv_sec * 1000000) + usage->ru_stime.tv_usec;
   p->cpu_time /= 10000;
   p->state = _process_state_name(kp->ki_stat);
   p->mem_virt = kp->ki_size;
   p->mem_rss = MEMSIZE(kp->ki_rssize) * MEMSIZE(pagesize);
   p->start = kp->ki_start.tv_sec;
   p->mem_size = p->mem_rss;
   p->nice = kp->ki_nice - NZERO;
   p->priority = kp->ki_pri.pri_level - PZERO;
   p->numthreads = kp->ki_numthreads;

   p->tid = kp->ki_tid;
   p->thread_name = strdup(kp->ki_tdname);

   return p;
}

static Eina_List *
_process_list_freebsd_fallback_get(void)
{
   Eina_List *list;
   struct kinfo_proc kp;
   int mib[4];
   size_t len;
   static int pid_max;

   pid_max = _pid_max();

   list = NULL;

   len = sizeof(int);
   if (sysctlnametomib("kern.proc.pid", mib, &len) == -1)
     return NULL;

   for (int i = 1; i <= pid_max; i++)
     {
        mib[3] = i;
        len = sizeof(kp);
        if (sysctl(mib, 4, &kp, &len, NULL, 0) == -1)
          continue;

        if (kp.ki_flag & P_KPROC && !proc_info_kthreads_show_get())
          continue;

        Proc_Info *p = _proc_thread_info(&kp, EINA_FALSE);
        if (p)
          list = eina_list_append(list, p);
     }

   return list;
}

static Eina_List *
_process_list_freebsd_get(void)
{
   kvm_t *kern;
   Eina_List *list = NULL;
   struct kinfo_proc *kps, *kp;
   char errbuf[_POSIX2_LINE_MAX];
   int pid_count;

   kern = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errbuf);
   if (!kern)
     return _process_list_freebsd_fallback_get();

   kps = kvm_getprocs(kern, KERN_PROC_PROC, 0, &pid_count);
   if (!kps)
     {
        kvm_close(kern);
        return _process_list_freebsd_fallback_get();
     }

   for (int i = 0; i < pid_count; i++)
     {
        if (kps[i].ki_flag & P_KPROC && !proc_info_kthreads_show_get())
          continue;

        kp = &kps[i];

        Proc_Info *p = _proc_thread_info(kp, EINA_FALSE);
        if (p)
          list = eina_list_append(list, p);
     }

   kvm_close(kern);

   return list;
}

static Proc_Info *
_proc_info_by_pid_fallback(int pid)
{
   struct kinfo_proc kp;
   int mib[4];
   size_t len;

   len = sizeof(int);
   if (sysctlnametomib("kern.proc.pid", mib, &len) == -1)
     return NULL;

   mib[3] = pid;

   len = sizeof(kp);
   if (sysctl(mib, 4, &kp, &len, NULL, 0) == -1)
     return NULL;

   Proc_Info *p = _proc_thread_info(&kp, EINA_FALSE);

   return p;
}

Proc_Info *
proc_info_by_pid(int pid)
{
   kvm_t *kern;
   struct kinfo_proc *kps, *kp;
   char errbuf[_POSIX2_LINE_MAX];
   int pid_count;

   kern = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errbuf);
   if (!kern)
     return _proc_info_by_pid_fallback(pid);

   kps = kvm_getprocs(kern, KERN_PROC_ALL, 0, &pid_count);
   if (!kps)
     {
        kvm_close(kern);
        return _proc_info_by_pid_fallback(pid);
     }

   Proc_Info *p = NULL;

   for (int i = 0; i < pid_count; i++)
     {
        if (kps[i].ki_flag & P_KPROC && !proc_info_kthreads_show_get())
          continue;
        if (kps[i].ki_pid != pid)
          continue;

        kp = &kps[i];
        Proc_Info *t = _proc_thread_info(kp, EINA_TRUE);
        if (!p)
          {
             p = _proc_thread_info(kp, EINA_FALSE);
             p->cpu_time = 0;
          }

        p->cpu_time += t->cpu_time;
        p->threads = eina_list_append(p->threads, t);
     }

   kvm_close(kern);

   if (!p) return _proc_info_by_pid_fallback(pid);

   return p;
}
#endif

void
proc_info_free(Proc_Info *proc)
{
   Proc_Info *t;

   EINA_LIST_FREE(proc->threads, t)
     {
        proc_info_free(t);
     }

   if (proc->children)
     eina_list_free(proc->children);

   if (proc->command)
     free(proc->command);
   if (proc->arguments)
     free(proc->arguments);
   if (proc->thread_name)
     free(proc->thread_name);

   free(proc);
}

Eina_List *
proc_info_all_get(void)
{
   Eina_List *processes;

#if defined(__linux__)
   processes = _process_list_linux_get();
#elif defined(__FreeBSD__) || defined(__DragonFly__)
   processes = _process_list_freebsd_get();
#elif defined(__OpenBSD__)
   processes = _process_list_openbsd_get();
#else
   processes = NULL;
#endif

   return processes;
}

static Eina_Bool
_child_add(Eina_List *parents, Proc_Info *child)
{
   Eina_List *l;
   Proc_Info *parent;

   EINA_LIST_FOREACH(parents, l, parent)
     {
        if (parent->pid == child->ppid)
          {
             parent->children = eina_list_append(parent->children, child);
             return 1;
          }
     }

   return 0;
}

Eina_List *
proc_info_all_children_get()
{
   Proc_Info *proc;
   Eina_List *l;
   Eina_List *procs;

   procs = proc_info_all_get();

   EINA_LIST_FOREACH(procs, l, proc)
     {
        int ok =_child_add(procs,  proc);
        (void) ok;
     }

    return procs;
}

Eina_List *
_append_wanted(Eina_List *wanted, Eina_List *tree)
{
   Eina_List *l;
   Proc_Info *parent;

   EINA_LIST_FOREACH(tree, l, parent)
     {
        wanted = eina_list_append(wanted, parent);
        if (parent->children)
          wanted = _append_wanted(wanted, parent->children);
     }
   return wanted;
}

Eina_List *
proc_info_pid_children_get(pid_t pid)
{
   Proc_Info *proc;
   Eina_List *l, *procs, *wanted = NULL;

   procs = proc_info_all_children_get();

   EINA_LIST_FOREACH(procs, l, proc)
     {
        if (!wanted && proc->pid == pid)
          {
             wanted = eina_list_append(wanted, proc);
             if (proc->children)
               wanted = _append_wanted(wanted, proc->children);
          }
     }

   EINA_LIST_FREE(procs, proc)
     {
        if (!eina_list_data_find(wanted, proc))
          {
             proc_info_free(proc);
          }
     }

    return wanted;
}

void
proc_info_all_children_free(Eina_List *pstree)
{
   Proc_Info *parent, *child;

   EINA_LIST_FREE(pstree, parent)
     {
        EINA_LIST_FREE(parent->children, child)
          proc_info_pid_children_free(child);
        proc_info_free(parent);
     }
}

void
proc_info_pid_children_free(Proc_Info *proc)
{
   Proc_Info *child;

   EINA_LIST_FREE(proc->children, child)
     proc_info_free(child);

   proc_info_free(proc);
}

