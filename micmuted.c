/***************************************
 * (c) Ventura, 2023-06-10
 *
 * micmuted.c
 *
 **************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <pulse/mainloop.h>
#include <pulse/context.h>
#include <pulse/subscribe.h>
#include <pulse/introspect.h>

#define LED_MUTE	"/sys/class/leds/platform::micmute/brightness"


void pa_source_info_cb( pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
   if (i != NULL) {
      pa_context_state_t state = pa_context_get_state(c);

      if (state == PA_CONTEXT_READY) {
         
	 //printf("Mute: %i, Index:%i\n", i->mute, i->index);
	 
	 char ch[2];
         int rc, fd = open(LED_MUTE, O_WRONLY); 

         if (i->mute == 0) {
            syslog(LOG_INFO, "Microphone unmuted: %s", i->description);
            rc = write(fd, "0", 1);
         } else
         if (i->mute == 1) {
            syslog(LOG_INFO, "Microphone muted: %s", i->description);
            rc = write(fd, "1", 1);
         }

         if (rc < 0) {
            syslog(LOG_ERR, "Led access error\n");
         }
         close(fd);
      }
   }
}


void pa_context_subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata)
{
   pa_context_state_t state = pa_context_get_state(c);
 
   switch  (state) {
      case PA_CONTEXT_READY:
	//printf("Event mask: %i\n", t);
	if (t & (PA_SUBSCRIPTION_EVENT_CHANGE | PA_SUBSCRIPTION_EVENT_SOURCE | PA_SUBSCRIPTION_EVENT_REMOVE | PA_SUBSCRIPTION_EVENT_SINK_INPUT)) {
	   //printf("Event mask: %i\n", t);
	   pa_context_get_source_info_by_index(c, -1, pa_source_info_cb, NULL);
	} else {
	   //printf("Index:%i\n", idx);
	   pa_context_get_source_info_by_index(c, idx, pa_source_info_cb, NULL);
	}
	break;
   }
}


void pa_state_cb(pa_context *c, void *userdata)
{
   pa_context_state_t state = pa_context_get_state(c);
 
   switch  (state) {
      case PA_CONTEXT_READY:
	      {
                 //set callback
                 pa_context_set_subscribe_callback(c, pa_context_subscribe_cb, NULL);

                 //set events mask and enable event callback.
                 //pa_operation* op = pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SOURCE, NULL, NULL);
                 pa_operation* op = pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_ALL, NULL, NULL);
       
                 if (op)
                   pa_operation_unref(op);
		    
		 pa_context_get_source_info_by_index(c, -1, pa_source_info_cb, NULL);
	      }
	      break;

      default:
              break;
   }
}

static void help(const char *argv0) {
    printf("%s [options]\n\n"
           "  -h, --help                            Show this help\n"
           "      --version                         Show version\n\n"
           ,
           argv0);
}


enum {
    ARG_VERSION = 256
};


int main(int argc, char* argv[])
{
   char* bn;
   int rc;
    
   static const struct option long_options[] = {
        {"version",      0, NULL, ARG_VERSION},
        {"help",         0, NULL, 'h'},
        {NULL,           0, NULL, 0}
   };

    while ((rc = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {

        switch (rc) {
            case 'h' :
                help(argv[0]);
                rc = 0;
                goto quit;

            case ARG_VERSION:
                printf("micmuted %s\nCompiled with libpulse %s\nLinked with libpulse %s\n", PACKAGE_VERSION, pa_get_headers_version(), pa_get_library_version());
                rc = 0;
                goto quit;
            
            default:
                goto quit;
        }
    }

   syslog(LOG_INFO, "micmuted started");
   pa_mainloop* m = pa_mainloop_new();
   pa_mainloop_api* m_api = pa_mainloop_get_api(m);

   /* Setup event */
   pa_context* ctx = pa_context_new(m_api, "micmute");
   
   pa_context_set_state_callback(ctx, pa_state_cb, NULL);
   pa_context_connect(ctx, NULL, 0, NULL);

   pa_mainloop_run(m, &rc);


   if (ctx)
      pa_context_unref(ctx);
   
   if (m)
      pa_mainloop_free(m);

   syslog(LOG_INFO, "micmuted terminated");

quit:
   exit(rc);
}


