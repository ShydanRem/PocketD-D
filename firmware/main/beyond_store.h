#pragma once
#include "beyond_account.h"
bool pd_account_store_load(pd_account_access *out);
bool pd_account_store_save(const pd_account_access *access);
bool pd_account_store_forget(void);
