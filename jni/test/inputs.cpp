#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>

#include <android/input.h>
#include <android/looper.h>

#include "activity.h"

/*
static void process_input(struct engine *engine) {
  if (engine->queue == NULL) return;

  while (AInputQueue_hasEvents(engine->queue) > 0) {
    AInputEvent* event = NULL;
    if (AInputQueue_getEvent(engine->queue, &event) >= 0) {
      LOGI("New input event: type=%d", AInputEvent_getType(event));
      if (AInputQueue_preDispatchEvent(engine->queue, event)) {
	return;
      }
      else {
	int32_t handled = 1;
	AInputQueue_finishEvent(engine->queue, event, handled);
      }
    }
    else {
      LOGI("Failure reading input event");
    }

  }
}
*/

static void* input_thread_entry(void* param) {
  struct engine *engine = (struct engine *)param;

  lua_State *Lthread = lua_newthread(engine->L);

  luaL_dostring(Lthread, "n = 0");
  while (1) {
    usleep(10000);
    luaL_dostring(Lthread, "n = n+1");

    /*
      // Kills UI thread
      sprintf(cmd,"toast.show(\"event %d\")",n);
      write(engine->msgwrite, cmd, strlen(cmd));
    */
  }

  return NULL;
}


extern "C"
int input_thread_start(void* param) {
  struct engine *engine = (struct engine *)param;

  /*
  pthread_mutex_init(&engine->mutex, NULL);
  pthread_cond_init(&engine->cond, NULL);
  */

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&engine->thread, &attr, input_thread_entry, engine);

}
