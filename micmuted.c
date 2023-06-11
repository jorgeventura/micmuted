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

static int source_index;
static char description[256];

void pa_source_info_name2index_cb( pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
   if (i != NULL) {
      pa_context_state_t state = pa_context_get_state(c);
      
      switch  (state) {
         case PA_CONTEXT_READY:
      
	   source_index = i->index;
	   
           if (!eol) {
              int fd = open(LED_MUTE, O_WRONLY); 
              int rc;

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
           break;
      }
   }

}

void pa_source_info_cb( pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
   if (i != NULL) {
      pa_context_state_t state = pa_context_get_state(c);

      switch  (state) {
         case PA_CONTEXT_READY:
	   //printf("Mute: %i\n", i->mute);
           if (!eol) {
              int fd = open(LED_MUTE, O_WRONLY); 
              int rc;

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
           break;
      }
   }
}


void pa_context_subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata)
{
   pa_context_state_t state = pa_context_get_state(c);
 
   switch  (state) {
      case PA_CONTEXT_READY:
	if (source_index > 0) {
	   pa_context_get_source_info_by_index(c, source_index, pa_source_info_cb, NULL);
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
                 pa_operation* op = pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SOURCE, NULL, NULL);
       
                 if (op)
                   pa_operation_unref(op);

		 if (source_index > 0) {
		    pa_context_get_source_info_by_index(c, source_index, pa_source_info_cb, NULL);
		 } else
		 if (description[0] != '\0') {
		    pa_context_get_source_info_by_name(c, description, pa_source_info_name2index_cb, NULL);
		 }
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
           "      --source-name=NAME                Source name as in Source.name"
	   "                                           ex: alsa_input.pci-0000_00_1f.3-platform-skl_hda_dsp_generic.HiFi__hw_sofhdadsp_6__source)\n"
           "      --source-index=IDX                Source index #\n"
           ,
           argv0);
}


enum {
    ARG_VERSION = 256,
    ARG_SOURCE_NAME,
    ARG_SOURCE_INDEX
};


int main(int argc, char* argv[])
{
   char* bn;
   int rc;
    
   static const struct option long_options[] = {
        {"source-name",  1, NULL, ARG_SOURCE_NAME},
        {"source-index", 1, NULL, ARG_SOURCE_INDEX},
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
            
	    case ARG_SOURCE_NAME:
                sprintf(description, "%.*s", (int)sizeof(description), optarg);
                rc = 0;
		break;
	    
	    case ARG_SOURCE_INDEX:
                source_index = (size_t) atoi(optarg);
                break;

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


