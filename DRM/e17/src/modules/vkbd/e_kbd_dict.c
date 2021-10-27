#include "e.h"
#include "e_kbd_dict.h"

#include <fcntl.h>
#include <sys/mman.h>

#define MAXLATIN 0x100

static unsigned char _e_kbd_normalise_base[MAXLATIN];
static unsigned char _e_kbd_normalise_ready = 0;

static void
_e_kbd_normalise_init(void)
{
   int i;
   const char *table[][2] =
     {
          {"À", "a"},
          {"Á", "a"},
          {"Â", "a"},
          {"Ã", "a"},
          {"Ä", "a"},
          {"Å", "a"},
          {"Æ", "a"},
          {"Ç", "c"},
          {"È", "e"},
          {"É", "e"},
          {"Ê", "e"},
          {"Ë", "e"},
          {"Ì", "i"},
          {"Í", "i"},
          {"Î", "i"},
          {"Ï", "i"},
          {"Ð", "d"},
          {"Ñ", "n"},
          {"Ò", "o"},
          {"Ó", "o"},
          {"Ô", "o"},
          {"Õ", "o"},
          {"Ö", "o"},
          {"×", "x"},
          {"Ø", "o"},
          {"Ù", "u"},
          {"Ú", "u"},
          {"Û", "u"},
          {"Ü", "u"},
          {"Ý", "y"},
          {"Þ", "p"},
          {"ß", "s"},
          {"à", "a"},
          {"á", "a"},
          {"â", "a"},
          {"ã", "a"},
          {"ä", "a"},
          {"å", "a"},
          {"æ", "a"},
          {"ç", "c"},
          {"è", "e"},
          {"é", "e"},
          {"ê", "e"},
          {"ë", "e"},
          {"ì", "i"},
          {"í", "i"},
          {"î", "i"},
          {"ï", "i"},
          {"ð", "o"},
          {"ñ", "n"},
          {"ò", "o"},
          {"ó", "o"},
          {"ô", "o"},
          {"õ", "o"},
          {"ö", "o"},
          {"ø", "o"},
          {"ù", "u"},
          {"ú", "u"},
          {"û", "u"},
          {"ü", "u"},
          {"ý", "y"},
          {"þ", "p"},
          {"ÿ", "y"}
     }; // 63 items

   if (_e_kbd_normalise_ready) return;
   _e_kbd_normalise_ready = 1;
   for (i = 0; i < 128; i++)
     _e_kbd_normalise_base[i] = tolower(i);
   for (;i < MAXLATIN; i++)
     {
        int glyph, j;

        for (j = 0; j < 63; j++)
          {
             evas_string_char_next_get(table[j][0], 0, &glyph);
             if (glyph == i)
               {
                  _e_kbd_normalise_base[i] = *table[j][1];
                  break;
               }
          }
     }
}

static int
_e_kbd_dict_letter_normalise(int glyph)
{
   // FIXME: ö -> o, ä -> a, Ó -> o etc. - ie normalise to latin-1
   if (glyph < MAXLATIN) return _e_kbd_normalise_base[glyph];
   return tolower(glyph) & 0x7f;
}

static int
_e_kbd_dict_normalized_strncmp(const char *a, const char *b, int len)
{
   // FIXME: normalise 2 strings and then compare
   if (len < 0) return strcasecmp(a, b);
   return strncasecmp(a, b, len);
}

static int
_e_kbd_dict_normalized_strcmp(const char *a, const char *b)
{
   return _e_kbd_dict_normalized_strncmp(a, b, -1);
}

static void
_e_kbd_dict_normalized_strcpy(char *dst, const char *src)
{
   const char *p;
   char *d;

   for (p = src, d = dst; *p; p++, d++)
     *d = _e_kbd_dict_letter_normalise(*p);
   *d = 0;
}

static int
_e_kbd_dict_matches_lookup_cb_sort(const void *d1, const void *d2)
{
   const E_Kbd_Dict_Word *kw1, *kw2;

   kw1 = d1;
   kw2 = d2;
   if (kw1->usage < kw2->usage) return 1;
   else if (kw1->usage > kw2->usage) return -1;
   return 0;
}

static int
_e_kbd_dict_writes_cb_sort(const void *d1, const void *d2)
{
   const E_Kbd_Dict_Word *kw1, *kw2;

   kw1 = d1;
   kw2 = d2;
   return _e_kbd_dict_normalized_strcmp(kw1->word, kw2->word);
}

