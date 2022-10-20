/*
 * HUAYI UCI Map
 * This map provdies the mapping information between the string index (e.g. radio_id) and uci section index 
 */

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>  
#include <stdio.h>  
#include <uci.h>

#include <sys/sysinfo.h>

#include <huayi_lock.h>
#include <huayi_uci.h>

/* A_*** redefine in atheros wifi driver */
#define A_DEFINE_MUTEX      HUAYI_DEFINE_MUTEX
#define A_MUTEX_LOCK        HUAYI_MUTEX_LOCK
#define A_MUTEX_UNLOCK      HUAYI_MUTEX_UNLOCK
#define A_MUTEX_DESTROY     HUAYI_MUTEX_DESTROY     
#define A_MUTEX_INIT        HUAYI_MUTEX_INIT

/* UCI context cache */
static struct huayi_uci_get_context* g_uci_contexts[PKG_NUM_OF_PKG];

static a_mutex_t g_ctx_cache_lock;
static A_DEFINE_MUTEX(g_ctx_cache_lock);


/***************
 *  UCI Context handling
 */

/**
 * Create UCI Context
 *
 * @return NULL if failed
 */
static struct uci_context* create_uci_context(const char* path) {

  HUAYI_UCI_LOG("Create HUAYI CTX, Path: %s\n", path);

  struct uci_context* ctx = uci_alloc_context();

  if (!ctx) {
	  HUAYI_UCI_LOG("Failed to allocate UCI context, out of memory\n");
	  return NULL;
  }

  HUAYI_UCI_LOG("Create UCI Context: %i\n", ctx);

  /*common it for fixed network ifname always br-lan*/
  //uci_add_delta_path(ctx, "/var/state");
  if (path)
    uci_set_confdir(ctx, path);
  else
    uci_set_confdir(ctx, DEFAULT_UCI_CONFIG_PATH);

  return ctx;

}

/**
 * Init UCI context
 * path - don't set path if path is null
 * packages - shall be multiple (i.e SYSTEM|NETWORK)
 */
static int
init_uci_context(struct uci_context* ctx, unsigned char package) {

  HUAYI_UCI_LOG("INIT HUAYI CTX: %i\n", package);

  if(uci_load(ctx, PKG_ID_TOFILE(package), NULL) != UCI_OK)
  {
	HUAYI_UCI_LOG("%s/%s is missing or corrupt\n", ctx->confdir, PKG_ID_TOFILE(package));
	return 0;
  }
  return 1;

}

/**
 * Clean up the specified ctx context
 */
static void
clean_uci_context(struct uci_context* ctx) {
  HUAYI_UCI_LOG("Clean HUAYI UCI Context: %i\n", ctx);
  if (ctx) {
      uci_free_context(ctx);
  }
}

static void release_global_get_context(struct huayi_uci_get_context* get_ctx) {
    if (get_ctx) {
        A_MUTEX_LOCK(&(get_ctx->get_ctx_lock));
        if (get_ctx->get_uci_context) {
            clean_uci_context(get_ctx->get_uci_context);
            get_ctx->get_uci_context = NULL;
        }
        A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
        A_MUTEX_DESTROY(&(get_ctx->get_ctx_lock));
    }
}

/*
 * Get global get context cache
 */
static struct huayi_uci_get_context* get_global_get_context(unsigned char package) {

    struct huayi_uci_get_context* get_ctx;

    get_ctx = g_uci_contexts[package];
    if (get_ctx == NULL) {
        A_MUTEX_LOCK(&g_ctx_cache_lock);
        get_ctx = g_uci_contexts[package];
        if (get_ctx == NULL) {
            get_ctx = (struct huayi_uci_get_context *) malloc(sizeof(struct huayi_uci_get_context));
            if (get_ctx == NULL) {
                HUAYI_UCI_LOG("Failed to create UCI get context\n");
                A_MUTEX_UNLOCK(&g_ctx_cache_lock);
                return NULL;
            }
            // Init get context
            memset(get_ctx, 0 , sizeof(struct huayi_uci_get_context));
            get_ctx->package = package;
            A_MUTEX_INIT(&(get_ctx->get_ctx_lock));

            // Store the context
            g_uci_contexts[package] = get_ctx;
        }
        A_MUTEX_UNLOCK(&g_ctx_cache_lock);
    }

    return get_ctx;

}

/**
 * Refresh UCI get context,
 * MUST get get_ctx->get_ctx_lock before call this method
 *
 * Refresh or create new underlying uci context
 */
