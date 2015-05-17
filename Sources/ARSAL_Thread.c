/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
/**
 * @file libARSAL/ARSAL_Thread.c
 * @brief This file contains sources about thread abstraction layer
 * @date 05/18/2012
 * @author frederic.dhaeyer@parrot.com
 */
#include <stdlib.h>
#include <config.h>
#include <stdio.h>
#include <libARSAL/ARSAL_Thread.h>
#include <libARSAL/ARSAL_Print.h>

#if defined(_PSP)
#include <pspthreadman.h>
#elif defined(HAVE_PTHREAD_H)
#include <pthread.h>
#else
#error The pthread.h header is required in order to build the library
#endif

#if defined(_PSP)
struct psp_thread_data
{
    void *ret;
    ARSAL_Thread_Routine_t func;
    void *data;
};

struct psp_thread
{
    SceUID id;
    struct psp_thread_data data;
};

static int psp_thread_generic_func(SceSize args, void *argp)
{
    struct psp_thread_data *data = (struct psp_thread_data *) argp;

    data->ret = data->func(data->data);
    return 0;
}
#endif

int ARSAL_Thread_Create(ARSAL_Thread_t *thread, ARSAL_Thread_Routine_t routine, void *arg)
{
    int result = 0;

#if defined(_PSP)
    struct psp_thread *psp_thread = malloc(sizeof(*psp_thread));

    psp_thread->data.func = routine;
    psp_thread->data.data = arg;

    psp_thread->id = sceKernelCreateThread("ARSAL thread",
            psp_thread_generic_func, 0x11, 0x8000, PSP_THREAD_ATTR_USER, NULL);

    if (psp_thread->id >= 0) {
        sceKernelStartThread(psp_thread->id, sizeof(struct psp_thread_data),
                &psp_thread->data);
        *thread = (ARSAL_Thread_t) psp_thread;
    } else {
        free(psp_thread);
        result = -1;
    }

#elif defined(HAVE_PTHREAD_H)
    pthread_t *pthread = (pthread_t *)malloc(sizeof(pthread_t));
    *thread = (ARSAL_Thread_t)pthread;
    result = pthread_create((pthread_t *)*thread, NULL, routine, arg);
#endif

    return result;
}

int ARSAL_Thread_Join(ARSAL_Thread_t thread, void **retval)
{
    int result = 0;

#if defined(_PSP)
    struct psp_thread *psp_thread = (struct psp_thread *) thread;

    sceKernelWaitThreadEnd(psp_thread->id, NULL);
    sceKernelDeleteThread(psp_thread->id);

    if (retval)
        *retval = psp_thread->data.ret;
#elif defined(HAVE_PTHREAD_H)
    result = pthread_join((*(pthread_t *)thread), retval);
#endif

    return result;
}

int ARSAL_Thread_Destroy(ARSAL_Thread_t *thread)
{
    int result = 0;

#if defined(_PSP)
    struct psp_thread *psp_thread = (struct psp_thread *) *thread;
    sceKernelTerminateDeleteThread (psp_thread->id);
    free(psp_thread);
#elif defined(HAVE_PTHREAD_H)
    free(*thread);
#endif

    return result;
}

int ARSAL_Thread_GetThreadID(void)
{
    int ret = -1;

#if defined(_PSP)
    ret = sceKernelGetThreadId();
#elif defined(HAVE_PTHREAD_H)
    ret = pthread_self();
#endif

    return ret;
}