static const char *
_e_kbd_dict_line_next(E_Kbd_Dict *kd, const char *p)
{
   const char *e, *pp;

   e = kd->file.dict + kd->file.size;
   for (pp = p; pp < e; pp++)
     if (*pp == '\n') return pp + 1;
   return NULL;
}

static char *
_e_kbd_dict_line_parse(E_Kbd_Dict *kd EINA_UNUSED, const char *p, int *usage)
{
   const char *ps;
   char *wd = NULL;

   for (ps = p; !isspace(*ps); ps++);
   wd = malloc(ps - p + 1);
   if (!wd) return NULL;
   strncpy(wd, p, ps - p);
   wd[ps - p] = 0;
   if (*ps == '\n') *usage = 0;
   else
     {
        ps++;
        *usage = atoi(ps);
     }
   return wd;
}

static void
_e_kbd_dict_lookup_build_line(E_Kbd_Dict *kd EINA_UNUSED, const char *p, const char *eol, int *glyphs)
{
   char *s;
   int p2;

   s = alloca(eol - p + 1);
   strncpy(s, p, eol - p);
   s[eol - p] = 0;
   p2 = evas_string_char_next_get(s, 0, &(glyphs[0]));
   if ((p2 > 0) && (glyphs[0] > 0))
     evas_string_char_next_get(s, p2, &(glyphs[1]));
}

static void
_e_kbd_dict_lookup_build(E_Kbd_Dict *kd)
{
   const char *p, *e, *eol, *base;
   int glyphs[2], pglyphs[2];
   int i, j, line = 0;

   p = base = kd->file.dict;
   e = p + kd->file.size;
   pglyphs[0] = pglyphs[1] = 0;
   for (j = 0; j < 128; j++)
     {
        for (i = 0; i < 128; i++)
          kd->lookup.tuples[j][i] = -1;
     }
   while (p < e)
     {
        eol = strchr(p, '\n');
        if (!eol) break;
        line++;
        if ((p - base) >= 0x7fffffff)
          {
             ERR("DICT %s TOO BIG! must be < 2GB", kd->file.file);
             return;
          }
        if (eol > p)
          {
             glyphs[0] = glyphs[1] = 0;
             _e_kbd_dict_lookup_build_line(kd, p, eol, glyphs);
             if ((glyphs[1] != pglyphs[1]) || (glyphs[0] != pglyphs[0]))
               {
                  int v1, v2;

                  if (isspace(glyphs[0]))
                    {
                       glyphs[0] = 0;
                       glyphs[1] = 0;
                    }
                  else if (isspace(glyphs[1]))
                    glyphs[1] = 0;
                  if (glyphs[0] == 0)
                    {
                       pglyphs[0] = pglyphs[1] = 0;
                       p = eol + 1;
                       continue;
                    }
                  v1 = _e_kbd_dict_letter_normalise(glyphs[0]);
                  v2 = _e_kbd_dict_letter_normalise(glyphs[1]);
                  if (kd->lookup.tuples[v1][v2] == -1)
                    kd->lookup.tuples[v1][v2] = (unsigned int)(p - base);
                  pglyphs[0] = v1;
                  pglyphs[1] = v2;
               }
          }
        p = eol + 1;
     }
}

static int
_e_kbd_dict_open(E_Kbd_Dict *kd)
{
   struct stat st;

   kd->file.fd = open(kd->file.file, O_RDONLY);
   if (kd->file.fd < 0) return 0;
   if (fstat(kd->file.fd, &st) < 0)
     {
        close(kd->file.fd);
        return 0;
     }
   kd->file.size = st.st_size;

   eina_mmap_safety_enabled_set(EINA_TRUE);

   kd->file.dict = mmap(NULL, kd->file.size, PROT_READ, MAP_SHARED,
                        kd->file.fd, 0);
   if ((kd->file.dict== MAP_FAILED) || (!kd->file.dict))
     {
        close(kd->file.fd);
        return 0;
     }
   return 1;
}

static void
_e_kbd_dict_close(E_Kbd_Dict *kd)
{
   if (kd->file.fd < 0) return;
   memset(kd->lookup.tuples, 0, sizeof(kd->lookup.tuples));
   munmap((void *)kd->file.dict, kd->file.size);
   close(kd->file.fd);
   kd->file.fd = -1;
   kd->file.dict = NULL;
   kd->file.size = 0;
}