static int refresh_uci_get_context(struct huayi_uci_get_context* get_ctx, long now) {

    int ret;

    HUAYI_UCI_LOG("Refresh Get Context, Package: %i\n", get_ctx->package);

    if (get_ctx->get_uci_context != NULL) {
        clean_uci_context(get_ctx->get_uci_context);
    }

    get_ctx->get_uci_context = create_uci_context(PKG_FILE_PATH(get_ctx->package));
    if (get_ctx->get_uci_context == NULL) {
        return 0;
    }

    ret = init_uci_context(get_ctx->get_uci_context, get_ctx->package);
    if (!ret) {
        clean_uci_context(get_ctx->get_uci_context);
        get_ctx->get_uci_context = NULL;
        return 0;
    }

    if (IS_PKG_CAN_CACHE(get_ctx->package)) {
        get_ctx->ctx_lastupdate = now;
    }

    return 1;
}


static void
invalidate_read_uci_contexts(){

    int i;
    HUAYI_UCI_LOG("Invalidate the read uci contexts\n");
    for (i=0;i<PKG_NUM_OF_PKG;i++) {
      if (g_uci_contexts[i]) {
          A_MUTEX_LOCK(&(g_uci_contexts[i]->get_ctx_lock));
          g_uci_contexts[i]->ctx_lastaccess = 0;
          g_uci_contexts[i]->ctx_lastupdate = 0;
          A_MUTEX_UNLOCK(&(g_uci_contexts[i]->get_ctx_lock));
      }
    }
}

static struct huayi_uci_get_context* i_huayi_uci_get_uci_context(unsigned char package, const char* path, bool disable_la) {

  struct huayi_uci_get_context* get_ctx;
  struct sysinfo info;
  long now = 0;
  int ret = 0;

  sysinfo(&info);
  now = (long)info.uptime;

  /* Init the get context if it is not initialized yet */
  get_ctx = get_global_get_context(package);
  if (get_ctx == NULL) {
      return NULL;
  }

  HUAYI_UCI_LOG("Get HUAYI CTX, Package: %i %s Time: %ld Last Updated: %ld Last Accessed: %ld\n", package, path, now, get_ctx->ctx_lastupdate, get_ctx->ctx_lastaccess);

  A_MUTEX_LOCK(&(get_ctx->get_ctx_lock));

  /* Refresh the UCI context if: */
  /* - The context is empty */
  /* - The existing context is timeout and last access is exceeded the timeout. */
  if ((get_ctx->get_uci_context == NULL) || (((get_ctx->ctx_lastupdate + DEFAULT_CTX_TIMEOUT) <= now )
				  && (disable_la || ((get_ctx->ctx_lastaccess + DEFAULT_CTX_LASTACCESS_TIMEOUT) <= now)))) {
      /* Refresh the underlying UCI context */
      if (!refresh_uci_get_context(get_ctx, now)) {
          A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
          return NULL;
      }
  }

  /* Update the last access time */
  if (IS_PKG_CAN_CACHE(package)) {
	  get_ctx->ctx_lastaccess = now;
  }

  A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));

  HUAYI_UCI_LOG("Got HUAYI CTX, Package: %i CTX dir:%s\n", package, get_ctx->get_uci_context->confdir);

  return get_ctx;

}

struct huayi_uci_get_context* huayi_uci_get_uci_context(unsigned char package, const char* path) {
	return i_huayi_uci_get_uci_context(package, path, true);
}

struct huayi_uci_get_context* huayi_uci_get_uci_context_nla(unsigned char package, const char* path) {
	return i_huayi_uci_get_uci_context(package, path, true);
}

struct huayi_uci_get_context* huayi_uci_get_uci_context_lap(unsigned char package, const char* path) {
	return i_huayi_uci_get_uci_context(package, path, false);
}

/**
 * Get the uci option using the uci string
 * If the uci option is a list, a concatenated string will be returned with the space as the delimiter
 * For option list, the list size larger than MAX_UCI_VALUE_LEN will be truncated
 * It is caller's responsibility to ensure the result has enough space to carry result
 * @ctx  		UCI context
 * @uci_str  	UCI option string
 * @result		UCI result value string
 * @return 0, if failed
 */
