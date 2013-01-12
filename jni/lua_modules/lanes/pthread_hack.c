// pthread functions not present in Android bionic:
#ifdef __ANDROID__

#include <pthread.h>

int pthread_cancel(pthread_t thread) { }
int pthread_yield() { }

#endif