EAPI E_Kbd_Dict *
e_kbd_dict_new(const char *file)
{
   // alloc and load new dict - build quick-lookup table. words MUST be sorted
   E_Kbd_Dict *kd;

   _e_kbd_normalise_init();
   kd = E_NEW(E_Kbd_Dict, 1);
   if (!kd) return NULL;
   kd->file.file = eina_stringshare_add(file);
   if (!kd->file.file)
     {
        free(kd);
        return NULL;
     }
   kd->file.fd = -1;
   if (!_e_kbd_dict_open(kd))
     {
        eina_stringshare_del(kd->file.file);
        free(kd);
        return NULL;
     }
   _e_kbd_dict_lookup_build(kd);
   return kd;
}

EAPI void
e_kbd_dict_free(E_Kbd_Dict *kd)
{
   // free dict and anything in it
   e_kbd_dict_word_letter_clear(kd);
   e_kbd_dict_save(kd);
   _e_kbd_dict_close(kd);
   free(kd);
}

static E_Kbd_Dict_Word *
_e_kbd_dict_changed_write_find(E_Kbd_Dict *kd, const char *word)
{
   Eina_List *l;

   for (l = kd->changed.writes; l; l = l->next)
     {
        E_Kbd_Dict_Word *kw;

        kw = l->data;
        if (!strcmp(kw->word, word)) return kw;
     }
   return NULL;
}

EAPI void
e_kbd_dict_save(E_Kbd_Dict *kd)
{
   FILE *f;

   // XXX: disable personal dict saving for now as we don't merge personal
   // XXX: and system dict stats very well at all
   return;
   // save any changes (new words added, usage adjustments).
   // all words MUST be sorted
   if (!kd->changed.writes) return;
   if (kd->changed.flush_timer)
     {
        ecore_timer_del(kd->changed.flush_timer);
        kd->changed.flush_timer = NULL;
     }
   ecore_file_unlink(kd->file.file);
   f = fopen(kd->file.file, "w");
   kd->changed.writes = eina_list_sort(kd->changed.writes,
                                       eina_list_count(kd->changed.writes),
                                       _e_kbd_dict_writes_cb_sort);
   if (f)
     {
        const char *p, *pn;

        p = kd->file.dict;
        while (p)
          {
             char *wd;
             int usage = 0;

             pn = _e_kbd_dict_line_next(kd, p);
             if (!pn)
               {
                  fclose(f);
                  return;
               }
             wd = _e_kbd_dict_line_parse(kd, p, &usage);
             if ((wd) && (strlen(wd) > 0))
               {
                  if (kd->changed.writes)
                    {
                       int writeline = 0;

                       while (kd->changed.writes)
                         {
                            E_Kbd_Dict_Word *kw;
                            int cmp;

                            kw = kd->changed.writes->data;
                            cmp = _e_kbd_dict_normalized_strcmp(kw->word, wd);
                            if (cmp < 0)
                              {
                                 fprintf(f, "%s %i\n", kw->word, kw->usage);
                                 writeline = 1;
                                 eina_stringshare_del(kw->word);
                                 free(kw);
                                 kd->changed.writes  = eina_list_remove_list(kd->changed.writes, kd->changed.writes);
                              }
                            else if (cmp == 0)
                              {
                                 fprintf(f, "%s %i\n", wd, kw->usage);
                                 if (!strcmp(kw->word, wd))
                                   writeline = 0;
                                 else
                                   writeline = 1;
                                 eina_stringshare_del(kw->word);
                                 free(kw);
                                 kd->changed.writes  = eina_list_remove_list(kd->changed.writes, kd->changed.writes);
                                 break;
                              }
                            else if (cmp > 0)
                              {
                                 writeline = 1;
                                 break;
                              }
                         }
                       if (writeline)
                         fprintf(f, "%s %i\n", wd, usage);
                    }
                  else
                    fprintf(f, "%s %i\n", wd, usage);
               }
             free(wd);
             p = pn;
             if (p >= (kd->file.dict + kd->file.size)) break;
          }
        while (kd->changed.writes)
          {
             E_Kbd_Dict_Word *kw;

             kw = kd->changed.writes->data;
             fprintf(f, "%s %i\n", kw->word, kw->usage);
             eina_stringshare_del(kw->word);
             free(kw);
             kd->changed.writes  = eina_list_remove_list(kd->changed.writes, kd->changed.writes);
          }
        fclose(f);
     }
   _e_kbd_dict_close(kd);
   if (_e_kbd_dict_open(kd)) _e_kbd_dict_lookup_build(kd);
}