int huayi_uci_get_option(struct huayi_uci_get_context *get_ctx,  char *uci_str, char *result)
{
	struct uci_ptr ptr;
	char get_uci_str[MAX_UCI_STRLEN + 1];
	struct uci_element *e = NULL;
	bool sep = false;
    char* t_result = result;
	long now = 0;
	struct sysinfo info;

  	sysinfo(&info);
	now = (long)info.uptime;

	if (!get_ctx || !uci_str || !result) {
		return 0;
	}

    A_MUTEX_LOCK(&(get_ctx->get_ctx_lock));

    /* The internal UCI context may become empty, if the cache is being dropped after the get context is initialized */
	if (get_ctx->get_uci_context == NULL) {
	    HUAYI_UCI_LOG("HUAYI UCI Get Context is invalid, reinit now\n");
	    if (!refresh_uci_get_context(get_ctx, (long)now)) {
	        HUAYI_UCI_LOG("Unable to reinit the context\n");
	        A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
	        return 0;
	    }
	}

	memset(&ptr, 0, sizeof(struct uci_ptr));
	memset(get_uci_str, 0, sizeof(get_uci_str));

	/* Construct the Get UCI string, since uci_str pass to uci api must not be a const string */
	strcpy(get_uci_str, uci_str);

	uci_lookup_ptr(get_ctx->get_uci_context, &ptr, get_uci_str, true);
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		HUAYI_UCI_LOG("Lookup is incomplete, %s CTX:%p\n", uci_str, get_ctx->get_uci_context);
		A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
		return 0;
	}

	e = ptr.last;
	switch (e->type) {
	case UCI_TYPE_OPTION:
		if (ptr.o->type == UCI_TYPE_STRING) {
			strcpy(result, ptr.o->v.string);
		} else if (ptr.o->type == UCI_TYPE_LIST) {
			*t_result = '\0';
			uci_foreach_element(&(ptr.o->v.list), e) {
				/* Ignore the string if the result is already full */
				if ((strlen(result) + strlen(e->name) + 2) <= (MAX_UCI_VALUE_LEN-1)) {
					t_result += sprintf(t_result, "%s%s", sep?" ":"", e->name);
					sep = true;
				}
			}
		} else {
		    A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
			return 0;
		}
		break;
	default:
	    A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
		return 0;
	}
	A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
	return 1;
}


/**
 * Get the next item in uci list
 * It is caller's responsibility to ensure the result has enough space to carry result
 * @ctx  		UCI context
 * @uci_str  	UCI option string
 * @cur_str	    Current item, null if want to get the first item
 * @result		UCI result value string
 * @return 0, if failed
 */
int huayi_uci_is_list_item_found(struct huayi_uci_get_context *get_ctx,  char *uci_str, char *cur_str, bool case_sensitive)
{
	struct uci_ptr ptr;
	char get_uci_str[MAX_UCI_STRLEN + 1];
	struct uci_element *e = NULL;
	long now = 0;
	struct sysinfo info;

  	sysinfo(&info);
	now = (long)info.uptime;

	if (!get_ctx || !uci_str) {
		return 0;
	}

    A_MUTEX_LOCK(&(get_ctx->get_ctx_lock));

    /* The internal UCI context may become empty, if the cache is being dropped after the get context is initialized */
    if (get_ctx->get_uci_context == NULL) {
        HUAYI_UCI_LOG("HUAYI UCI Get Context is invalid, reinit now\n");
        if (!refresh_uci_get_context(get_ctx, (long)now)) {
            HUAYI_UCI_LOG("Unable to reinit the context\n");
            A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
            return 0;
        }
    }

	memset(&ptr, 0, sizeof(struct uci_ptr));
	memset(get_uci_str, 0, sizeof(get_uci_str));

	/* Construct the Get UCI string, since uci_str pass to uci api must not be a const string */
	strcpy(get_uci_str, uci_str);

	HUAYI_UCI_LOG("Searching List UCI: %s Item: %s\n", get_uci_str, cur_str);

	uci_lookup_ptr(get_ctx->get_uci_context, &ptr, get_uci_str, true);
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
	    A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
		return 0;
	}

	e = ptr.last;
	if (e->type == UCI_TYPE_OPTION && ptr.o->type == UCI_TYPE_LIST) {
		uci_foreach_element(&(ptr.o->v.list), e) {
			HUAYI_UCI_LOG("Matching Items: %s, Item: %s\n", e->name, cur_str);
			if (case_sensitive) {
				if (strcmp(e->name, cur_str)==0) {
				    A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
					return 1;
				}
			} else {
				if (strcasecmp(e->name, cur_str)==0) {
				    A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
					return 1;
				}			
			}
		}
	}

	HUAYI_UCI_LOG("Item is not found UCI: %s Item: %s\n", uci_str, cur_str);

	A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));

	return 0;
}

/**
 * Get the next item in uci list
 * It is caller's responsibility to ensure the result has enough space to carry result
 * @ctx  		UCI context
 * @uci_str  	UCI option string
 * @cur_str	    Current item, null if want to get the first item
 * @result		UCI result value string
 * @return 0, if failed
 */
