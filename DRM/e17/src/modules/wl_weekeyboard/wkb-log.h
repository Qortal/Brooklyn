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

#ifndef _WKB_LOG_H_
#define _WKB_LOG_H_

#include <Eina.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DBG
#undef DBG
#endif
#ifdef INF
#undef INF
#endif
#ifdef WRN
#undef WRN
#endif
#ifdef ERR
#undef ERR
#endif
#ifdef CRITICAL
#undef CRITICAL
#endif

extern int _wkb_log_domain;
#define DBG(...)      EINA_LOG_DOM_DBG(_wkb_log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_wkb_log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_wkb_log_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_wkb_log_domain, __VA_ARGS__)
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_wkb_log_domain, __VA_ARGS__)

int wkb_log_init(const char *domain);
void wkb_log_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* _WKB_LOG_H_ */