static Eina_Bool
_e_kbd_dict_cb_save_flush(void *data)
{
   E_Kbd_Dict *kd;

   kd = data;
   if ((kd->matches.list) || (kd->word.letters) || (kd->matches.deadends) ||
       (kd->matches.leads))
     return EINA_TRUE;
   kd->changed.flush_timer = NULL;
   e_kbd_dict_save(kd);
   return EINA_FALSE;
}

static void
_e_kbd_dict_changed_write_add(E_Kbd_Dict *kd, const char *word, int usage)
{
   E_Kbd_Dict_Word *kw;

   kw = E_NEW(E_Kbd_Dict_Word, 1);
   kw->word = eina_stringshare_add(word);
   kw->usage = usage;
   kd->changed.writes = eina_list_prepend(kd->changed.writes, kw);
   if (eina_list_count(kd->changed.writes) > 64)
     e_kbd_dict_save(kd);
   else
     {
        if (kd->changed.flush_timer)
          ecore_timer_del(kd->changed.flush_timer);
        kd->changed.flush_timer =
          ecore_timer_add(5.0, _e_kbd_dict_cb_save_flush, kd);
     }
}

static const char *
_e_kbd_dict_find_pointer(E_Kbd_Dict *kd, const char *p, int baselen, const char *word)
{
   const char *pn;
   int len;

   if (!p) return NULL;
   len = strlen(word);
   while (p)
     {
        pn = _e_kbd_dict_line_next(kd, p);
        if (!pn) return NULL;
        if ((pn - p) > len)
          {
             if (!_e_kbd_dict_normalized_strncmp(p, word, len))
               return p;
          }
        if (_e_kbd_dict_normalized_strncmp(p, word, baselen))
          return NULL;
        p = pn;
        if (p >= (kd->file.dict + kd->file.size)) break;
     }
   return NULL;
}

static const char *
_e_kbd_dict_find(E_Kbd_Dict *kd, const char *word)
{
   const char *p;
   char *tword;
   int glyphs[2], p2, v1, v2, i;

   /* work backwards in leads. i.e.:
    * going
    * goin
    * goi
    * go
    * g
    */
   tword = alloca(strlen(word) + 1);
   _e_kbd_dict_normalized_strcpy(tword, word);
   p = eina_hash_find(kd->matches.leads, tword);
   if (p) return p;
   p2 = strlen(tword);
   while (tword[0])
     {
        p2 = evas_string_char_prev_get(tword, p2, &i);
        if (p2 < 0) break;
        tword[p2] = 0;
        p = eina_hash_find(kd->matches.leads, tword);
        if (p)
          return _e_kbd_dict_find_pointer(kd, p, p2, word);
     }
   /* looking at leads going back letters didn't work */
   p = kd->file.dict;
   if ((p[0] == '\n') && (kd->file.size <= 1)) return NULL;
   glyphs[0] = glyphs[1] = 0;
   p2 = evas_string_char_next_get(word, 0, &(glyphs[0]));
   if ((p2 > 0) && (glyphs[0] > 0))
     p2 = evas_string_char_next_get(word, p2, &(glyphs[1]));
   v1 = _e_kbd_dict_letter_normalise(glyphs[0]);
   if (glyphs[1] != 0)
     {
        v2 = _e_kbd_dict_letter_normalise(glyphs[1]);
        if (kd->lookup.tuples[v1][v2] >= 0)
          p = kd->file.dict + kd->lookup.tuples[v1][v2];
     }
   else
     {
        for (i = 0; i < 128; i++)
          {
             if (kd->lookup.tuples[v1][i] >= 0)
               p = kd->file.dict + kd->lookup.tuples[v1][i];
             if (p) break;
          }
     }
   return _e_kbd_dict_find_pointer(kd, p, p2, word);
}

static const char *
_e_kbd_dict_find_full(E_Kbd_Dict *kd, const char *word)
{
   const char *p;
   int len;

   p = _e_kbd_dict_find(kd, word);
   if (!p) return NULL;
   len = strlen(word);
   if (isspace(p[len])) return p;
   return NULL;
}

