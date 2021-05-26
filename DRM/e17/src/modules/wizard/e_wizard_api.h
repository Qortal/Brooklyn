#ifndef E_WIZARD_API_H

static const E_Wizard_Api *api = NULL;

E_API void
wizard_api_set(const E_Wizard_Api *a)
{
   api = a;
}

#endif