int huayi_uci_get_list_next(struct huayi_uci_get_context *get_ctx,  char *uci_str, char *cur_str, bool case_sensitive, char *result)
{
	struct uci_ptr ptr;
	char get_uci_str[MAX_UCI_STRLEN + 1];
	struct uci_element *e = NULL;
	bool sep = false;
	bool found = false;
	long now = 0;
	struct sysinfo info;

  	sysinfo(&info);
	now = (long)info.uptime;

	if (!get_ctx || !uci_str || !result) {
		return 0;
	}

    A_MUTEX_LOCK(&(get_ctx->get_ctx_lock));

    /* The internal UCI context may become empty, if the cache is being dropped after the get context is initialized */
    if (get_ctx->get_uci_context == NULL) {
        HUAYI_UCI_LOG("HUAYI UCI Get Context is invalid, reinit now\n");
        if (!refresh_uci_get_context(get_ctx, (long)now)) {
            HUAYI_UCI_LOG("Unable to reinit the context\n");
            A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
            return 0;
        }
    }

	memset(&ptr, 0, sizeof(struct uci_ptr));
	memset(get_uci_str, 0, sizeof(get_uci_str));

	/* Construct the Get UCI string, since uci_str pass to uci api must not be a const string */
	strcpy(get_uci_str, uci_str);

	uci_lookup_ptr(get_ctx->get_uci_context, &ptr, get_uci_str, true);
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
	    A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
		return 0;
	}

	e = ptr.last;
	if (e->type == UCI_TYPE_OPTION && ptr.o->type == UCI_TYPE_LIST) {
		uci_foreach_element(&(ptr.o->v.list), e) {
			HUAYI_UCI_LOG("Getting list next item, Cur: %s, This Item: %s\n", cur_str, e->name);
			/* Get the first item if the cur_str is NULL */
			if (!cur_str) {
				/* Ignore the empty strings */
				if (strlen(e->name)>0) {
					strcpy(result, e->name);
					A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
					return 1;
				}
			} else {
				if (found) {
					/* Ignore the empty string and The next item should not the same as the current one (duplicated entry) */
					if (strlen(e->name)>0) {
						if (case_sensitive) {
							if (strcmp(e->name, cur_str)!=0) {
								strcpy(result, e->name);
								A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
								return 1;
							}
						} else {
							if (strcasecmp(e->name, cur_str)!=0) {
								strcpy(result, e->name);
								A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
								return 1;
							}						
						}
					} else {
						found = false;
					}
				}

				if (case_sensitive) {
					if (strcmp(e->name, cur_str)==0) {
						found = true;
					}
				} else {
					if (strcasecmp(e->name, cur_str)==0) {
						found = true;
					}				
				}
			}
		}
	}

	A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));

	return 0;
}



void huayi_uci_clean_uci_context_cache(unsigned char package, bool clean_useless_only) {

  int i = 0;
  long now = 0;
  struct huayi_uci_get_context* get_ctx;
  struct sysinfo info;

  sysinfo(&info);
  now = (long)info.uptime;

  HUAYI_UCI_LOG("Cleaning UCI context cache...\n");

  /* Clean the cached context */
  /* If the existing context is timeout and last access is exceeded the timeout. */
  for (i=1;i<PKG_NUM_OF_PKG;i++) {
	if (package == 0 || package == i) {
	    get_ctx = g_uci_contexts[i];
		if (get_ctx) {
            A_MUTEX_LOCK(&(get_ctx->get_ctx_lock));
			if ((!clean_useless_only) || ((get_ctx->ctx_lastupdate + DEFAULT_CTX_TIMEOUT) <= now )
				&& ((get_ctx->ctx_lastaccess + DEFAULT_CTX_LASTACCESS_TIMEOUT) <= now)) {
				HUAYI_UCI_LOG("Dropping UCI context PKG: %d\n", i);
                if (get_ctx->get_uci_context) {
                    clean_uci_context(get_ctx->get_uci_context);
                    get_ctx->get_uci_context = NULL;
                }
                get_ctx->ctx_lastaccess = 0;
                get_ctx->ctx_lastupdate = 0;               
			}
            A_MUTEX_UNLOCK(&(get_ctx->get_ctx_lock));
		}
		/* Clean the targeted package already, can exit now */
		if (package != 0) {
			break;
		}
	}
  }

}


/** Initializes the huayi_uci module */
void
huayi_uci_init(void)
{
  memset(g_uci_contexts, 0, sizeof(g_uci_contexts));
}

void huayi_uci_release(void) {

  int i;

  HUAYI_UCI_LOG("Release UCI\n");

  /* Clean the cached context */
  A_MUTEX_LOCK(&g_ctx_cache_lock);
  for (i=0;i<PKG_NUM_OF_PKG;i++) {
	if (g_uci_contexts[i]) {
	    release_global_get_context(g_uci_contexts[i]);
		free(g_uci_contexts[i]);
		g_uci_contexts[i] = NULL;
	}
  }
  A_MUTEX_UNLOCK(&g_ctx_cache_lock);

  /* Destroy locks */
  A_MUTEX_DESTROY(&g_ctx_cache_lock);


}