EAPI void
e_kbd_dict_word_usage_adjust(E_Kbd_Dict *kd, const char *word, int adjust)
{
   // add "adjust" to word usage count
   E_Kbd_Dict_Word *kw;

   kw = _e_kbd_dict_changed_write_find(kd, word);
   if (kw)
     {
        kw->usage += adjust;
        if (kd->changed.flush_timer)
          ecore_timer_del(kd->changed.flush_timer);
        kd->changed.flush_timer = ecore_timer_add(5.0, _e_kbd_dict_cb_save_flush, kd);
     }
   else
     {
        const char *line;
        int usage = 0;

        line = _e_kbd_dict_find_full(kd, word);
        if (line)
          {
             char *wd;

             // FIXME: we need to find an EXACT line match - case and all
             wd = _e_kbd_dict_line_parse(kd, line, &usage);
             free(wd);
          }
        usage += adjust;
        _e_kbd_dict_changed_write_add(kd, word, usage);
     }
}

EAPI void
e_kbd_dict_word_delete(E_Kbd_Dict *kd, const char *word)
{
   // delete a word from the dictionary
   E_Kbd_Dict_Word *kw;

   kw = _e_kbd_dict_changed_write_find(kd, word);
   if (kw)
     kw->usage = -1;
   else
     {
        if (_e_kbd_dict_find_full(kd, word))
          _e_kbd_dict_changed_write_add(kd, word, -1);
     }
}

EAPI void
e_kbd_dict_word_letter_clear(E_Kbd_Dict *kd)
{
   // clear the current word buffer
   while (kd->word.letters)
     e_kbd_dict_word_letter_delete(kd);
   if (kd->matches.deadends)
     {
        eina_hash_free(kd->matches.deadends);
        kd->matches.deadends = NULL;
     }
   if (kd->matches.leads)
     {
        eina_hash_free(kd->matches.leads);
        kd->matches.leads = NULL;
     }
   while (kd->matches.list)
     {
        E_Kbd_Dict_Word *kw;

        kw = kd->matches.list->data;
        eina_stringshare_del(kw->word);
        free(kw);
        kd->matches.list = eina_list_remove_list(kd->matches.list, kd->matches.list);
    }
}

EAPI void
e_kbd_dict_word_letter_add(E_Kbd_Dict *kd, const char *letter, int dist)
{
   // add a letter with a distance (0 == closest) as an option for the current
   // letter position - advance starts a new letter position
   Eina_List *l, *list;
   E_Kbd_Dict_Letter *kl;

   l = eina_list_last(kd->word.letters);
   if (!l) return;
   list = l->data;
   kl = E_NEW(E_Kbd_Dict_Letter, 1);
   if (!kl) return;
   kl->letter = eina_stringshare_add(letter);
   kl->dist = dist;
   list = eina_list_append(list, kl);
   l->data = list;
}

EAPI void
e_kbd_dict_word_letter_advance(E_Kbd_Dict *kd)
{
   // start a new letter in the word
   kd->word.letters = eina_list_append(kd->word.letters, NULL);
}

EAPI void
e_kbd_dict_word_letter_delete(E_Kbd_Dict *kd)
{
   // delete the current letter completely
   Eina_List *l, *list;

   l = eina_list_last(kd->word.letters);
   if (!l) return;
   list = l->data;
   while (list)
     {
        E_Kbd_Dict_Letter *kl;

        kl = list->data;
        eina_stringshare_del(kl->letter);
        free(kl);
        list = eina_list_remove_list(list, list);
     }
   kd->word.letters = eina_list_remove_list(kd->word.letters, l);
}

static void
_e_kbd_dict_matches_lookup_do(E_Kbd_Dict *kd, Eina_List *letters, char *buf, char *bufp, int maxdist, int wordlen, int distance, int *searched, int *found)
{
   Eina_List *l;
   E_Kbd_Dict_Letter *kl;
   const char *p;
   char *wd;
   E_Kbd_Dict_Word *kw;
   int usage = 0, len, d;

   if (letters)
     {
        for (l = letters->data; l; l = l->next)
          {
             kl = l->data;
             len = strlen(kl->letter);
             memcpy(bufp, kl->letter, len);
             bufp[len] = 0;
             if (_e_kbd_dict_find(kd, buf))
               {
                  d = kl->dist;
                  _e_kbd_dict_matches_lookup_do(kd, letters->next,
                                                buf, bufp + len, maxdist,
                                                wordlen,
                                                distance + (d * d * d),
                                                searched, found);
               }
          }
        return;
     }
   (*searched)++;

   p = _e_kbd_dict_find_full(kd, buf);
   if (!p) return;
   wd = _e_kbd_dict_line_parse(kd, p, &usage);
   if (wd)
     {
        if (_e_kbd_dict_normalized_strcmp(wd, buf))
          {
             free(wd);
             return;
          }
        kw = E_NEW(E_Kbd_Dict_Word, 1);
        if (kw)
          {
             int w, b, w2, b2, wc, bc;

             // match any capitalisation
             for (w = 0, b = 0; wd[w] && buf[b];)
               {
                  b2 = evas_string_char_next_get(buf, b, &bc);
                  w2 = evas_string_char_next_get(wd, w, &wc);
                  if (isupper(bc)) wd[w] = toupper(wc);
                  w = w2;
                  b = b2;
               }
             kw->word = eina_stringshare_add(wd);
             // FIXME: magic combination of distance metric and
             // frequency of usage. this is simple now, but could
             // be tweaked

             // basically a metric to see how far away the keys that
             // were actually pressed are away from the letters of
             // this word in a physical on-screen sense
             kw->accuracy = (maxdist - distance) / wordlen;
             // usage is the frequency of usage in the dictionary.
             // it its < 1 time, it's assumed to be 1.
             if (usage < 1) usage = 1;
             // multiply usage by a factor of 100 for better detailed
             // sorting. 10 == 1/10th factor
             kw->usage = 10 + (usage - 1);
             kd->matches.list = eina_list_append(kd->matches.list, kw);
             (*found)++;
          }
        free(wd);
     }
}

