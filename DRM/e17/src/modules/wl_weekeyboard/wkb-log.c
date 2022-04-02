/*
 * Copyright © 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "wkb-log.h"

int _wkb_log_domain = -1;
static int _init_count = 0;

int
wkb_log_init(const char *domain)
{
   if (_init_count)
      goto end;

   if (!eina_init())
     {
        fprintf(stderr, "%s:%d - %s() Error initializing Eina\n", __FILE__, __LINE__, __FUNCTION__);
        return 0;
     }

   _wkb_log_domain = eina_log_domain_register(domain, EINA_COLOR_LIGHTCYAN);

   if (_wkb_log_domain < 0)
      {
         EINA_LOG_ERR("Unable to register '%s' log domain", domain);
         eina_shutdown();
         return 0;
      }

end:
   return ++_init_count;
}

void
wkb_log_shutdown(void)
{
   if (_init_count <= 0 || --_init_count > 0)
      return;

   eina_log_domain_unregister(_wkb_log_domain);
   eina_shutdown();
}
