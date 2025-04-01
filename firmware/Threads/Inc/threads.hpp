#ifndef __TASKS_HPP__
#define __TASKS_HPP__

#include "tx_api.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

VOID main_thread_entry(ULONG thread_input);
VOID test_thread_entry(ULONG thread_input);
VOID bq796xx_thread_entry(ULONG thread_input);
VOID ads131m04_thread_entry(ULONG thread_input);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TASKS_HPP */