EAPI void
e_kbd_dict_matches_lookup(E_Kbd_Dict *kd)
{
   Eina_List *l, *ll;
   E_Kbd_Dict_Word *kw;
   E_Kbd_Dict_Letter *kl;
   int searched = 0;
   int found = 0;
   int maxdist = 0, lettermaxdist, wordlen, d, d1, d2;
   char *buf;

   // find all matches and sort them
   wordlen = eina_list_count(kd->word.letters);
   buf = alloca((wordlen + 1) * 10);
   while (kd->matches.list)
     {
        kw = kd->matches.list->data;
        eina_stringshare_del(kw->word);
        free(kw);
        kd->matches.list = eina_list_remove_list(kd->matches.list, kd->matches.list);
    }
   for (l = kd->word.letters; l; l = l->next)
     {
        lettermaxdist = 0;
        for (ll = l->data; ll; ll = ll->next)
          {
             kl = ll->data;
             d = kl->dist;
             lettermaxdist += d * d * d;
          }
        maxdist += lettermaxdist;
     }
   if (kd->word.letters)
     _e_kbd_dict_matches_lookup_do(kd, kd->word.letters, buf, buf, maxdist,
                                   wordlen, 0, &searched, &found);
   d1 = 0x7fffffff;
   d2 = 0;
   for (l = kd->matches.list; l; l = l->next)
     {
        kw = l->data;
        if (kw->accuracy < d1) d1 = kw->accuracy;
        if (kw->accuracy > d2) d2 = kw->accuracy;
     }
   for (l = kd->matches.list; l; l = l->next)
     {
        int a;
        long long tmp;

        kw = l->data;
        kw->accuracy -= d1;
        a = (kw->accuracy + 10) / 10;
        printf("-> %s:   acc=%i, use=%i -> ", kw->word, a, kw->usage);
        tmp = kw->usage + 100;
        tmp *= (a * a);
        tmp /= 10000;
        tmp *= (a * a);
        tmp /= 10000;
        tmp *= (a * a);
        tmp /= 10000;
        tmp *= (a * a);
        tmp /= 10000;
        kw->usage = tmp / 10000;
        printf("%i\n", kw->usage);
     }
   printf("====== searched: %i/%i\n", found, searched);
   kd->matches.list = eina_list_sort(kd->matches.list,
                                     eina_list_count(kd->matches.list),
                                     _e_kbd_dict_matches_lookup_cb_sort);
}

EAPI void
e_kbd_dict_matches_first(E_Kbd_Dict *kd)
{
   // jump to first match
   kd->matches.list_ptr = kd->matches.list;
}

EAPI void
e_kbd_dict_matches_next(E_Kbd_Dict *kd)
{
   // jump to next match
   kd->matches.list_ptr = kd->matches.list_ptr->next;
}

EAPI const char *
e_kbd_dict_matches_match_get(E_Kbd_Dict *kd, int *pri_ret)
{
   // return the word (string utf-8) for the current match
   if (kd->matches.list_ptr)
     {
        E_Kbd_Dict_Word *kw;

        kw = kd->matches.list_ptr->data;
        if (kw)
          {
             *pri_ret = kw->usage;
             return kw->word;
          }
     }
   return NULL;
}
